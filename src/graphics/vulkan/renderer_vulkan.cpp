#include "renderer_vulkan.hpp"
#include "material_vulkan.hpp"
#include "mesh_vulkan.hpp"
#include "texture_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/components/camera.hpp>
#include <rabbit/components/geometry.hpp>
#include <rabbit/components/transform.hpp>
#include <rabbit/math/math.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

using namespace rb;

renderer_vulkan::renderer_vulkan(graphics_device_vulkan& graphics_device)
    : _graphics_device(graphics_device)
    , _device(graphics_device.device())
    , _command_pool(graphics_device.command_pool())
    , _allocator(graphics_device.allocator())
    , _main_render_pass(graphics_device.render_pass()) {
    _create_command_buffer();
    _create_camera_buffer();
    _create_forward_pipeline();

    unsigned char pixels[] = { 255, 255, 255, 255 };

    texture_desc texture_desc;
    texture_desc.pixels = pixels;
    texture_desc.size = { 1, 1, 0 };
    texture_desc.type = texture_type::texture_2d;
    texture_desc.filter = texture_filter::nearest;
    texture_desc.format = texture_format::rgba8;
    texture_desc.wrap = texture_wrap::repeat;
    texture_desc.mipmaps = 1;
    texture_desc.layers = 1;
    texture_desc.is_render_target = false;
    _white_texture = _graphics_device.create_texture(texture_desc);
}

