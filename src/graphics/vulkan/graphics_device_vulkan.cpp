#include "graphics_device_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/platform/window.hpp>

#if RB_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using namespace rb;

// TODO: Move surface creation into separate classes for every platform.

namespace {
    const char* validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    VKAPI_ATTR
    VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::fprintf(stderr, "%s\n", pCallbackData->pMessage);
        return VK_FALSE;
    }
}

graphics_device_vulkan::graphics_device_vulkan(const graphics_device_desc& desc)
    : graphics_device(desc) {
    _initialize_volk(desc);
    _create_instance(desc);
    _choose_physical_device(desc);
    _create_surface(desc);
    _create_device(desc);
    _create_allocator(desc);
    _query_surface(desc);
}

graphics_device_vulkan::~graphics_device_vulkan() {
    vmaDestroyAllocator(_allocator);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
}

void graphics_device_vulkan::_initialize_volk(const graphics_device_desc& desc) {
    RB_MAYBE_UNUSED const auto result = volkInitialize();
    RB_ASSERT(result == VK_SUCCESS, "Cannot initialize volk library");
}

void graphics_device_vulkan::_create_instance(const graphics_device_desc& desc) {
    const auto [major, minor, patch] = desc.application_version;

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = desc.application_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
    app_info.pEngineName = "RabBit";
    app_info.engineVersion = VK_MAKE_VERSION(RB_VERSION_MAJOR, RB_VERSION_MINOR, RB_VERSION_PATCH);
    app_info.apiVersion = VK_API_VERSION_1_0;

    const char* enabled_extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if RB_WINDOWS
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
    };

    VkDebugUtilsMessengerCreateInfoEXT debug_info;
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.pNext = nullptr;
    debug_info.flags = 0;
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = &debug_callback;
    debug_info.pUserData = nullptr;

    VkInstanceCreateInfo instance_info;
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = &debug_info;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = sizeof(validation_layers) / sizeof(*validation_layers);
    instance_info.ppEnabledLayerNames = validation_layers;
    instance_info.enabledExtensionCount = sizeof(enabled_extensions) / sizeof(*enabled_extensions);
    instance_info.ppEnabledExtensionNames = enabled_extensions;

    RB_MAYBE_UNUSED const auto result = vkCreateInstance(&instance_info, nullptr, &_instance);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan instance");

    volkLoadInstance(_instance);
}

