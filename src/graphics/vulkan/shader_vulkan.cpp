#include "shader_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>

#include <memory>

using namespace rb;

shader_vulkan::shader_vulkan(VkDevice device, VkRenderPass render_pass, VkExtent2D swapchain_extent, const shader_desc& desc)
    : shader(desc)
    , _device(device) {
    _create_descriptor_set_layout(desc);
    _create_pipeline_layout(desc);

    auto vertex_shader_module = _create_shader_module(desc.vertex_bytecode);
    auto fragment_shader_module = _create_shader_module(desc.fragment_bytecode);

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

    _create_pipeline(shader_stages, render_pass, swapchain_extent, desc);

    vkDestroyShaderModule(_device, fragment_shader_module, nullptr);
    vkDestroyShaderModule(_device, vertex_shader_module, nullptr);
}

shader_vulkan::~shader_vulkan() {
    vkDestroyPipeline(_device, _pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
}

VkDescriptorSetLayout shader_vulkan::descriptor_set_layout() const {
    return _descriptor_set_layout;
}

VkPipelineLayout shader_vulkan::pipeline_layout() const {
    return _pipeline_layout;
}

VkPipeline shader_vulkan::pipeline() const {
    return _pipeline;
}

void shader_vulkan::_create_descriptor_set_layout(const shader_desc& desc) {
    auto layout_bindings = std::make_unique<VkDescriptorSetLayoutBinding[]>(desc.bindings.size());
    for (std::size_t index{ 0 }; index < desc.bindings.size(); ++index) {
        layout_bindings[index].binding = static_cast<std::uint32_t>(desc.bindings[index].slot);
        layout_bindings[index].descriptorType = utils_vulkan::descriptor_type(desc.bindings[index].binding_type);
        layout_bindings[index].descriptorCount = static_cast<std::uint32_t>(desc.bindings[index].array_size);
        layout_bindings[index].stageFlags = utils_vulkan::stage_flags(desc.bindings[index].stage_flags);
        layout_bindings[index].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_info.pNext = nullptr;
    descriptor_set_layout_info.flags = 0;
    descriptor_set_layout_info.bindingCount = static_cast<std::uint32_t>(desc.bindings.size());
    descriptor_set_layout_info.pBindings = layout_bindings.get();

    RB_MAYBE_UNUSED auto result = vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_info, nullptr, &_descriptor_set_layout);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan descriptor set layout");
}

void shader_vulkan::_create_pipeline_layout(const shader_desc& desc) {
    VkPipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pNext = nullptr;
    pipeline_layout_info.flags = 0;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    RB_MAYBE_UNUSED auto result = vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_pipeline_layout);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan pipeline layout");
}

VkShaderModule shader_vulkan::_create_shader_module(const span<const std::uint32_t>& bytecode) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = bytecode.size_bytes();
    create_info.pCode = bytecode.data();

    VkShaderModule shader_module;
    RB_MAYBE_UNUSED auto result = vkCreateShaderModule(_device, &create_info, nullptr, &shader_module);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create shader module!");

    return shader_module;
}

void shader_vulkan::_create_pipeline(const span<const VkPipelineShaderStageCreateInfo>& shader_stages, VkRenderPass render_pass, VkExtent2D swapchain_extent, const shader_desc& desc) {
    VkVertexInputBindingDescription vertex_input_binding_desc;
    vertex_input_binding_desc.binding = 0;
    vertex_input_binding_desc.stride = static_cast<std::uint32_t>(desc.vertex_desc.stride());
    vertex_input_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    auto attributes = std::make_unique<VkVertexInputAttributeDescription[]>(desc.vertex_desc.size());
    for (std::size_t index{ 0 }; index < desc.vertex_desc.size(); ++index) {
        attributes[index].binding = 0;
        attributes[index].location = static_cast<std::uint32_t>(index);
        attributes[index].format = utils_vulkan::format(desc.vertex_desc[index].format);
        attributes[index].offset = static_cast<std::uint32_t>(desc.vertex_desc.offset(index));
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &vertex_input_binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(desc.vertex_desc.size());
    vertex_input_info.pVertexAttributeDescriptions = attributes.get();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain_extent.width);
    viewport.height = static_cast<float>(swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state_info{};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer_state_info{};
    rasterizer_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_state_info.depthClampEnable = VK_FALSE;
    rasterizer_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_state_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_state_info.lineWidth = 1.0f;
    rasterizer_state_info.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
    rasterizer_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_state_info.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling_state_info{};
    multisampling_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_state_info.sampleShadingEnable = VK_FALSE;
    multisampling_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info{};
    depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_info.depthTestEnable = VK_TRUE;
    depth_stencil_state_info.depthWriteEnable = VK_TRUE;
    depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state_info{};
    color_blend_attachment_state_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterizer_state_info;
    pipeline_info.pMultisampleState = &multisampling_state_info;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_state_info;
    pipeline_info.layout = _pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    RB_MAYBE_UNUSED auto result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &_pipeline);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan graphics pipeline");
}
