#include "command_buffer_vulkan.hpp"
#include "graphics_device_vulkan.hpp"
#include "utils_vulkan.hpp"
#include "texture_vulkan.hpp"
#include "mesh_vulkan.hpp"
#include "material_vulkan.hpp"

#include <rabbit/math/vec4.hpp>
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

    vkCmdBeginRenderPass(_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void command_buffer_vulkan::begin_render_pass(const std::shared_ptr<texture>& render_target) {
    const auto native_render_target = std::static_pointer_cast<texture_vulkan>(render_target);

    const auto& size = native_render_target->size();

    VkClearValue clear_values[1];
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = native_render_target->render_pass();
	render_pass_begin_info.framebuffer = native_render_target->framebuffer(0);
	render_pass_begin_info.renderArea.offset = { 0, 0 };
	render_pass_begin_info.renderArea.extent = { size.x, size.y };
	render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
	render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void command_buffer_vulkan::end_render_pass() {
    vkCmdEndRenderPass(_command_buffer);
}

void command_buffer_vulkan::set_viewport(const vec4f& viewport) {
    VkViewport native_viewport;
    native_viewport.x = viewport.x;
    native_viewport.y = viewport.y;
    native_viewport.width = viewport.z;
    native_viewport.height = viewport.w;
    native_viewport.minDepth = 0.0f;
    native_viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_command_buffer, 0, 1, &native_viewport);
}

void command_buffer_vulkan::draw(const std::shared_ptr<mesh>& mesh, const std::shared_ptr<material>& material) {
    // RB_ASSERT(mesh, "Mesh is not provided");
    // RB_ASSERT(material, "Material is not provided");

    // const auto native_mesh = std::static_pointer_cast<mesh_vulkan>(mesh);
    // const auto native_material = std::static_pointer_cast<material_vulkan>(material);

    // // vkCmdBindPipeline(_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, native_material->pipeline());

    // // TODO: Bind descriptor set

    // const auto vertex_buffer = native_mesh->vertex_buffer();
    // const VkDeviceSize vertex_offset{ 0 };

    // vkCmdBindVertexBuffers(_command_buffer, 0, 1, &vertex_buffer, &vertex_offset);

    // const auto index_buffer = native_mesh->index_buffer();
    // if (index_buffer) {
    //     vkCmdBindIndexBuffer(_command_buffer, index_buffer, 0, utils_vulkan::index_type(mesh->index_type()));
    // }

    // if (index_buffer) {
    //     vkCmdDrawIndexed(_command_buffer, static_cast<std::uint32_t>(mesh->index_count()), 1, 0, 0, 0);
    // } else {
    //     vkCmdDraw(_command_buffer, static_cast<std::uint32_t>(mesh->vertex_count()), 1, 0, 0);
    // }
}

VkCommandBuffer command_buffer_vulkan::command_buffer() const {
    return _command_buffer;
}

VkFence command_buffer_vulkan::fence() const {
    return _fence;
}
