#pragma once 

#include "image.hpp"

#include <vector>
#include <cstddef>

namespace rb {
    /** 
     * @brief S3TC compressor.
     */
    class s3tc {
    public:
    public:
        void bc1(const void* uncompressed_pixels, std::size_t uncompressed_size, std::size_t stride, void* compressed_pixels) const;

        void bc3(const void* uncompressed_pixels, std::size_t uncompressed_size, std::size_t stride, void* compressed_pixels) const;

        [[nodiscard]] std::vector<std::uint8_t> bc1(const image& image) const;

        [[nodiscard]] std::vector<std::uint8_t> bc3(const image& image) const;
    };
}
