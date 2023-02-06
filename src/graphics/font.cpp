#include <rabbit/graphics/font.hpp>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

using namespace rb;

struct font::data {
    stbtt_fontinfo info = {};
    std::vector<unsigned char> font_data;
    std::vector<color> pixels;
};

font::font(renderer& renderer, span<const unsigned char> data)
    : m_texture(renderer, { 512, 512 }, texture_filter::nearest, pixel_format::rgba8)
    , m_rect_pack({ 512, 512 })
    , m_data(new font::data()) {
    // We need to store whole data buffer in memory.
    m_data->font_data = { data.begin(), data.end() };

    // Initialize font using stored memory buffer.
    [[maybe_unused]] int result = stbtt_InitFont(&m_data->info, m_data->font_data.data(), 0);
    assert(result);

    // Setup initial transparent texture atlas.
    m_data->pixels.resize(512 * 512);
    for (std::size_t i = 0; i < 512 * 512; ++i) {
        m_data->pixels[i] = color::transparent();
    }

    // Update texture atlas.
    m_texture.update(m_data->pixels.data());
}

font::font(font&& font) noexcept
    : m_data(std::move(font.m_data))
    , m_texture(std::move(font.m_texture))
    , m_rect_pack(std::move(font.m_rect_pack))
    , m_glyphs(std::move(font.m_glyphs)) {
}

font::~font() {
    // For this specific implementation we do not need
    // this destructor at all. 
}

const glyph& font::get_glyph(unsigned int code_point, unsigned int character_size) const {
    glyph& glyph = m_glyphs[code_point];
    if (glyph.rect.z > 0 && glyph.rect.w > 0) {
        return glyph;
    }

    int width, height, xoff, yoff;
    float scale = stbtt_ScaleForPixelHeight(&m_data->info, float(character_size));
    unsigned char* bitmap = stbtt_GetCodepointBitmap(&m_data->info, 0.0f, scale, code_point, &width, &height, &xoff, &yoff);
    glyph.advance = 8.0f;
    ivec4 rect = m_rect_pack.pack({ (unsigned int)(width), (unsigned int)(height) });

    for (int y = 0; y < rect.w; ++y) {
        for (int x = 0; x < rect.z; ++x) {
            unsigned int index = (rect.y + y) * 512 + (rect.x + x);
            m_data->pixels[index] = { 255, 255, 255, bitmap[y * rect.z + x] };
        }
    }

    free(bitmap);

    m_texture.update(m_data->pixels.data());

    int advance, lsb;
    stbtt_GetCodepointHMetrics(&m_data->info, code_point, &advance, &lsb);

    glyph.advance = float(advance) * scale;
    glyph.offset = { float(xoff), float(yoff) };
    glyph.rect = rect;
    return glyph;
}


float font::get_kerning(unsigned int codepoint1, unsigned int codepoint2, unsigned int character_size) const {
    float scale = stbtt_ScaleForPixelHeight(&m_data->info, float(character_size));

    return stbtt_GetCodepointKernAdvance(&m_data->info, codepoint1, codepoint2) * scale;
}

vec2 font::get_text_size(unsigned int character_size, std::string_view text) const {
    float scale = stbtt_ScaleForPixelHeight(&m_data->info, float(character_size));

    vec2 size = { 0.0f, float(character_size) };
    for (std::size_t i = 0; i < text.size(); ++i) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&m_data->info, text[i], &advance, &lsb);

        size.x += advance * scale;

        if (i + 1 < text.size()) {
            size.x += stbtt_GetCodepointKernAdvance(&m_data->info, text[i], text[i + 1]) * scale;
        }
    }
    return size;
}

const texture& font::atlas() const {
    return m_texture;
}
