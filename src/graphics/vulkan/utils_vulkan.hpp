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

	void begin(std::unique_ptr<renderer::data>& data);

	void end(std::unique_ptr<renderer::data>& data);
}
