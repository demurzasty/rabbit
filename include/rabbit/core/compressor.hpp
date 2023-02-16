#pragma once 

#include "span.hpp"

#include <cstdint>
#include <vector>

namespace rb {
    /**
     * @brief Lossless, high performance data compressor.
     */
    class compressor {
    public:
		[[nodiscard]] std::size_t bound(std::size_t uncompressed_size) const;

        std::size_t compress(const void* uncompressed_data, std::size_t uncompressed_size, void* compressed_data, std::size_t compressed_bound) const;

        std::size_t uncompress(const void* compressed_data, std::size_t compressed_size, void* uncompressed_data, std::size_t uncompressed_size) const;
    
		template<typename T>
		[[nodiscard]] std::vector<std::uint8_t> compress(const span<const T>& uncompressed_data) const {
			std::vector<std::uint8_t> compressed_data(bound(uncompressed_data.size_bytes()));
			std::size_t compressed_size = compress(uncompressed_data.data(), uncompressed_data.size_bytes(), compressed_data.data(), compressed_data.size());
			
			if (compressed_size > 0) {
				compressed_data.resize(compressed_size);
				compressed_data.shrink_to_fit();
				return compressed_data;
			}

			return {};
		}

		template<typename T = std::uint8_t>
		[[nodiscard]] std::vector<T> uncompress(std::size_t uncompressed_size, span<const std::uint8_t> compressed_data) const {
			std::vector<T> uncompressed_data(uncompressed_size / sizeof(T));
			std::size_t size = uncompress(compressed_data.data(), compressed_data.size_bytes(), uncompressed_data.data(), uncompressed_size);
			return size > 0 ? uncompressed_data : std::vector<T>{};
		}
    };
}
