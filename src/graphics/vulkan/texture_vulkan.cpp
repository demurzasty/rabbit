#include "texture_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>

#include <algorithm>

using namespace rb;

texture_vulkan::texture_vulkan(VkDevice device,
    const VkPhysicalDeviceProperties& physical_device_properties,
    VkQueue graphics_queue,
    VkCommandPool command_pool,
    VmaAllocator allocator,
    const texture_desc& desc)
    : texture(desc)
    , _device(device)
    , _allocator(allocator) {
    _create_image(desc);

    if (desc.pixels) {
        _update_image(graphics_queue, command_pool, desc);

        if (desc.mipmaps == 0) {
            _generate_mipmaps(graphics_queue, command_pool, desc);
        }
    }

    _create_image_view(desc);
    _create_sampler(physical_device_properties, desc);

    if (desc.is_render_target) {
        _create_render_pass(desc);
        _create_framebuffer(desc);
    }
}

texture_vulkan::~texture_vulkan() {
    if (is_render_target()) {
        for (std::size_t index{ 0 }; index < layers() * mipmaps(); ++index) {
            vkDestroyFramebuffer(_device, _framebuffers[index], nullptr);
            vkDestroyImageView(_device, _target_image_views[index], nullptr);
        }

        vkDestroyRenderPass(_device, _render_pass, nullptr);
    }

    vkDestroySampler(_device, _sampler, nullptr);
    vkDestroyImageView(_device, _image_view, nullptr);
    vmaDestroyImage(_allocator, _image, _allocation);
}

VkImage texture_vulkan::image() const {
    return _image;
}

VkImageView texture_vulkan::image_view() const {
    return _image_view;
}

VkImageView texture_vulkan::target_image_view(std::size_t layer, std::size_t mipmap) const {
    RB_ASSERT(layer < layers(), "Out of bound");
    RB_ASSERT(mipmap < mipmaps(), "Out of bound");

    return _target_image_views[layer * mipmaps() + mipmap];
}

VkSampler texture_vulkan::sampler() const {
    return _sampler;
}

VkFramebuffer texture_vulkan::framebuffer(std::size_t layer, std::size_t mipmap) const {
    RB_ASSERT(layer < layers(), "Out of bound");
    RB_ASSERT(mipmap < mipmaps(), "Out of bound");

    return _framebuffers[layer * mipmaps() + mipmap];
}

VkRenderPass texture_vulkan::render_pass() const {
    return _render_pass;
}

