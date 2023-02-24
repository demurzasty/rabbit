#pragma once 

#include "texture.hpp"
#include "rect_pack.hpp"
#include "../core/span.hpp"
#include "../core/reference.hpp"

#include <memory>
#include <unordered_map>

namespace rb {
    /**
     * @brief Structure describing a font glyph.
     */
    struct glyph {
        /**
         * @brief Offset to move horizontally to the next character.
         */
        float advance;

        /**
         * @brief Texture coordinates of the glyph inside the font's texture.
         */
        irect rect;

        /**
         * @brief Glyph texture offset.
         */
        vec2 offset;
    };

    /**
     * @brief Class for loading and manipulating character fonts.
     */
    class font : public reference {
    public:
        /**
         * @brief Disable default construction.
         */
        font() = delete;

        /**
         * @brief Construct a new font.
         */
        font(renderer& renderer, span<const unsigned char> data);

        /**
         * @brief Disabled copy constructor.
         */
        font(const font&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        font(font&& font) noexcept;

        /**
         * @brief Destructor of the font.
         */
        ~font();

        /**
         * @brief Disabled copy assignment.
         */
        font& operator=(const font&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        font& operator=(font&&) = delete;

        /**
         * @brief Retrieve a glyph of the font.
         * 
         * @return Codepoint glyph.
         */
        const glyph& get_glyph(unsigned int codepoint, unsigned int character_size) const;

        /**
         * @brief Retrieve a kerning between two codepoints.
         * 
         * @return Kerning in pixels.
         */
        float get_kerning(unsigned int codepoint1, unsigned int codepoint2, unsigned int character_size) const;

        /**
         * @brief Get size of text.
         * 
         * @return Size of text.
         */
        vec2 get_text_size(unsigned int character_size, std::string_view text) const;

        /**
         * @brief Get the texture containing the loaded glyphs of a certain size.
         */
        const texture& atlas() const;

    private:
        /**
         * @brief Impementation defined data structure.
         */
        struct data;

        /**
         * @brief Texture atlas of glyphs.
         */
        mutable texture m_texture;

        /**
         * @brief Glyphs rectangle packer.
         */
        mutable rect_pack m_rect_pack;

        /**
         * @brief Glyphs map.
         */
        mutable std::unordered_map<unsigned int, glyph> m_glyphs;

        /**
         * @brief Impementation defined data pointer.
         */
        std::unique_ptr<data> m_data;
    };
}
