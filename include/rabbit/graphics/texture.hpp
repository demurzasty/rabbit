#pragma once

#include "../math/vec3.hpp"

#include <cstddef>

namespace rb {
    /**
     * @brief Texture type enumeration.
     */
    enum class texture_type {
        undefined,
        texture_2d,
        texture_cube
    };

    /**
     * @brief Sampling filter enumeration.
     */
    enum class texture_filter {
        undefined,

        /**
         * @brief Take the nearest texture sample.
         */
        nearest,

        /**
         * @brief Interpolate between multiple texture samples.
         */
        linear
    };

    enum class texture_wrap {
        undefined,

        /**
         * @brief Clamp texture coordinates to the interval [0, 1].
         */
        clamp,

        /**
         * @brief Repeat texture coordinates within the interval [0, 1)
         */
        repeat
    };

    /**
     * @brief Defines various types of texture formats.
     */
    enum class texture_format {
        undefined,

        /**
         * @brief 8-bit R pixel format with one channel
         */
        r8,

        /**
         * @brief 16-bit RG pixel format, using 8 bits per channel.
         */
        rg8,

        /**
         * @brief 32-bit RGBA pixel format with alpha, using 8 bits per channel.
         */
        rgba8,

        /**
         * @brief 32-bit RGBA pixel format with alpha, using 8 bits per channel.
         */
        bgra8,

        /**
         * @brief 32-bit depth-stencil pixel format, using 24 bits for depth, and 8 bits for stencil.
         */
        d24s8
    };

    /**
     * @brief Texture creation descriptor.
     */
    struct texture_desc {
        /**
         * @brief Data to transfer to graphhics device.
         */
        const void* pixels{ nullptr };

        /**
         * @brief Determine texture size.
         */
        vec3u size{ 0, 0, 0 };

        /**
         * @brief Determine texture type.
         */
        texture_type type{ texture_type::undefined };

        /**
         * @brief Determine texture filter.
         */
        texture_filter filter{ texture_filter::undefined };

        /**
         * @brief Determine texture wrapping.
         */
        texture_wrap wrap{ texture_wrap::undefined };

        /**
         * @brief Determine texture pixels format.
         */
        texture_format format{ texture_format::undefined };

        /**
         * @brief Anisotropic filtering level.
         */
        std::size_t anisotropy{ 1 };

        /**
         * @brief Determine mipmap count.
         *
         *        Provide 0 to generate mipmaps.
         */
        std::size_t mipmaps{ 1 };

        /**
         * @brief Determine texture layers.
         *
         *        For cubemap provide 6 layers as faces.
         */
        std::size_t layers{ 1 };

        /**
         * @brief Determine that texture will be treated as a render target.
         */
        bool is_render_target{ false };
    };

    /**
     * @brief Texture resource interface.
     */
    class texture {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~texture() = default;

        /**
         * @brief Returns texture size.
         */
        const vec3u& size() const;

        /**
         * @brief Returns texture type.
         */
        texture_type type() const;

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
         * @brief Returns anisotropy level.
         */
        std::size_t anisotropy() const;

        /**
         * @brief Returns mipmap level count.
         */
        std::size_t mipmaps() const;

        /**
         * @brief Returns layer count.
         */
        std::size_t layers() const;

        /**
         * @brief Tell whether texture is render target.
         */
        bool is_render_target() const;

        /**
         * @brief Returns bytes per pixel.
         */
        std::size_t bytes_per_pixel() const;

    protected:
        /**
         * @brief Protected constructor.
         */
        texture(const texture_desc& desc);

    private:
        const vec3u _size;
        const texture_type _type;
        const texture_format _format;
        const texture_filter _filter;
        const texture_wrap _wrap;
        const std::size_t _anisotropy;
        const std::size_t _mipmaps;
        const std::size_t _layers;
        const bool _is_render_target;
    };
}