renderer_vulkan::~renderer_vulkan() {
    vkWaitForFences(_device, 1, &_fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(_device, _fence, nullptr);
    vkFreeCommandBuffers(_device, _command_pool, 1, &_command_buffer);

    vmaDestroyBuffer(_allocator, _camera_buffer, _camera_buffer_allocation);

    for (const auto& [entity, data] : _entity_local_data) {
        vkDestroyDescriptorPool(_device, data.descriptor_pool, nullptr);
        vmaDestroyBuffer(_allocator, data.local_buffer, data.local_buffer_allocation);
    }

    vkDestroyPipeline(_device, _forward_pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _forward_pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _forward_descriptor_layout, nullptr);
}

void renderer_vulkan::render(registry& registry) {
    _begin();

    registry.view<transform, camera>().each([&](transform& transform, camera& camera) {
        camera_data camera_data;
        camera_data.proj = mat4f::perspective(deg2rad(camera.field_of_view), 1280.0f / 720.0f, 0.1f, 100.0f);
        camera_data.view = invert(mat4f::translation(transform.position));
        vkCmdUpdateBuffer(_command_buffer, _camera_buffer, 0, sizeof(camera_data), &camera_data);
    });

    registry.view<transform, geometry>().each([&](const entity entity, transform& transform, geometry& geometry) {
        if (!geometry.mesh || !geometry.material) {
            return;
        }

        auto& entity_local_data = _entity_local_data[entity];

        if (!entity_local_data.local_buffer) {
            _create_local_buffer(entity_local_data);
        }

        if (!entity_local_data.descriptor_set) {
            _create_descriptor_set(entity_local_data);
            _update_descriptor_set(entity_local_data, geometry.material);
        }

        local_data local_data;
        local_data.world = mat4f::rotation(transform.rotation);
        vkCmdUpdateBuffer(_command_buffer, entity_local_data.local_buffer, 0, sizeof(local_data), &local_data);
    });

    _begin_main_render_pass();

    vkCmdBindPipeline(_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _forward_pipeline);

    registry.view<transform, geometry>().each([&](const entity entity, transform& transform, geometry& geometry) {
        auto& entity_local_data = _entity_local_data[entity];
        if (!entity_local_data.local_buffer) {
            return;
        }

        const auto mesh = std::static_pointer_cast<mesh_vulkan>(geometry.mesh);

        const auto vertex_buffer = mesh->vertex_buffer();
        const VkDeviceSize offset{ 0 };

        vkCmdBindVertexBuffers(_command_buffer, 0, 1, &vertex_buffer, &offset);
        vkCmdBindDescriptorSets(_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _forward_pipeline_layout, 0, 1, &entity_local_data.descriptor_set, 0, nullptr);
        vkCmdDraw(_command_buffer, static_cast<std::uint32_t>(mesh->vertex_count()), 1, 0, 0);
    });

    _end_main_render_pass();

    _end();

    _graphics_device.submit(_command_buffer, _fence);
}

void renderer_vulkan::_begin() {
    RB_MAYBE_UNUSED auto result = vkWaitForFences(_device, 1, &_fence, VK_TRUE, UINT64_MAX);
    RB_ASSERT(result == VK_SUCCESS, "Failed to wait for render fence");

    result = vkResetFences(_device, 1, &_fence);
    RB_ASSERT(result == VK_SUCCESS, "Failed to reset render fence");

     // Now that we are sure that the commands finished executing,
    // we can safely reset the command buffer to begin recording again.
    result = vkResetCommandBuffer(_command_buffer, 0);
    RB_ASSERT(result == VK_SUCCESS, "Failed to reset command buffer");

    // Begin the command buffer recording.
    // We will use this command buffer exactly once, so we want to let Vulkan know that.
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    result = vkBeginCommandBuffer(_command_buffer, &begin_info);
    RB_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer");
}

void renderer_vulkan::_end() {
    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    RB_MAYBE_UNUSED auto result = vkEndCommandBuffer(_command_buffer);
    RB_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
}

void renderer_vulkan::_begin_main_render_pass() {
    VkClearValue clear_values[2];
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = _main_render_pass;
	render_pass_begin_info.framebuffer = _graphics_device.framebuffer();
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = _graphics_device.swapchain_extent();
	render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
	render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void renderer_vulkan::_end_main_render_pass() {
    vkCmdEndRenderPass(_command_buffer);
}

void renderer_vulkan::_create_command_buffer() {
    VkCommandBufferAllocateInfo command_buffer_alloc_info;
    command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_alloc_info.pNext = nullptr;
    command_buffer_alloc_info.commandPool = _command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_alloc_info.commandBufferCount = 1;

    RB_MAYBE_UNUSED auto result = vkAllocateCommandBuffers(_device, &command_buffer_alloc_info, &_command_buffer);
    RB_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffer");

    VkFenceCreateInfo fence_info;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    result = vkCreateFence(_device, &fence_info, nullptr, &_fence);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create fence");
}

void renderer_vulkan::_create_camera_buffer() {
    VkBufferCreateInfo uniform_buffer_info;
    uniform_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniform_buffer_info.pNext = nullptr;
    uniform_buffer_info.flags = 0;
    uniform_buffer_info.size = sizeof(camera_data);
    uniform_buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    uniform_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uniform_buffer_info.queueFamilyIndexCount = 0;
    uniform_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo uniform_buffer_allocation_info;
    uniform_buffer_allocation_info.flags = 0;
    uniform_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    uniform_buffer_allocation_info.requiredFlags = 0;
    uniform_buffer_allocation_info.preferredFlags = 0;
    uniform_buffer_allocation_info.memoryTypeBits = 0;
    uniform_buffer_allocation_info.pool = VK_NULL_HANDLE;
    uniform_buffer_allocation_info.pUserData = nullptr;

    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(_allocator,
        &uniform_buffer_info,
        &uniform_buffer_allocation_info,
        &_camera_buffer,
        &_camera_buffer_allocation,
        nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan uniform buffer");
}

void renderer_vulkan::_create_local_buffer(entity_local_data& entity_local_data) {
    VkBufferCreateInfo uniform_buffer_info;
    uniform_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniform_buffer_info.pNext = nullptr;
    uniform_buffer_info.flags = 0;
    uniform_buffer_info.size = sizeof(local_data);
    uniform_buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    uniform_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uniform_buffer_info.queueFamilyIndexCount = 0;
    uniform_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo uniform_buffer_allocation_info;
    uniform_buffer_allocation_info.flags = 0;
    uniform_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    uniform_buffer_allocation_info.requiredFlags = 0;
    uniform_buffer_allocation_info.preferredFlags = 0;
    uniform_buffer_allocation_info.memoryTypeBits = 0;
    uniform_buffer_allocation_info.pool = VK_NULL_HANDLE;
    uniform_buffer_allocation_info.pUserData = nullptr;

    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(_allocator,
        &uniform_buffer_info,
        &uniform_buffer_allocation_info,
        &entity_local_data.local_buffer,
        &entity_local_data.local_buffer_allocation,
        nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan uniform buffer");
}

void renderer_vulkan::_create_descriptor_set(renderer_vulkan::entity_local_data& entity_local_data) {
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
    };

    VkDescriptorPoolCreateInfo descriptor_pool_info;
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = nullptr;
    descriptor_pool_info.flags = 0;
    descriptor_pool_info.maxSets = 1;
    descriptor_pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(*pool_sizes);
    descriptor_pool_info.pPoolSizes = pool_sizes;

    RB_MAYBE_UNUSED auto result = vkCreateDescriptorPool(_device, &descriptor_pool_info, nullptr, &entity_local_data.descriptor_pool);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan descriptol pool");

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.pNext = nullptr;
    descriptor_set_allocate_info.descriptorPool = entity_local_data.descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = 1; // static_cast<std::uint32_t>(_descriptor_sets.size());
    descriptor_set_allocate_info.pSetLayouts = &_forward_descriptor_layout;

    result = vkAllocateDescriptorSets(_device, &descriptor_set_allocate_info, &entity_local_data.descriptor_set);
    RB_ASSERT(result == VK_SUCCESS, "Failed to allocate Vulkan descriptor sets");
}

void renderer_vulkan::_update_descriptor_set(renderer_vulkan::entity_local_data& entity_local_data, const std::shared_ptr<material>& material) {
    VkWriteDescriptorSet write_infos[4];
    VkDescriptorBufferInfo buffer_infos[3];
    VkDescriptorImageInfo image_infos[1];

    buffer_infos[0].buffer = _camera_buffer;
    buffer_infos[0].offset = 0;
    buffer_infos[0].range = sizeof(camera_data);

    write_infos[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_infos[0].pNext = nullptr;
    write_infos[0].dstBinding = 0;
    write_infos[0].dstArrayElement = 0;
    write_infos[0].descriptorCount = 1;
    write_infos[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_infos[0].pBufferInfo = &buffer_infos[0];
    write_infos[0].pImageInfo = nullptr;
    write_infos[0].pTexelBufferView = nullptr;
    write_infos[0].dstSet = entity_local_data.descriptor_set;

    buffer_infos[1].buffer = entity_local_data.local_buffer;
    buffer_infos[1].offset = 0;
    buffer_infos[1].range = sizeof(local_data);

    write_infos[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_infos[1].pNext = nullptr;
    write_infos[1].dstBinding = 1;
    write_infos[1].dstArrayElement = 0;
    write_infos[1].descriptorCount = 1;
    write_infos[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_infos[1].pBufferInfo = &buffer_infos[1];
    write_infos[1].pImageInfo = nullptr;
    write_infos[1].pTexelBufferView = nullptr;
    write_infos[1].dstSet = entity_local_data.descriptor_set;

    buffer_infos[2].buffer = std::static_pointer_cast<material_vulkan>(material)->buffer();
    buffer_infos[2].offset = 0;
    buffer_infos[2].range = sizeof(material_data);

    write_infos[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_infos[2].pNext = nullptr;
    write_infos[2].dstBinding = 2;
    write_infos[2].dstArrayElement = 0;
    write_infos[2].descriptorCount = 1;
    write_infos[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_infos[2].pBufferInfo = &buffer_infos[2];
    write_infos[2].pImageInfo = nullptr;
    write_infos[2].pTexelBufferView = nullptr;
    write_infos[2].dstSet = entity_local_data.descriptor_set;

    auto texture = material->albedo_map() ?
        std::static_pointer_cast<texture_vulkan>(material->albedo_map()) :
        std::static_pointer_cast<texture_vulkan>(_white_texture);

    image_infos[0].sampler = texture->sampler();
    image_infos[0].imageView = texture->image_view();
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    write_infos[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_infos[3].pNext = nullptr;
    write_infos[3].dstBinding = 3;
    write_infos[3].dstArrayElement = 0;
    write_infos[3].descriptorCount = 1;
    write_infos[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_infos[3].pBufferInfo = nullptr;
    write_infos[3].pImageInfo = &image_infos[0];
    write_infos[3].pTexelBufferView = nullptr;
    write_infos[3].dstSet = entity_local_data.descriptor_set;

    vkUpdateDescriptorSets(_device, sizeof(write_infos) / sizeof(*write_infos), write_infos, 0, nullptr);
}

void renderer_vulkan::_create_forward_pipeline() {
    VkDescriptorSetLayoutBinding layout_bindings[] = {
        { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
        { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
        { 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_info.pNext = nullptr;
    descriptor_set_layout_info.flags = 0;
    descriptor_set_layout_info.bindingCount = sizeof(layout_bindings) / sizeof(*layout_bindings);
    descriptor_set_layout_info.pBindings = layout_bindings;

    RB_MAYBE_UNUSED auto result = vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_info, nullptr, &_forward_descriptor_layout);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan descriptor set layout");

    VkPipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pNext = nullptr;
    pipeline_layout_info.flags = 0;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &_forward_descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    result = vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_forward_pipeline_layout);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan pipeline layout");

    VkVertexInputBindingDescription vertex_input_binding_desc;
    vertex_input_binding_desc.binding = 0;
    vertex_input_binding_desc.stride = sizeof(vec3f) + sizeof(vec2f) + sizeof(vec3f);
    vertex_input_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_input_attributes[] = {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, 12 },
        { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, 20 }
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.pNext = nullptr;
    vertex_input_info.flags = 0;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &vertex_input_binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = sizeof(vertex_input_attributes) / sizeof(*vertex_input_attributes);
    vertex_input_info.pVertexAttributeDescriptions = vertex_input_attributes;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.pNext = nullptr;
    input_assembly_info.flags = 0;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_graphics_device.swapchain_extent().width);
    viewport.height = static_cast<float>(_graphics_device.swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = _graphics_device.swapchain_extent();

    VkPipelineViewportStateCreateInfo viewport_state_info;
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.pNext = nullptr;
    viewport_state_info.flags = 0;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer_state_info{};
    rasterizer_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_state_info.pNext = nullptr;
    rasterizer_state_info.flags = 0;
    rasterizer_state_info.depthClampEnable = VK_FALSE;
    rasterizer_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_state_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_state_info.depthBiasEnable = VK_FALSE;
    rasterizer_state_info.depthBiasConstantFactor = 0.0f;
    rasterizer_state_info.depthBiasClamp = 0.0f;
    rasterizer_state_info.depthBiasSlopeFactor = 0.0f;
    rasterizer_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling_state_info;
    multisampling_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_state_info.pNext = nullptr;
    multisampling_state_info.flags = 0;
    multisampling_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_state_info.sampleShadingEnable = VK_FALSE;
    multisampling_state_info.minSampleShading = 0.0f;
    multisampling_state_info.pSampleMask = nullptr;
    multisampling_state_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_state_info.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info{};
    depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_info.pNext = nullptr;
    depth_stencil_state_info.flags = 0;
    depth_stencil_state_info.depthTestEnable = VK_TRUE;
    depth_stencil_state_info.depthWriteEnable = VK_TRUE;
    depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state_info{};
    color_blend_attachment_state_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state_info.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_info{};
    color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_info.logicOpEnable = VK_FALSE;
    color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_info.attachmentCount = 1;
    color_blend_state_info.pAttachments = &color_blend_attachment_state_info;
    color_blend_state_info.blendConstants[0] = 0.0f;
    color_blend_state_info.blendConstants[1] = 0.0f;
    color_blend_state_info.blendConstants[2] = 0.0f;
    color_blend_state_info.blendConstants[3] = 0.0f;

    auto vertex_shader_module = _create_shader_module(forward_vert);
    auto fragment_shader_module = _create_shader_module(forward_frag);

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
    vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = vertex_shader_module;
    vertex_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
    fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = fragment_shader_module;
    fragment_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_shader_stage_info,
        fragment_shader_stage_info
    };

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = sizeof(shader_stages) / sizeof(*shader_stages);
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterizer_state_info;
    pipeline_info.pMultisampleState = &multisampling_state_info;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_state_info;
    pipeline_info.layout = _forward_pipeline_layout;
    pipeline_info.renderPass = _graphics_device.render_pass();
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &_forward_pipeline);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan graphics pipeline");

    vkDestroyShaderModule(_device, vertex_shader_module, nullptr);
    vkDestroyShaderModule(_device, fragment_shader_module, nullptr);
}

VkShaderModule renderer_vulkan::_create_shader_module(const span<const std::uint32_t>& bytecode) {
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.codeSize = bytecode.size_bytes();
    create_info.pCode = bytecode.data();

    VkShaderModule shader_module;
    RB_MAYBE_UNUSED auto result = vkCreateShaderModule(_device, &create_info, nullptr, &shader_module);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create shader module!");

    return shader_module;
}