#include <rabbit/graphics/texture.hpp>

using namespace rb;

texture::texture(renderer& renderer)
    : m_renderer(&renderer), m_id(renderer.create_texture()) {
}

texture::~texture() {
    if (m_id != null) {
        // We assume that if id is valid that renderer is valid also.
        m_renderer->destroy_texture(m_id);
    }
}

texture::texture(texture&& texture) noexcept
    : m_renderer(texture.m_renderer), m_id(texture.m_id) {
    texture.m_renderer = nullptr;
    texture.m_id = null;
}

void texture::set_data(const uvec2& size, pixel_format format, const void* pixels) {
    assert(valid());

    m_renderer->set_texture_data(m_id, size, format, pixels);
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
    assert(valid());

    return m_renderer->get_texture_size(m_id);
}

pixel_format texture::format() const {
    assert(valid());

    return m_renderer->get_texture_format(m_id);
}
