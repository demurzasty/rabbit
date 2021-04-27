#pragma once

#include "texture_desc.hpp"
#include "../math/fwd.hpp"

#include <cstddef>

namespace rb {
    class texture {
    public:
        virtual ~texture() = default;

        /**
         * @brief Returns texture size.
         */
        const vec3u& size() const;

        /**
         * @brief Returns texture texel size.
         */
        vec2f texel() const;

        /**
         * @brief Returns texture format.
         */
        texture_format format() const;

        /**
         * @brief Returns texture filter.
         */
        texture_filter filter() const;

        /**
         * @brief Returns texture wrap.
         */
        texture_wrap wrap() const;

        /**
         * @brief Returns mipmap level count.
         */
        std::size_t mipmaps() const;

        /**
         * @brief Returns bytes per pixel.
         */
        std::size_t bytes_per_pixel() const;

    protected:
        texture(const texture_desc& desc);

    private:
        const vec3u _size;
        const texture_format _format;
        const texture_filter _filter;
        const texture_wrap _wrap;
        const std::size_t _mipmaps;
    };
}