void texture_vulkan::_create_image(const texture_desc& desc) {
    VkImageCreateInfo image_info;
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = nullptr;
    image_info.flags = utils_vulkan::image_create_flags(type());
    image_info.imageType = utils_vulkan::image_type(type());
    image_info.format = utils_vulkan::format(format());
    image_info.extent = { desc.size.x, desc.size.y, 1 };
    image_info.mipLevels = static_cast<std::uint32_t>(mipmaps());
    image_info.arrayLayers = static_cast<std::uint32_t>(layers());
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.is_render_target) { // TODO: Cleanup.
        image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = 0;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocation_info = {};
	allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    RB_MAYBE_UNUSED auto result = vmaCreateImage(_allocator, &image_info, &allocation_info, &_image, &_allocation, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan image");
}

void texture_vulkan::_update_image(VkQueue graphics_queue, VkCommandPool command_pool, const texture_desc& desc) {
    // Create staging buffer.
    VkBufferCreateInfo buffer_info;
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.pNext = nullptr;
    buffer_info.flags = 0;
    buffer_info.size = desc.size.x * desc.size.y * bytes_per_pixel() * layers();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = 0;
    buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocation_info = {};
	allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_allocation;

    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(_allocator, &buffer_info, &allocation_info, &staging_buffer, &staging_buffer_allocation, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan buffer");

    // Transfer pixels into buffer.
    void* data;
    result = vmaMapMemory(_allocator, staging_buffer_allocation, &data);
    RB_ASSERT(result == VK_SUCCESS, "Failed to map staging buffer memory");

    std::memcpy(data, desc.pixels, buffer_info.size);

    vmaUnmapMemory(_allocator, staging_buffer_allocation);

    auto command_buffer = utils_vulkan::begin_single_time_commands(_device, command_pool);

    for (std::size_t layer{ 0 }; layer < layers(); ++layer) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = _image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = static_cast<std::uint32_t>(mipmaps());
        barrier.subresourceRange.baseArrayLayer = static_cast<std::uint32_t>(layer);
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region{};
        region.bufferOffset = desc.size.x * desc.size.y * bytes_per_pixel() * layer;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = static_cast<std::uint32_t>(layer);
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { desc.size.x, desc.size.y, 1 };

        vkCmdCopyBufferToImage(command_buffer, staging_buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = _image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = static_cast<std::uint32_t>(mipmaps());
        barrier.subresourceRange.baseArrayLayer = static_cast<std::uint32_t>(layer);
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    utils_vulkan::end_single_time_commands(_device, graphics_queue, command_pool, command_buffer);

    vmaDestroyBuffer(_allocator, staging_buffer, staging_buffer_allocation);
}

void texture_vulkan::_generate_mipmaps(VkQueue graphics_queue, VkCommandPool command_pool, const texture_desc& desc) {
    auto command_buffer = utils_vulkan::begin_single_time_commands(_device, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = _image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mip_width = (int)desc.size.x;
    int32_t mip_height = (int)desc.size.y;

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    for (uint32_t i = 1; i < mipmaps(); i++) {
        barrier.subresourceRange.baseMipLevel = i;
        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mip_width, mip_height, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(command_buffer,
            _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mip_width > 1) {
            mip_width /= 2;
        }

        if (mip_height > 1) {
            mip_height /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = static_cast<std::uint32_t>(mipmaps()) - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    utils_vulkan::end_single_time_commands(_device, graphics_queue, command_pool, command_buffer);
}

void texture_vulkan::_create_image_view(const texture_desc& desc) {
    VkImageViewCreateInfo image_view_info;
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.pNext = nullptr;
    image_view_info.flags = 0;
    image_view_info.image = _image;
    image_view_info.viewType = utils_vulkan::image_view_type(type());
    image_view_info.format = utils_vulkan::format(format());
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = static_cast<std::uint32_t>(mipmaps());
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = static_cast<std::uint32_t>(layers());

    RB_MAYBE_UNUSED auto result = vkCreateImageView(_device, &image_view_info, nullptr, &_image_view);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan image view");
}

void texture_vulkan::_create_sampler(const VkPhysicalDeviceProperties& physical_device_properties,const texture_desc& desc) {
    VkSamplerCreateInfo sampler_info;
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = nullptr;
    sampler_info.flags = 0;
    sampler_info.magFilter = utils_vulkan::filter(filter());
    sampler_info.minFilter = utils_vulkan::filter(filter());
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // TODO: Do mapping.
    sampler_info.addressModeU = utils_vulkan::sampler_address_mode(wrap());
    sampler_info.addressModeV = utils_vulkan::sampler_address_mode(wrap());
    sampler_info.addressModeW = utils_vulkan::sampler_address_mode(wrap());
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = desc.anisotropy > 1 ? VK_TRUE : VK_FALSE;
    sampler_info.maxAnisotropy = std::min(static_cast<float>(desc.anisotropy), physical_device_properties.limits.maxSamplerAnisotropy);
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(mipmaps());
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    RB_MAYBE_UNUSED auto result = vkCreateSampler(_device, &sampler_info, nullptr, &_sampler);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan sampler");
}

void texture_vulkan::_create_framebuffer(const texture_desc& desc) {
    _target_image_views = std::make_unique<VkImageView[]>(layers() * mipmaps());
    _framebuffers = std::make_unique<VkFramebuffer[]>(layers() * mipmaps());

    for (std::size_t layer{ 0 }; layer < layers(); ++layer) {
        auto size = desc.size;
        for (std::size_t mipmap{ 0 }; mipmap < mipmaps(); ++mipmap) {
            VkImageViewCreateInfo image_view_info;
            image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_info.pNext = nullptr;
            image_view_info.flags = 0;
            image_view_info.image = _image;
            image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_info.format = utils_vulkan::format(format());
            image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_info.subresourceRange.baseMipLevel = static_cast<std::uint32_t>(mipmap);
            image_view_info.subresourceRange.levelCount = 1;
            image_view_info.subresourceRange.baseArrayLayer = static_cast<std::uint32_t>(layer);
            image_view_info.subresourceRange.layerCount = 1;

            RB_MAYBE_UNUSED auto result = vkCreateImageView(_device, &image_view_info, nullptr, &_target_image_views[layer * mipmaps() + mipmap]);
            RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan image view");

            VkFramebufferCreateInfo framebuffer_info;
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.pNext = nullptr;
            framebuffer_info.flags = 0;
            framebuffer_info.renderPass = _render_pass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = &_target_image_views[layer * mipmaps() + mipmap];
            framebuffer_info.width = size.x;
            framebuffer_info.height = size.y;
            framebuffer_info.layers = 1;

            result = vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_framebuffers[layer * mipmaps() + mipmap]);
            RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan framebuffer");

            size.x = std::max(size.x / 2, 1u);
            size.y = std::max(size.y / 2, 1u);
        }
    }
}

void texture_vulkan::_create_render_pass(const texture_desc& desc) {
    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = utils_vulkan::format(format());
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pDepthStencilAttachment = VK_NULL_HANDLE;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info;
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pNext = nullptr;
    render_pass_info.flags = 0;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &subpass_dependency;

    RB_MAYBE_UNUSED const auto result = vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan framebuffer");
}
