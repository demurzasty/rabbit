#include <rabbit/graphics/canvas.hpp>

using namespace rb;

canvas::canvas(renderer& renderer)
    : m_renderer(renderer) {
}

void canvas::draw(const vec4& destination, color color) {
    vertex2d vertices[4];
    vertices[0].position = { destination.x, destination.y };
    vertices[1].position = { destination.x, destination.y + destination.w };
    vertices[2].position = { destination.x + destination.z, destination.y + destination.w };
    vertices[3].position = { destination.x + destination.z, destination.y };

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

void canvas::draw(const texture& texture, const ivec4& source, const vec4& destination, color color) {
    vertex2d vertices[4];
    vertices[0].position = { destination.x, destination.y };
    vertices[1].position = { destination.x, destination.y + destination.w };
    vertices[2].position = { destination.x + destination.z, destination.y + destination.w };
    vertices[3].position = { destination.x + destination.z, destination.y };

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

void canvas::draw(const font& font, unsigned char size, std::string_view text, const vec2& position, color color) {
    vec2 location = position;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const glyph& glyph = font.get_glyph(text[i], size);

        draw(font.atlas(), glyph.rect, { location.x + glyph.offset.x, location.y + glyph.offset.y, float(glyph.rect.z), float(glyph.rect.w) }, color);

        location.x += glyph.advance;

        if (i + 1 < text.size()) {
            location.x += font.get_kerning(text[i], text[i] + 1, size);
        }
    }
}
