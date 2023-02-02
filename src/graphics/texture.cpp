#include <rabbit/graphics/texture.hpp>

using namespace rb;

texture::texture(renderer& renderer)
    : m_renderer(renderer), m_id(renderer.create_texture()) {
}

texture::~texture() {
    if (m_id != null) {
        m_renderer.destroy_texture(m_id);
    }
}

texture::texture(texture&& texture) noexcept
    : m_renderer(texture.m_renderer), m_id(texture.m_id) {
    texture.m_id = null;
}

void texture::set_data(const uvec2& size, pixel_format format, const void* pixels) {
    m_renderer.set_texture_data(m_id, size, format, pixels);
}

uvec2 texture::size() const {
    return m_renderer.get_texture_size(m_id);
}

pixel_format texture::format() const {
    return m_renderer.get_texture_format(m_id);
}
