#include <rabbit/graphics/texture.hpp>

using namespace rb;

texture::texture(renderer& renderer)
    : m_renderer(renderer), m_id(renderer.create_texture()) {
}

texture::~texture() {
    m_renderer.destroy_texture(m_id);
}

uvec2 texture::size() const {
    return m_renderer.get_texture_size(m_id);
}

pixel_format texture::format() const {
    return m_renderer.get_texture_format(m_id);
}
