#include <rabbit/graphics/painter.hpp>

using namespace rb;

painter::painter(renderer& renderer, const uvec2& viewport_size)
    : m_renderer(renderer), m_viewport_size(viewport_size) {
}

void painter::draw(const vec4& destination, color color) {
    vertex2d vertices[4];
    vertices[0].position = { destination.x, destination.y };
    vertices[1].position = { destination.x, destination.y + destination.w };
    vertices[2].position = { destination.x + destination.z, destination.y + destination.w };
    vertices[3].position = { destination.x + destination.z, destination.y };

    uvec2 surface_size = m_renderer.surface_size();
    vec2 scale = { surface_size.x / float(m_viewport_size.x), surface_size.y / float(m_viewport_size.y) };

    for (vertex2d& vertex : vertices) {
        vertex.position = vertex.position * scale;
    }

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    unsigned int indices[]{
        0, 1, 2,
        2, 3, 0
    };

    m_renderer.draw(null, vertices, indices);
}

void painter::draw(const texture& texture, const vec2& position, color color) {
    uvec2 size = texture.size();

    vertex2d vertices[4];
    vertices[0].position = { position.x, position.y };
    vertices[1].position = { position.x, position.y + size.y };
    vertices[2].position = { position.x + size.x, position.y + size.y };
    vertices[3].position = { position.x + size.x, position.y };

    uvec2 surface_size = m_renderer.surface_size();
    vec2 scale = { surface_size.x / float(m_viewport_size.x), surface_size.y / float(m_viewport_size.y) };

    for (vertex2d& vertex : vertices) {
        vertex.position = vertex.position * scale;
    }

    vertices[0].texcoord = { 0.0, 0.0f };
    vertices[1].texcoord = { 0.0, 1.0f };
    vertices[2].texcoord = { 1.0, 1.0f };
    vertices[3].texcoord = { 1.0, 0.0f };

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    unsigned int indices[]{
        0, 1, 2,
        2, 3, 0
    };

    m_renderer.draw(texture, vertices, indices);
}

void painter::draw(const texture& texture, const ivec4& source, const vec4& destination, color color) {
    vertex2d vertices[4];
    vertices[0].position = { destination.x, destination.y };
    vertices[1].position = { destination.x, destination.y + destination.w };
    vertices[2].position = { destination.x + destination.z, destination.y + destination.w };
    vertices[3].position = { destination.x + destination.z, destination.y };

    uvec2 surface_size = m_renderer.surface_size();
    vec2 scale = { surface_size.x / float(m_viewport_size.x), surface_size.y / float(m_viewport_size.y) };

    for (vertex2d& vertex : vertices) {
        vertex.position = vertex.position * scale;
    }

    uvec2 size = texture.size();
    vec2 inv_size = { 1.0f / size.x, 1.0f / size.y };

    vertices[0].texcoord = { source.x * inv_size.x, source.y * inv_size.y };
    vertices[1].texcoord = { source.x * inv_size.x, (source.y + source.w) * inv_size.y };
    vertices[2].texcoord = { (source.x + source.z) * inv_size.x, (source.y + source.w) * inv_size.y };
    vertices[3].texcoord = { (source.x + source.z) * inv_size.x, source.y * inv_size.y };

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    unsigned int indices[]{
        0, 1, 2,
        2, 3, 0
    };

    m_renderer.draw(texture, vertices, indices);
}

void painter::draw(const texture& texture, const ivec4& source, const vec4& destination, color color, const vec2& center, float rotation) {
    uvec2 surface_size = m_renderer.surface_size();
    vec2 scale = { surface_size.x / float(m_viewport_size.x), surface_size.y / float(m_viewport_size.y) };

    vec4 scaled_destination{
        destination.x * scale.x,
        destination.y * scale.y,
        destination.z * scale.x,
        destination.w * scale.y,
    };


    vertex2d vertices[4];
    vertices[0].position = { scaled_destination.x, scaled_destination.y };
    vertices[1].position = { scaled_destination.x, scaled_destination.y + scaled_destination.w };
    vertices[2].position = { scaled_destination.x + scaled_destination.z, scaled_destination.y + scaled_destination.w };
    vertices[3].position = { scaled_destination.x + scaled_destination.z, scaled_destination.y };

    //for (vertex2d& vertex : vertices) {
    //    vertex.position = vertex.position * scale;
    //}

    vec2 pivot = vec2(destination.x + center.x, destination.y + center.y) * scale;
    for (vertex2d& vertex : vertices) {
        vertex.position = vertex.position.rotated(pivot, rotation);
    }

    uvec2 size = texture.size();
    vec2 inv_size = { 1.0f / size.x, 1.0f / size.y };

    vertices[0].texcoord = { source.x * inv_size.x, source.y * inv_size.y };
    vertices[1].texcoord = { source.x * inv_size.x, (source.y + source.w) * inv_size.y };
    vertices[2].texcoord = { (source.x + source.z) * inv_size.x, (source.y + source.w) * inv_size.y };
    vertices[3].texcoord = { (source.x + source.z) * inv_size.x, source.y * inv_size.y };

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    unsigned int indices[]{
        0, 1, 2,
        2, 3, 0
    };

    m_renderer.draw(texture, vertices, indices);
}

void painter::draw(const font& font, unsigned char size, std::string_view text, const vec2& position, color color) {
    vec2 location = position;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const glyph& glyph = font.get_glyph(text[i], size);

        draw(font.atlas(), glyph.rect, { location.x + glyph.offset.x, location.y + glyph.offset.y, float(glyph.rect.z), float(glyph.rect.w) }, color);

        location.x += glyph.advance;

        if (i + 1 < text.size()) {
            location.x += font.get_kerning(text[i], text[i + 1], size);
        }
    }
}
