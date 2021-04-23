#pragma once

#include <rabbit/graphics/graphics_device.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class graphics_device_vulkan : public graphics_device {
    public:
        graphics_device_vulkan(const graphics_device_desc& desc);

        ~graphics_device_vulkan();

    private:
        void _initialize_volk(const graphics_device_desc& desc);

        void _create_instance(const graphics_device_desc& desc);

        void _choose_physical_device(const graphics_device_desc& desc);

        void _create_surface(const graphics_device_desc& desc);

        void _create_device(const graphics_device_desc& desc);

        void _create_allocator(const graphics_device_desc& desc);

        void _query_surface(const graphics_device_desc& desc);

    private:
        VkInstance _instance;
        VkPhysicalDevice _physical_device;
        VkSurfaceKHR _surface;
        VkDevice _device;
        std::uint32_t _graphics_family;
        std::uint32_t _present_family;
        VkQueue _graphics_queue;
        VkQueue _present_queue;
        VmaAllocator _allocator;
        VkSurfaceFormatKHR _surface_format;
        VkExtent2D _swapchain_extent;
    };
}
