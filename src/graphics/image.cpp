#include <rabbit/graphics/image.hpp>
#include <rabbit/graphics/color.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace rb;

image_info image::info(std::string_view path) {
    int width, height, channels;
    if (stbi_info(std::string(path).c_str(), &width, &height, &channels)) {
        return {
            { (unsigned int)(width), (unsigned int)(height) },
            channels
        };
    }

    return {};
}

image image::from(std::string_view path, bool fix_alpha_border) {
    int width, height, channels;
    stbi_uc* data = stbi_load(std::string(path).c_str(), &width, &height, &channels, 4);
    assert(data);

    if (fix_alpha_border) {
        span<color> pixels((color*)data, std::size_t(width) * height);
        for (color& pixel : pixels) {
            if (pixel.a == 0) {
                pixel.r = pixel.g = pixel.b = 0;
            }
        }
    }

    return { data, { (unsigned int)width, (unsigned int)height } };
}

span<const std::uint8_t> image::pixels() const {
    return { m_pixels.get(), std::size_t(m_size.x) * m_size.y * 4 };
}

const uvec2& image::size() const {
    return m_size;
}

std::size_t image::stride() const {
    return std::size_t(m_size.x) * 4;
}

image::image(unsigned char* pixels, const uvec2& size)
    : m_pixels(pixels, &free), m_size(size) {
}
