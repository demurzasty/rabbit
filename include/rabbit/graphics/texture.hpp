#pragma once

#include "texture_desc.hpp"

#include <cstddef>

namespace rb {
    class texture {
    public:
        texture(const texture_desc& desc);

        virtual ~texture() = default;

        const vec2u& size() const;

        vec2f texel() const;

        texture_format format() const;

        std::size_t bytes_per_pixel() const;

    private:
        const vec2u _size;
        const texture_format _format;
    };
}
