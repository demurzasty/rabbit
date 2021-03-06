#include "command_buffer_vulkan.hpp"
#include "graphics_device_vulkan.hpp"
#include "utils_vulkan.hpp"
#include "texture_vulkan.hpp"
#include "buffer_vulkan.hpp"
#include "resource_heap_vulkan.hpp"
#include "shader_vulkan.hpp"

#include <rabbit/math/vec4.hpp>
#include <rabbit/core/config.hpp>

#include <algorithm>

using namespace rb;

constexpr std::size_t native_buffer_count{ 3 };

command_buffer_vulkan::command_buffer_vulkan(VkDevice device, VkCommandPool command_pool)
    : _device(device)
    , _command_pool(command_pool) {
    _command_buffers = std::make_unique<VkCommandBuffer[]>(native_buffer_count);

    VkCommandBufferAllocateInfo command_buffer_alloc_info;
    command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_alloc_info.pNext = nullptr;
    command_buffer_alloc_info.commandPool = _command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_alloc_info.commandBufferCount = static_cast<std::uint32_t>(native_buffer_count);

    RB_MAYBE_UNUSED auto result = vkAllocateCommandBuffers(_device, &command_buffer_alloc_info, _command_buffers.get());
    RB_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffer");

    _fences = std::make_unique<VkFence[]>(native_buffer_count);

    VkFenceCreateInfo fence_info;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t index{ 0 }; index < native_buffer_count; ++index) {
        result = vkCreateFence(_device, &fence_info, nullptr, &_fences[index]);
        RB_ASSERT(result == VK_SUCCESS, "Failed to create fence");
    }
}

command_buffer_vulkan::~command_buffer_vulkan() {
    for (std::size_t index{ 0 }; index < native_buffer_count; ++index) {
        vkWaitForFences(_device, 1, &_fences[index], VK_TRUE, 1000000000);
        vkDestroyFence(_device, _fences[index], nullptr);
    }
    vkFreeCommandBuffers(_device, _command_pool, static_cast<std::uint32_t>(native_buffer_count), _command_buffers.get());
}

void command_buffer_vulkan::begin() {
    _index = (_index + 1) % native_buffer_count;

    RB_MAYBE_UNUSED auto result = vkWaitForFences(_device, 1, &_fences[_index], VK_TRUE, 1000000000);
    RB_ASSERT(result == VK_SUCCESS, "Failed to wait for render fence");

    result = vkResetFences(_device, 1, &_fences[_index]);
    RB_ASSERT(result == VK_SUCCESS, "Failed to reset render fence");

    // Now that we are sure that the commands finished executing,
    // we can safely reset the command buffer to begin recording again.
    result = vkResetCommandBuffer(_command_buffers[_index], 0);
    RB_ASSERT(result == VK_SUCCESS, "Failed to reset command buffer");

    // Begin the command buffer recording.
    // We will use this command buffer exactly once, so we want to let Vulkan know that.
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    result = vkBeginCommandBuffer(_command_buffers[_index], &begin_info);
    RB_ASSERT(result == VK_SUCCESS, "Failed to begin command buffer");
}

void command_buffer_vulkan::end() {
    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    RB_MAYBE_UNUSED auto result = vkEndCommandBuffer(_command_buffers[_index]);
    RB_ASSERT(result == VK_SUCCESS, "Failed to end command buffer");
}

