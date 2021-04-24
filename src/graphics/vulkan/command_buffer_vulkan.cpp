#include "command_buffer_vulkan.hpp"
#include "graphics_device_vulkan.hpp"
#include "shader_vulkan.hpp"

#include <rabbit/core/config.hpp>

using namespace rb;

command_buffer_vulkan::command_buffer_vulkan(VkDevice device, VkCommandPool command_pool)
    : _device(device)
    , _command_pool(command_pool) {
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

command_buffer_vulkan::~command_buffer_vulkan() {
    vkWaitForFences(_device, 1, &_fence, VK_TRUE, 1000000000);
    vkDestroyFence(_device, _fence, nullptr);
    vkFreeCommandBuffers(_device, _command_pool, 1, &_command_buffer);
}

void command_buffer_vulkan::begin() {
    RB_MAYBE_UNUSED auto result = vkWaitForFences(_device, 1, &_fence, VK_TRUE, 1000000000);
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

void command_buffer_vulkan::end() {
    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    RB_MAYBE_UNUSED auto result = vkEndCommandBuffer(_command_buffer);
    RB_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
}

void command_buffer_vulkan::begin_render_pass(const std::shared_ptr<graphics_device>& graphics_device) {
    auto native_graphics_device = std::static_pointer_cast<graphics_device_vulkan>(graphics_device);

    VkClearValue clear_values[2];
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = native_graphics_device->render_pass();
	render_pass_begin_info.framebuffer = native_graphics_device->framebuffer();
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = native_graphics_device->swapchain_extent();
	render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
	render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void command_buffer_vulkan::end_render_pass() {
    vkCmdEndRenderPass(_command_buffer);
}

void command_buffer_vulkan::set_shader(const std::shared_ptr<shader>& shader) {
    const auto native_shader = std::static_pointer_cast<shader_vulkan>(shader);

    vkCmdBindPipeline(_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, native_shader->pipeline());
}

VkCommandBuffer command_buffer_vulkan::command_buffer() const {
    return _command_buffer;
}

VkFence command_buffer_vulkan::fence() const {
    return _fence;
}
