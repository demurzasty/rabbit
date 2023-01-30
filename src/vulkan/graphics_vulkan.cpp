#include <rabbit/graphics.hpp>
#include <rabbit/format.hpp>

#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <vector>

using namespace rb;

#ifdef _DEBUG
#	define vk(expr) do { \
			[[maybe_unused]] const VkResult result = (expr); \
			assert(result == VK_SUCCESS); \
		} while (0)
#else
#	define vk(expr) (expr)
#endif

#if _DEBUG
static const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};
#endif

VKAPI_ATTR
static VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    println(pCallbackData->pMessage);
    return VK_FALSE;
}

struct graphics::impl {
    // Core

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    uint32_t graphics_family;
    uint32_t present_family;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VmaAllocator allocator;
    
    // Screen

    VkSurfaceFormatKHR _surface_format;
    VkExtent2D _swapchain_extent;
    VkSwapchainKHR _swapchain;
    VkPresentModeKHR _present_mode;

    std::vector<VkImage> _screen_images;
    std::vector<VkImageView> _screen_image_views;

    VkRenderPass _screen_render_pass;
    std::vector<VkFramebuffer> _screen_framebuffers;

    // Render Loop

    VkCommandPool _command_pool;
    std::vector<VkCommandBuffer> _command_buffers;
    std::vector<VkFence> _fences;

    VkSemaphore _render_semaphore;
    VkSemaphore _present_semaphore;

    std::uint32_t _image_index = 0;
};

graphics::graphics(const window& p_window)
    : m_impl(std::make_shared<impl>()) {
    volkInitialize();

    VkApplicationInfo app_info{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.pApplicationName = "RabBit";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "RabBit";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    const char* enabled_extensions[] = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
    };

#ifdef _DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debug_info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = &debug_callback;
    debug_info.pUserData = nullptr;
#endif

    VkInstanceCreateInfo instance_info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
#ifdef _DEBUG
    instance_info.pNext = &debug_info;
#else
    instance_info.pNext = nullptr;
#endif
    instance_info.pApplicationInfo = &app_info;
#ifdef _DEBUG
    instance_info.enabledLayerCount = sizeof(validation_layers) / sizeof(*validation_layers);
    instance_info.ppEnabledLayerNames = validation_layers;
#else
    instance_info.enabledLayerCount = 0;
    instance_info.ppEnabledLayerNames = nullptr;
#endif
    instance_info.enabledExtensionCount = sizeof(enabled_extensions) / sizeof(*enabled_extensions);
    instance_info.ppEnabledExtensionNames = enabled_extensions;
    vk(vkCreateInstance(&instance_info, nullptr, &m_impl->instance));

    volkLoadInstance(m_impl->instance);
    
#if _WIN32
    VkWin32SurfaceCreateInfoKHR surface_info{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surface_info.hinstance = GetModuleHandle(NULL);
    surface_info.hwnd = (HWND)p_window.handle();
    vk(vkCreateWin32SurfaceKHR(m_impl->instance, &surface_info, nullptr, &m_impl->surface));
#else
    assert(0);
#endif

    std::uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(m_impl->instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(m_impl->instance, &physical_device_count, physical_devices.data());

    m_impl->physical_device = physical_devices[0];

    vkGetPhysicalDeviceProperties(m_impl->physical_device, &m_impl->physical_device_properties);


    std::uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physical_device, &queue_family_count, queue_family_properties.data());

    for (std::uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_impl->graphics_family = i;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_impl->physical_device, i, m_impl->surface, &present_support);

        if (present_support == VK_TRUE) {
            m_impl->present_family = i;
        }
    }


    // Fill queue priorities array.
    float queuePriorities[] = { 1.0f };

    // Fill graphics device queue create informations.
    VkDeviceQueueCreateInfo device_graphics_queue_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    device_graphics_queue_info.queueFamilyIndex = m_impl->graphics_family;
    device_graphics_queue_info.queueCount = 1;
    device_graphics_queue_info.pQueuePriorities = queuePriorities;

    // Fill present queue create informations.
    VkDeviceQueueCreateInfo device_present_queue_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    device_present_queue_info.queueFamilyIndex = m_impl->present_family;
    device_present_queue_info.queueCount = 1;
    device_present_queue_info.pQueuePriorities = queuePriorities;

    VkDeviceQueueCreateInfo device_queue_infos[] = {
        device_graphics_queue_info,
        device_present_queue_info
    };

    // Fill logical device extensions array.
    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Need swapchain to present render result onto a screen.
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, // For resolve multisampled depth
    };

    VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
    VkPhysicalDeviceFeatures2 advance_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexing_features };
    vkGetPhysicalDeviceFeatures2(m_impl->physical_device, &advance_features);

    // Fill device create informations.
    VkDeviceCreateInfo device_info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    device_info.pNext = &advance_features;
#ifdef _DEBUG
    device_info.enabledLayerCount = sizeof(validation_layers) / sizeof(*validation_layers);
    device_info.ppEnabledLayerNames = validation_layers;
#else
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = nullptr;
#endif
    device_info.enabledExtensionCount = sizeof(device_extensions) / sizeof(*device_extensions);
    device_info.ppEnabledExtensionNames = device_extensions;
    // device_info.pEnabledFeatures = &supported_features;
    device_info.pEnabledFeatures = nullptr;
    device_info.queueCreateInfoCount = sizeof(device_queue_infos) / sizeof(*device_queue_infos);
    device_info.pQueueCreateInfos = device_queue_infos;

    // Create new Vulkan logical device using physical one.
    vk(vkCreateDevice(m_impl->physical_device, &device_info, nullptr, &m_impl->device));

    // Gets logical device queues.
    vkGetDeviceQueue(m_impl->device, m_impl->graphics_family, 0, &m_impl->graphics_queue);
    vkGetDeviceQueue(m_impl->device, m_impl->present_family, 0, &m_impl->present_queue);
}

graphics::~graphics() {
    vkDestroyDevice(m_impl->device, nullptr);
    vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
    vkDestroyInstance(m_impl->instance, nullptr);
}

void graphics::present() {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}
