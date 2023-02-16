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

	VkFilter get_filter(texture_filter filter);

	texture_data create_texture(std::unique_ptr<renderer::data>& data, const uvec2& size, texture_filter filter, pixel_format format);

	void update_texture(std::unique_ptr<renderer::data>& data, texture_data& texture, const void* pixels);

	void cleanup_texture(std::unique_ptr<renderer::data>& data, texture_data& texture);
}