void graphics_device_vulkan::_choose_physical_device(const graphics_device_desc& desc) {
    // Query physical device count. We should pick one.
    std::uint32_t physical_device_count{ 0 };
    RB_MAYBE_UNUSED auto result = vkEnumeratePhysicalDevices(_instance, &physical_device_count, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to query the number of physical devices");

    // No supported physical devices?
    RB_ASSERT(physical_device_count > 0, "Couldn't detect any physical device with Vulkan support");

    // Enumerate through physical devices to pick one.
    auto physical_devices = std::make_unique<VkPhysicalDevice[]>(physical_device_count);
    result = vkEnumeratePhysicalDevices(_instance, &physical_device_count, physical_devices.get());
    RB_ASSERT(result == VK_SUCCESS, "Failed to enumarate physical devices");

    // TODO: We should find best device.
    _physical_device = physical_devices[0];
}

void graphics_device_vulkan::_create_surface(const graphics_device_desc& desc) {
#if RB_WINDOWS
    // Fill Win32 surface create informations.
    VkWin32SurfaceCreateInfoKHR surface_info;
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = nullptr;
    surface_info.flags = 0;
    surface_info.hinstance = GetModuleHandle(nullptr);
    surface_info.hwnd = associated_window()->native_handle();

    // Create new Vulkan surface.
    RB_MAYBE_UNUSED const auto result = vkCreateWin32SurfaceKHR(_instance, &surface_info, nullptr, &_surface);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan surface");
#else
    RB_ASSERT(false, "Graphics for that platform is not implemented");
#endif
}

void graphics_device_vulkan::_create_device(const graphics_device_desc& desc) {
    std::uint32_t queue_family_count{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, nullptr);

    auto queue_families = std::make_unique<VkQueueFamilyProperties[]>(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, queue_families.get());

    for (std::uint32_t index{ 0 }; index < queue_family_count; ++index) {
        if (queue_families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            _graphics_family = index;
        }

        VkBool32 present_support{ VK_FALSE };
        vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, index, _surface, &present_support);

        if (present_support == VK_TRUE) {
            _present_family = index;
        }

        if (_graphics_family < UINT32_MAX && _present_family < UINT32_MAX) {
            // TODO: Should break?
            // break;
        }
    }

    // Fill queue priorities array.
    float queue_prorities[] = { 1.0f };

    // Fill graphics device queue create informations.
    VkDeviceQueueCreateInfo device_graphics_queue_info;
    device_graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_graphics_queue_info.pNext = nullptr;
    device_graphics_queue_info.flags = 0;
    device_graphics_queue_info.queueFamilyIndex = _graphics_family;
    device_graphics_queue_info.queueCount = 1;
    device_graphics_queue_info.pQueuePriorities = queue_prorities;

    // Fill present queue create informations.
    VkDeviceQueueCreateInfo device_present_queue_info;
    device_present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_present_queue_info.pNext = nullptr;
    device_present_queue_info.flags = 0;
    device_present_queue_info.queueFamilyIndex = _present_family;
    device_present_queue_info.queueCount = 1;
    device_present_queue_info.pQueuePriorities = queue_prorities;

    VkDeviceQueueCreateInfo device_queue_infos[] = {
        device_graphics_queue_info,
        device_present_queue_info
    };

    // Fill logical device extensions array.
    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME // Need swapchain to present render result onto a screen.
    };

    // Fill device create informations.
    VkDeviceCreateInfo device_info;
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = nullptr;
    device_info.flags = 0;
    device_info.enabledLayerCount = sizeof(validation_layers) / sizeof(*validation_layers);
    device_info.ppEnabledLayerNames = validation_layers;
    device_info.enabledExtensionCount = sizeof(device_extensions) / sizeof(*device_extensions);
    device_info.ppEnabledExtensionNames = device_extensions;
    device_info.pEnabledFeatures = nullptr;
    device_info.queueCreateInfoCount = sizeof(device_queue_infos) / sizeof(*device_queue_infos);
    device_info.pQueueCreateInfos = device_queue_infos;

    // Create new Vulkan logical device using physical one.
    RB_MAYBE_UNUSED const auto result = vkCreateDevice(_physical_device, &device_info, nullptr, &_device);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan logical device");

    // Gets logical device queues.
    vkGetDeviceQueue(_device, _graphics_family, 0, &_graphics_queue);
    vkGetDeviceQueue(_device, _present_family, 0, &_present_queue);
}

void graphics_device_vulkan::_create_allocator(const graphics_device_desc& desc) {
    // Create Vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.instance = _instance;
    allocator_info.physicalDevice = _physical_device;
    allocator_info.device = _device;

    RB_MAYBE_UNUSED const auto result = vmaCreateAllocator(&allocator_info, &_allocator);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan memory allocator");
}

void graphics_device_vulkan::_query_surface(const graphics_device_desc& desc) {
    // Query surface format count of picked physical device.
    std::uint32_t surface_format_count{ 0 };
    auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, _surface, &surface_format_count, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to query surface format count");

    // Enumarate all surface formats.
    auto surface_formats = std::make_unique<VkSurfaceFormatKHR[]>(surface_format_count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, _surface, &surface_format_count, surface_formats.get());
    RB_ASSERT(result == VK_SUCCESS, "Failed to enumerate surface formats");

    // Choose surface color format.
    if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        _surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        _surface_format.format = surface_formats[0].format;
    }

    _surface_format.colorSpace = surface_formats[0].colorSpace;

    // Query surface capabilities.
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _surface, &surface_capabilities);
    RB_ASSERT(result == VK_SUCCESS, "Failed to retrieve physical device surface capabilities");

    // Store swapchain extent
    _swapchain_extent = surface_capabilities.currentExtent;
}
