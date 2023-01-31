#pragma once 

#include "config.hpp"
#include "memory.hpp"
#include "span.hpp"

#include <string_view>

namespace rb {
    class image : public non_copyable {
    public:
        static image from(std::string_view p_path);

    public:
        image(image&&) noexcept = default;

        image& operator=(image&&) noexcept = default;

        span<const std::uint8_t> pixels() const {
            return { m_pixels.get(), std::size_t(m_width) * m_height * 4 };
        }

        int width() const {
            return m_width;
        }

        int height() const {
            return m_height;
        }

    private:
        image(std::uint8_t* p_pixels, int p_width, int p_height)
            : m_pixels(p_pixels, &free), m_width(p_width), m_height(p_height) {
        }

    private:
        std::unique_ptr<std::uint8_t, decltype(&free)> m_pixels;
        int m_width = 0;
        int m_height = 0;
    };
}