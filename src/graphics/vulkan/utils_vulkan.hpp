#pragma once 

#include "renderer_vulkan.hpp"

#include <cassert>

#ifdef _DEBUG
#	define vk(expr) do { \
			[[maybe_unused]] const VkResult result = (expr); \
			assert(result == VK_SUCCESS); \
		} while (0)
#else
#	define vk(expr) (expr)
#endif

namespace rb::vku {
	void setup(std::unique_ptr<renderer::data>& data, window& window);

	void quit(std::unique_ptr<renderer::data>& data);

	void cleanup(std::unique_ptr<renderer::data>& data);

	void begin(std::unique_ptr<renderer::data>& data);

	void end(std::unique_ptr<renderer::data>& data);

	VkDeviceSize get_bits_per_pixel(pixel_format format);

	VkFormat get_pixel_format(pixel_format format);

	VkFilter get_filter(texture_filter filter);

	texture_data create_texture(std::unique_ptr<renderer::data>& data, const uvec2& size, texture_filter filter, pixel_format format);

	void update_texture(std::unique_ptr<renderer::data>& data, texture_data& texture, const void* pixels);

	void cleanup_texture(std::unique_ptr<renderer::data>& data, texture_data& texture);

	void clear_buffer(std::unique_ptr<renderer::data>& data, VmaAllocation allocation, std::size_t size_bytes);

	void update_buffer(std::unique_ptr<renderer::data>& data, VmaAllocation allocation, const void* src_data, std::size_t src_size_bytes, std::size_t dst_offset_bytes);

	template<typename T>
	void update_buffer(std::unique_ptr<renderer::data>& data, VmaAllocation allocation, const T& src_data) {
		update_buffer(data, allocation, &src_data, sizeof(T), sizeof(T));
	}

	template<typename T>
	void update_buffer_index(std::unique_ptr<renderer::data>& data, VmaAllocation allocation, const T& src_data, std::size_t dst_offset) {
		update_buffer(data, allocation, &src_data, sizeof(T), dst_offset * sizeof(T));
	}

	template<typename T>
	void update_buffer_range(std::unique_ptr<renderer::data>& data, VmaAllocation allocation, span<const T> src_data, std::size_t dst_offset) {
		update_buffer(data, allocation, src_data.data(), src_data.size_bytes(), dst_offset * sizeof(T));
	}
}
