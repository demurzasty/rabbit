
#include <rabbit/graphics/texture.hpp>
#include <rabbit/core/config.hpp>
#include <rabbit/math/vec2.hpp>
#include <rabbit/math/math.hpp>
#include <cmath>
#include <algorithm>

using namespace rb;

constexpr std::size_t calculate_mipmap_levels(const vec3u& texture_size) {
    return rb::log2(std::max(texture_size.x, texture_size.y));
}

texture::texture(const texture_desc& desc)
    : _size(desc.size)
    , _type(desc.type)
    , _format(desc.format)
    , _filter(desc.filter)
    , _wrap(desc.wrap)
    , _anisotropy(desc.anisotropy)
    , _mipmaps(desc.mipmaps > 0 ? desc.mipmaps : calculate_mipmap_levels(desc.size))
    , _layers(desc.layers)
    , _is_render_target(desc.is_render_target) {
    RB_ASSERT(_size.x > 0 || _size.y > 0 || _size.z > 0, "Size of texture must be greater than 0");
    RB_ASSERT(_format != texture_format::undefined, "Undefined texture format");
    RB_ASSERT(_filter != texture_filter::undefined, "Undefined texture filter");
    RB_ASSERT(_wrap != texture_wrap::undefined, "Undefined texture wrapping");
    RB_ASSERT(_type != texture_type::undefined, "Undefined texture type");
    RB_ASSERT(_layers > 0, "Layer count must be greater than 0");
    RB_ASSERT(_mipmaps > 0, "Mipmaps count must be greater than 0");
}

const vec3u& texture::size() const {
    return _size;
}

texture_type texture::type() const {
    return _type;
}

texture_format texture::format() const {
    return _format;
}

texture_filter texture::filter() const {
    return _filter;
}

texture_wrap texture::wrap() const {
    return _wrap;
}

std::size_t texture::anisotropy() const {
    return _anisotropy;
}

std::size_t texture::mipmaps() const {
    return _mipmaps;
}

std::size_t texture::layers() const {
    return _layers;
}

bool texture::is_render_target() const {
    return _is_render_target;
}

std::size_t texture::bytes_per_pixel() const {
    switch (_format) {
        case texture_format::r8: return 1;
        case texture_format::rg8: return 2;
        case texture_format::rgba8: return 4;
        case texture_format::bgra8: return 4;
        case texture_format::d24s8: return 4;
    }

    RB_ASSERT(false, "Incorrect texture format");
    return 0;
}
