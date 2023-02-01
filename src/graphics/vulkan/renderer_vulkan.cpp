#include "renderer_vulkan.hpp"
#include "utils_vulkan.hpp"

using namespace rb;

renderer::renderer(window& window)
    : m_window(window), m_data(std::make_unique<data>()) {
    vku::setup(m_data, window);
}

renderer::~renderer() {
    vku::quit(m_data);
}

handle renderer::create_texture() {
    return null;
}

void renderer::destroy_texture(handle id) {
}

void renderer::set_texture_data(handle id, const uvec2& size, pixel_format format, const void* pixels) {
}

uvec2 renderer::get_texture_size(handle id) const {
    return { 0, 0 };
}

pixel_format renderer::get_texture_format(handle id) const {
    return pixel_format::undefined;
}

void renderer::display() {
    vku::begin(m_data);

    VkClearValue clear_values[1];
    clear_values[0].color = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f };

    VkRenderPassBeginInfo render_pass_begin_info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    render_pass_begin_info.renderPass = m_data->screen_render_pass;
    render_pass_begin_info.framebuffer = m_data->screen_framebuffers[m_data->image_index];
    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.renderArea.extent = m_data->swapchain_extent;
    render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
    render_pass_begin_info.pClearValues = clear_values;
    vkCmdBeginRenderPass(m_data->command_buffers[m_data->image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{ 0.0f, 0.0f, float(m_data->swapchain_extent.width), float(m_data->swapchain_extent.height), 0.0f, 1.0f };
    vkCmdSetViewport(m_data->command_buffers[m_data->image_index], 0, 1, &viewport);

    VkRect2D scissor{ { 0, 0 }, { m_data->swapchain_extent.width, m_data->swapchain_extent.height } };
    vkCmdSetScissor(m_data->command_buffers[m_data->image_index], 0, 1, &scissor);

    vkCmdEndRenderPass(m_data->command_buffers[m_data->image_index]);

    vku::end(m_data);
}
