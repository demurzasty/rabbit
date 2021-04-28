#include "render_pass_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/graphics/render_pass_desc.hpp>

#include <memory>

using namespace rb;

render_pass_vulkan::render_pass_vulkan(VkDevice device, const render_pass_desc& desc)
    : _device(device) {
    auto color_attachments = std::make_unique<VkAttachmentDescription[]>(desc.color_attachments.size());
    auto color_references = std::make_unique<VkAttachmentReference[]>(desc.color_attachments.size());

    for (std::size_t index{ 0 }; index < desc.color_attachments.size(); ++index) {
        color_attachments[index].flags = 0;
        color_attachments[index].format = utils_vulkan::format(desc.color_attachments[index].format);
        color_attachments[index].samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachments[index].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachments[index].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachments[index].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachments[index].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        color_references[index].attachment = static_cast<std::uint32_t>(index);
        color_references[index].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentDescription depth_attachment;
    if (desc.depth_attachment.format != texture_format::undefined) {
        depth_attachment.flags = 0;
        depth_attachment.format = utils_vulkan::format(desc.depth_attachment.format);
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depth_reference;
    depth_reference.attachment = static_cast<std::uint32_t>(desc.color_attachments.size());
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<std::uint32_t>(desc.color_attachments.size());
    subpass.pColorAttachments = color_references.get();
    if (desc.depth_attachment.format != texture_format::undefined) {
        subpass.pDepthStencilAttachment = &depth_reference;
    } else {
        subpass.pDepthStencilAttachment = nullptr;
    }

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[12];
    for (std::size_t index{ 0 }; index < desc.color_attachments.size(); ++index) {
        attachments[index] = color_attachments[index];
    }

    attachments[desc.color_attachments.size()] = depth_attachment;

    VkRenderPassCreateInfo render_pass_info;
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pNext = nullptr;
    render_pass_info.flags = 0;
    if (desc.depth_attachment.format == texture_format::undefined) {
        render_pass_info.attachmentCount = static_cast<std::uint32_t>(desc.color_attachments.size());
    } else {
        render_pass_info.attachmentCount = static_cast<std::uint32_t>(desc.color_attachments.size()) + 1;
    }
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &subpass_dependency;

    RB_MAYBE_UNUSED const auto result = vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan framebuffer");
}

render_pass_vulkan::~render_pass_vulkan() {
    vkDestroyRenderPass(_device, _render_pass, nullptr);
}

VkRenderPass render_pass_vulkan::render_pass() const {
    return _render_pass;
}