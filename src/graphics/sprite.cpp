#include <rabbit/graphics/sprite.hpp>

using namespace rb;

sprite::sprite(renderer& renderer)
    : m_renderer(renderer), m_id(renderer.create_sprite()) {
}

sprite::sprite(renderer& renderer, ref<rb::texture> texture)
    : m_renderer(renderer), m_id(renderer.create_sprite()) {
    set_texture(texture);
}

sprite::sprite(sprite&& sprite) noexcept
    : m_renderer(sprite.m_renderer), m_id(sprite.m_id) {
    sprite.m_id = null;
}

sprite::~sprite() {
    if (m_id != null) {
        m_renderer.destroy_sprite(m_id);
    }
}

sprite::operator handle() const {
    return m_id;
}

void sprite::set_transform(const mat2x3& transform) {
    m_renderer.set_sprite_transform(m_id, transform);
}

const mat2x3& sprite::transform() const {
    return m_renderer.get_sprite_transform(m_id);
}

void sprite::set_texture(const ref<rb::texture>& texture) {
    m_renderer.set_sprite_texture(m_id, texture ? texture->id() : null);
    m_texture = texture;
}

const ref<texture>& sprite::texture() const {
    return m_texture;
}
