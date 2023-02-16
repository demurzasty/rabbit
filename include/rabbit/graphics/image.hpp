#pragma once 

#include "../core/span.hpp"
#include "../math/vec2.hpp"

#include <memory>
#include <string_view>

namespace rb {
    /** 
     * @brief Structure describing a image info.
     */
    struct image_info {
        /** 
         * @brief Size of the image.
         */
        uvec2 size = { 0, 0 };

        /**
         * @brief Number of channels of the image.
         */
        int channels = 0;
    };

    /**
     * @brief Object oriented wrapper of pixels buffer.
     */
    class image {
    public:
        /**
         * @brief Get image dimensions and channels without fully decoding
         *
         * @param path Image filename path to fetch.
         *
         * @return Read image info.
         */
        static image_info info(std::string_view path);

        /** 
         * @brief Load image from file.
         * 
         * @param path Image filename path to load.
         * @param fix_alpha_border Tell whether alpha border should be fixed.
         * 
         * @return Loaded image.
         */
        static image from(std::string_view path, bool fix_alpha_border = false);
        
        /**
         * @brief Disabled copy constructor.
         */
        image(const image&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        image(image&&) noexcept = default;

        /**
         * @brief Disabled copy assignment.
         */
        image& operator=(const image&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        image& operator=(image&&) noexcept = default;

        /**
         * @brief Get the image pixels buffer.
         * 
         * @return Image pixels buffer.
         */
        [[nodiscard]] span<const unsigned char> pixels() const;

        /**
         * @brief Get the image size.
         * 
         * @return Image size in pixels.
         */
        [[nodiscard]] const uvec2& size() const;

        /**
         * @brief Get image stide.
         * 
         * @return Size of row in bytes.
         */
        [[nodiscard]] std::size_t stride() const;

    private:
        /**
         * @brief Construct a new image.
         * 
         * @param pixels Pixels buffer.
         * @param size Image size in pixels.
         */
        image(unsigned char* pixels, const uvec2& size);

        /**
         * @brief Image pixels buffer.
         */
        std::unique_ptr<unsigned char, decltype(&free)> m_pixels;

        /**
         * @brief Image size in pixels.
         */
        uvec2 m_size;
    };
}
