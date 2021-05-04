#include "material_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/math/vec2.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

using namespace rb;

VkShaderModule material_vulkan::_vertex_shader_module{ VK_NULL_HANDLE };
VkShaderModule material_vulkan::_fragment_shader_module{ VK_NULL_HANDLE };
std::size_t material_vulkan::_material_count{ 0 };

material_vulkan::material_vulkan(VkDevice device,
    VmaAllocator allocator,
    VkRenderPass render_pass,
    const material_desc& desc)
    : material(desc)
    , _device(device)
    , _allocator(allocator)
    , _render_pass(render_pass) {
    if (_material_count == 0) {
        _vertex_shader_module = _create_shader_module(forward_vert);
        _fragment_shader_module = _create_shader_module(forward_frag);
    }

    _create_uniform_buffer(desc);
    _create_pipeline(desc);

    _material_count++;
}

material_vulkan::~material_vulkan() {
    _material_count--;

    if (_material_count == 0) {
        vkDestroyShaderModule(_device, _vertex_shader_module, nullptr);
        vkDestroyShaderModule(_device, _fragment_shader_module, nullptr);
    }

    vkDestroyPipeline(_device, _pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);

    vmaDestroyBuffer(_allocator, _uniform_buffer, _uniform_buffer_allocation);
}

VkBuffer material_vulkan::buffer() const {
    return _uniform_buffer;
}

VkDescriptorSetLayout material_vulkan::descriptor_set_layout() const {
    return _descriptor_set_layout;
}

VkPipelineLayout material_vulkan::pipeline_layout() const {
    return _pipeline_layout;
}

VkPipeline material_vulkan::pipeline() const {
    return _pipeline;
}

void material_vulkan::_create_uniform_buffer(const material_desc& desc) {
    VkBufferCreateInfo uniform_buffer_info;
    uniform_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniform_buffer_info.pNext = nullptr;
    uniform_buffer_info.flags = 0;
    uniform_buffer_info.size = sizeof(material_data);
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
        &_uniform_buffer,
        &_uniform_buffer_allocation,
        nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan uniform buffer");

    material_data material_data;
    material_data.base_color = desc.base_color;
    material_data.roughness = desc.roughness;
    material_data.metallic = desc.metallic;

    void* data;
    result = vmaMapMemory(_allocator, _uniform_buffer_allocation, &data);
    RB_ASSERT(result == VK_SUCCESS, "Failed to map uniform buffer memory");

    memcpy(data, &material_data, sizeof(material_data));

    vmaUnmapMemory(_allocator, _uniform_buffer_allocation);
}

void material_vulkan::_create_pipeline(const material_desc& desc) {
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

    RB_MAYBE_UNUSED auto result = vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_info, nullptr, &_descriptor_set_layout);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan descriptor set layout");

    VkPipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pNext = nullptr;
    pipeline_layout_info.flags = 0;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    result = vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_pipeline_layout);
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
    viewport.width = 1280.0f; // ! FIXME
    viewport.height = 720.0f; // ! FIXME
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = { 1280, 720 }; // ! FIXME

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

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
    vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = _vertex_shader_module;
    vertex_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
    fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = _fragment_shader_module;
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
    pipeline_info.layout = _pipeline_layout;
    pipeline_info.renderPass = _render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &_pipeline);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan graphics pipeline");
}

VkShaderModule material_vulkan::_create_shader_module(const span<const std::uint32_t>& bytecode) {
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
