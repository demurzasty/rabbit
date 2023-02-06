#include <rabbit/graphics/texture.hpp>

using namespace rb;

texture::texture(renderer& renderer, const uvec2& size, texture_filter filter, pixel_format format)
    : m_renderer(renderer), m_id(renderer.create_texture(size, filter, format)) {
}

texture::texture(texture&& texture) noexcept
    : m_renderer(texture.m_renderer), m_id(texture.m_id) {
    texture.m_id = null;
}

texture::~texture() {
    if (m_id != null) {
        m_renderer.destroy_texture(m_id);
    }
}

texture::operator handle() const {
    return m_id;
}

void texture::update(const void* pixels) {
    m_renderer.update_texture_data(m_id, pixels);
}

bool texture::valid() const {
    // We don't need to check that handle is valid within renderer.
    // We assume that once created handle in the constructor
    // makes handle valid until destructor is called.
    return m_id != null;
}

handle texture::id() const {
    return m_id;
}

uvec2 texture::size() const {
    return m_renderer.get_texture_size(m_id);
}

pixel_format texture::format() const {
    return m_renderer.get_texture_format(m_id);
}
