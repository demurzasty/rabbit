#include <rabbit/graphics/texture.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

texture::texture(const texture_desc& desc)
    : _size(desc.size)
    , _format(desc.format) {
    RB_ASSERT(_size.x > 0 && _size.y > 0, "Size of texture must be greater than 0");
}

const vec2u& texture::size() const {
    return _size;
}

vec2f texture::texel() const {
    return { 1.0f / _size.x, 1.0f / _size.y };
}

texture_format texture::format() const {
    return _format;
}

std::size_t texture::bytes_per_pixel() const {
    switch (_format) {
        case texture_format::r8: return 1;
        case texture_format::rg8: return 2;
        case texture_format::rgba8: return 4;
        case texture_format::d24s8: return 4;
    }

    RB_ASSERT(false, "Incorrect texture format");
    return 0;
}
