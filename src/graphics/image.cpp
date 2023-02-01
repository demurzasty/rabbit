#include <rabbit/graphics/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace rb;

image image::from(std::string_view path) {
    int width, height, comp;
    stbi_uc* pixels = stbi_load(std::string(path).c_str(), &width, &height, &comp, 4);
    assert(pixels);
    return { pixels, { (unsigned int)width, (unsigned int)height } };
}

span<const std::uint8_t> image::pixels() const {
    return { m_pixels.get(), std::size_t(m_size.x) * m_size.y * 4 };
}

image::image(unsigned char* pixels, const uvec2& size)
    : m_pixels(pixels, &free), m_size(size) {
}
