#pragma once

#include "../math/vec2.hpp"

#include <cstddef>

namespace rb {
    enum class texture_format {
        r8,
        rg8,
        rgba8,
        d24s8
    };

    struct texture_desc {
        const void* data{ nullptr };
        vec2u size{ 0, 0 };
        texture_format format{ texture_format::rgba8 };
    };

    class texture {
    public:
        texture(const texture_desc& desc);

        virtual ~texture() = default;

        const vec2u& size() const;

        vec2f texel() const;

        texture_format format() const;

        std::size_t bytes_per_pixel() const;

    private:
        vec2u _size;
        texture_format _format;
    };
}
