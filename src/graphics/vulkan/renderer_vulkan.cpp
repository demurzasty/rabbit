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

handle renderer::create_texture(const uvec2& size, texture_filter filter, pixel_format format) {
    handle id = m_data->textures.create();

    gpu_texture_data gpu_data;
    gpu_data.size = { float(size.x), float(size.y) };
    vku::update_buffer_index(m_data, m_data->texture_buffer_allocation, gpu_data, std::size_t(id));

    texture_data& data = m_data->textures[id];
    data = vku::create_texture(m_data, size, filter, format);

    return id;
}

void renderer::destroy_texture(handle id) {
    assert(m_data->textures.valid(id));

    vku::cleanup_texture(m_data, m_data->textures[id]);

    vku::update_buffer_index(m_data, m_data->texture_buffer_allocation, gpu_texture_data(), std::size_t(id));

    m_data->textures.destroy(id);
}

void renderer::update_texture_data(handle id, const void* pixels) {
    assert(m_data->textures.valid(id));

    texture_data& texture = m_data->textures[id];

    vku::update_texture(m_data, texture, pixels);

    VkDescriptorImageInfo descriptor_image_info{};
    descriptor_image_info.sampler = texture.sampler;
    descriptor_image_info.imageView = texture.image_view;
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write_info{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write_info.dstSet = m_data->main_descriptor_set;
    write_info.dstBinding = 3;
    write_info.dstArrayElement = std::uint32_t(id);
    write_info.descriptorCount = 1;
    write_info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_info.pImageInfo = &descriptor_image_info;
    vkUpdateDescriptorSets(m_data->device, 1, &write_info, 0, nullptr);

    gpu_texture_data gpu_data;
    gpu_data.valid = 1;
    gpu_data.size = { float(texture.size.x), float(texture.size.y) };
    vku::update_buffer_index(m_data, m_data->texture_buffer_allocation, gpu_data, std::size_t(id));
}

bool renderer::is_texture_valid(handle id) const {
    return m_data->textures.valid(id);
}

uvec2 renderer::get_texture_size(handle id) const {
    assert(m_data->textures.valid(id));

    return m_data->textures[id].size;
}

pixel_format renderer::get_texture_format(handle id) const {
    assert(m_data->textures.valid(id));

    return m_data->textures[id].format;
}

handle renderer::create_sprite() {
    handle id = m_data->gpu_sprites.create();

    vku::update_buffer_index(m_data, m_data->sprite_buffer_allocation, m_data->gpu_sprites[id], std::size_t(id));

    return id;
}

void renderer::destroy_sprite(handle id) {
    assert(m_data->gpu_sprites.valid(id));

    vku::update_buffer_index(m_data, m_data->sprite_buffer_allocation, gpu_sprite_data(), std::size_t(id));

    VkDrawIndexedIndirectCommand command;
    command.indexCount = 0;
    command.instanceCount = 0;
    command.firstIndex = 0;
    command.vertexOffset = 0;
    command.firstInstance = 0;
    vku::update_buffer_index(m_data, m_data->sprite_draw_buffer_allocation, command, std::size_t(id));

    m_data->gpu_sprites.destroy(id);
}

void renderer::set_sprite_transform(handle id, const mat2x3& transform) {
    assert(m_data->gpu_sprites.valid(id));

    m_data->gpu_sprites[id].transform = transform;

    vku::update_buffer_index(m_data, m_data->sprite_buffer_allocation, m_data->gpu_sprites[id], std::size_t(id));
}

void renderer::set_sprite_texture(handle id, handle texture_id) {
    assert(m_data->gpu_sprites.valid(id));

    m_data->gpu_sprites[id].texture_id = texture_id != null ? int(texture_id) : -1;

    vku::update_buffer_index(m_data, m_data->sprite_buffer_allocation, m_data->gpu_sprites[id], std::size_t(id));

    VkDrawIndexedIndirectCommand command;
    command.indexCount = 6;
    command.instanceCount = texture_id != null ? 1 : 0;
    command.firstIndex = 0;
    command.vertexOffset = 0;
    command.firstInstance = std::uint32_t(id);
    vku::update_buffer_index(m_data, m_data->sprite_draw_buffer_allocation, command, std::size_t(id));
}

const mat2x3& renderer::get_sprite_transform(handle id) {
    assert(m_data->gpu_sprites.valid(id));

    return m_data->gpu_sprites[id].transform;
}

handle renderer::get_sprite_texture(handle id) {
    assert(m_data->gpu_sprites.valid(id));

    gpu_sprite_data& data = m_data->gpu_sprites[id];

    return data.texture_id >= 0 ? handle(data.texture_id) : null;
}

void renderer::draw(handle texture_id, span<const vertex2d> vertices, span<const unsigned int> indices) {
    vku::update_buffer_range(m_data, m_data->canvas_vertex_buffer_allocation, vertices, m_data->canvas_vertex_buffer_offset);

    vku::update_buffer_range(m_data, m_data->canvas_index_buffer_allocation, indices, m_data->canvas_index_buffer_offset);

    draw_data command;
    command.texture_index = texture_id == null ? -1 : int(texture_id);
    command.index_offset = m_data->canvas_index_buffer_offset;
    command.index_count = (unsigned int)(indices.size());
    command.vertex_offset = m_data->canvas_vertex_buffer_offset;
    m_data->draw_commands.push_back(command);

    m_data->canvas_vertex_buffer_offset += (unsigned int)(vertices.size());
    m_data->canvas_index_buffer_offset += (unsigned int)(indices.size());
}

void renderer::display(color color) {
    vku::update_buffer(m_data, m_data->global_buffer_allocation, m_data->gpu_global_data);

    // TODO: Global buffer memory barrier.

    vertex2d vertices[4];
    vertices[0].position = { 0.0f, 0.0f };
    vertices[1].position = { 0.0f, 1.0f };
    vertices[2].position = { 1.0f, 1.0f };
    vertices[3].position = { 1.0f, 0.0f };

    vertices[0].texcoord = { 0.0f, 0.0f };
    vertices[1].texcoord = { 0.0f, 1.0f };
    vertices[2].texcoord = { 1.0f, 1.0f };
    vertices[3].texcoord = { 1.0f, 0.0f };

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    unsigned int indices[]{
        0, 1, 2,
        2, 3, 0
    };

    vku::update_buffer_range(m_data, m_data->canvas_vertex_buffer_allocation, span<const vertex2d>(vertices), m_data->canvas_vertex_buffer_offset);

    vku::update_buffer_range(m_data, m_data->canvas_index_buffer_allocation, span<const unsigned int>(indices), m_data->canvas_index_buffer_offset);

    m_data->canvas_vertex_buffer_offset += 4;
    m_data->canvas_index_buffer_offset += 6;

    vku::begin(m_data);

    VkClearValue clear_values[1];
    clear_values[0].color = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

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

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_data->command_buffers[m_data->image_index], 0, 1, &m_data->canvas_vertex_buffer, &offset);
    vkCmdBindIndexBuffer(m_data->command_buffers[m_data->image_index], m_data->canvas_index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(m_data->command_buffers[m_data->image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_data->canvas_pipeline_layout,
        0, 1, &m_data->main_descriptor_set, 0, nullptr);

    vkCmdBindPipeline(m_data->command_buffers[m_data->image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_data->canvas_pipeline);

    for (draw_data& command : m_data->draw_commands) {
        vkCmdPushConstants(m_data->command_buffers[m_data->image_index], m_data->canvas_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), &command.texture_index);

        vkCmdDrawIndexed(m_data->command_buffers[m_data->image_index],
            command.index_count,
            1,
            command.index_offset,
            command.vertex_offset,
            0);
    }

    vkCmdBindDescriptorSets(m_data->command_buffers[m_data->image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_data->sprite_pipeline_layout,
        0, 1, &m_data->main_descriptor_set, 0, nullptr);

    vkCmdBindPipeline(m_data->command_buffers[m_data->image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_data->sprite_pipeline);

    vkCmdDrawIndexedIndirect(m_data->command_buffers[m_data->image_index], m_data->sprite_draw_buffer, 0, std::uint32_t(m_data->gpu_sprites.max_size()), sizeof(VkDrawIndexedIndirectCommand));

    //m_data->gpu_sprites.each([this](handle id, gpu_sprite_data& data) {
    //    vkCmdDrawIndexed(m_data->command_buffers[m_data->image_index],
    //        6,
    //        1,
    //        0,
    //        0,
    //        std::uint32_t(id));
    //});

    vkCmdEndRenderPass(m_data->command_buffers[m_data->image_index]);

    vku::end(m_data);

    m_data->canvas_vertex_buffer_offset = 0;
    m_data->canvas_index_buffer_offset = 0;
    m_data->draw_commands.clear();
}

uvec2 renderer::surface_size() const {
    return { m_data->swapchain_extent.width, m_data->swapchain_extent.height };
}