void command_buffer_vulkan::begin_render_pass(graphics_device& graphics_device) {
    const auto& native_graphics_device = static_cast<graphics_device_vulkan&>(graphics_device);

    VkClearValue clear_values[2];
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = native_graphics_device.render_pass();
	render_pass_begin_info.framebuffer = native_graphics_device.framebuffer();
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = native_graphics_device.swapchain_extent();
	render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
	render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(_command_buffers[_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void command_buffer_vulkan::begin_render_pass(const std::shared_ptr<texture>& render_target, std::size_t layer, std::size_t mipmap) {
    const auto native_render_target = std::static_pointer_cast<texture_vulkan>(render_target);

    auto size = native_render_target->size();

    for (std::size_t index{ 0 }; index < mipmap; ++index) {
        size.x = std::max(size.x / 2, 1u);
        size.y = std::max(size.y / 2, 1u);
    }

    VkClearValue clear_values[1];
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = native_render_target->render_pass();
	render_pass_begin_info.framebuffer = native_render_target->framebuffer(layer, mipmap);
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = { size.x, size.y };
	render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
	render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(_command_buffers[_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void command_buffer_vulkan::end_render_pass() {
    vkCmdEndRenderPass(_command_buffers[_index]);
}

void command_buffer_vulkan::update_buffer(const std::shared_ptr<buffer>& buffer, const void* data, std::size_t offset, std::size_t size) {
    auto native_buffer = std::static_pointer_cast<buffer_vulkan>(buffer)->buffer();
    vkCmdUpdateBuffer(_command_buffers[_index], native_buffer, offset, size, data);
}

void command_buffer_vulkan::set_viewport(const vec4f& viewport) {
    VkViewport native_viewport;
    native_viewport.x = viewport.x;
    native_viewport.y = viewport.y;
    native_viewport.width = viewport.z;
    native_viewport.height = viewport.w;
    native_viewport.minDepth = 0.0f;
    native_viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_command_buffers[_index], 0, 1, &native_viewport);
}

void command_buffer_vulkan::set_shader(const std::shared_ptr<shader>& shader) {
    const auto native_shader = std::static_pointer_cast<shader_vulkan>(shader);

    vkCmdBindPipeline(_command_buffers[_index], VK_PIPELINE_BIND_POINT_GRAPHICS, native_shader->pipeline());
}

void command_buffer_vulkan::set_resource_heap(const std::shared_ptr<resource_heap>& resource_heap) {
    const auto native_resource_heap = std::static_pointer_cast<resource_heap_vulkan>(resource_heap);
    const auto native_shader = std::static_pointer_cast<shader_vulkan>(resource_heap->shader());

    const auto descriptor_set = native_resource_heap->descriptor_set();

    vkCmdBindDescriptorSets(_command_buffers[_index],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        native_shader->pipeline_layout(),
        0,
        1,
        &descriptor_set,
        0,
        nullptr);
}

void command_buffer_vulkan::set_vertex_buffer(const std::shared_ptr<buffer>& vertex_buffer) {
    const auto native_buffer = std::static_pointer_cast<buffer_vulkan>(vertex_buffer);

    const auto buffer = native_buffer->buffer();
    const VkDeviceSize offset{ 0 };

    vkCmdBindVertexBuffers(_command_buffers[_index], 0, 1, &buffer, &offset);
}

void command_buffer_vulkan::set_index_buffer(const std::shared_ptr<buffer>& index_buffer) {
    const auto native_buffer = std::static_pointer_cast<buffer_vulkan>(index_buffer);
    const auto index_type = native_buffer->stride() == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    vkCmdBindIndexBuffer(_command_buffers[_index], native_buffer->buffer(), 0, index_type);
}

void command_buffer_vulkan::draw(std::size_t vertex_count, std::size_t instance_count, std::size_t first_vertex, std::size_t first_instance) {
    vkCmdDraw(_command_buffers[_index],
        static_cast<std::uint32_t>(vertex_count),
        static_cast<std::uint32_t>(instance_count),
        static_cast<std::uint32_t>(first_vertex),
        static_cast<std::uint32_t>(first_instance));
}

void command_buffer_vulkan::draw_indexed(std::size_t index_count, std::size_t instance_count, std::size_t first_index, std::size_t vertex_offset, std::size_t first_instance) {
    vkCmdDrawIndexed(_command_buffers[_index],
        static_cast<std::uint32_t>(index_count),
        static_cast<std::uint32_t>(instance_count),
        static_cast<std::uint32_t>(first_index),
        static_cast<std::int32_t>(vertex_offset),
        static_cast<std::uint32_t>(first_instance));
}


VkCommandBuffer command_buffer_vulkan::command_buffer() const {
    return _command_buffers[_index];
}

VkFence command_buffer_vulkan::fence() const {
    return _fences[_index];
}
