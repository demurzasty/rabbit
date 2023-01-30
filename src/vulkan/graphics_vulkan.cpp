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

    VkSurfaceFormatKHR surface_format;
    VkExtent2D swapchain_extent;
    VkSwapchainKHR swapchain;
    VkPresentModeKHR present_mode;

    std::vector<VkImage> screen_images;
    std::vector<VkImageView> screen_image_views;

    VkRenderPass screen_render_pass;
    std::vector<VkFramebuffer> screen_framebuffers;

    // Render Loop

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;
    std::vector<VkFence> fences;

    VkSemaphore render_semaphore;
    VkSemaphore present_semaphore;

    std::uint32_t image_index = 0;
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

    // Create Vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.instance = m_impl->instance;
    allocator_info.physicalDevice = m_impl->physical_device;
    allocator_info.device = m_impl->device;
    vk(vmaCreateAllocator(&allocator_info, &m_impl->allocator));

    // Query surface format count of picked physical device.
    std::uint32_t surface_format_count = 0;
    vk(vkGetPhysicalDeviceSurfaceFormatsKHR(m_impl->physical_device, m_impl->surface, &surface_format_count, nullptr));

    // Enumarate all surface formats.
    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vk(vkGetPhysicalDeviceSurfaceFormatsKHR(m_impl->physical_device, m_impl->surface, &surface_format_count, surface_formats.data()));

    // Choose surface color format.
    if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        m_impl->surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        m_impl->surface_format.format = surface_formats[0].format;
    }

    m_impl->surface_format.colorSpace = surface_formats[0].colorSpace;

    // Query surface capabilities.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_impl->physical_device, m_impl->surface, &surfaceCapabilities));

    // Store swapchain extent
    m_impl->swapchain_extent = surfaceCapabilities.currentExtent;

    // Get window size.
    const auto windowWidth = std::uint32_t(p_window.width());
    const auto windowHeight = std::uint32_t(p_window.height());

    std::uint32_t present_mode_count = 0;
    vk(vkGetPhysicalDeviceSurfacePresentModesKHR(m_impl->physical_device, m_impl->surface, &present_mode_count, nullptr));

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vk(vkGetPhysicalDeviceSurfacePresentModesKHR(m_impl->physical_device, m_impl->surface, &present_mode_count, present_modes.data()));

    m_impl->present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // TODO: Select present mode if vertical synchronization is enabled.

    std::uint32_t min_image_count = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && min_image_count > surfaceCapabilities.maxImageCount) {
        min_image_count = surfaceCapabilities.maxImageCount;
    }

    std::uint32_t queue_indices[] = { m_impl->graphics_family, m_impl->present_family };

    VkSwapchainCreateInfoKHR swapchain_info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchain_info.surface = m_impl->surface;
    swapchain_info.minImageCount = min_image_count;
    swapchain_info.imageFormat = m_impl->surface_format.format;
    swapchain_info.imageColorSpace = m_impl->surface_format.colorSpace;
    swapchain_info.imageExtent = { windowWidth, windowHeight };
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (m_impl->graphics_family != m_impl->present_family) {
        swapchain_info.queueFamilyIndexCount = sizeof(queue_indices) / sizeof(*queue_indices);
        swapchain_info.pQueueFamilyIndices = queue_indices;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    } else {
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = nullptr;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchain_info.presentMode = m_impl->present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.preTransform = surfaceCapabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    // Create Vulkan swapchain.
    vk(vkCreateSwapchainKHR(m_impl->device, &swapchain_info, nullptr, &m_impl->swapchain));

    std::uint32_t image_count;
    vk(vkGetSwapchainImagesKHR(m_impl->device, m_impl->swapchain, &image_count, nullptr));

    // Get swapchain images list.
    m_impl->screen_images.resize(image_count);
    vk(vkGetSwapchainImagesKHR(m_impl->device, m_impl->swapchain, &image_count, m_impl->screen_images.data()));

    m_impl->screen_image_views.resize(m_impl->screen_images.size());
    for (std::size_t i = 0; i < m_impl->screen_images.size(); ++i) {
        VkImageViewCreateInfo image_view_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        image_view_info.image = m_impl->screen_images[i];
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = m_impl->surface_format.format;
        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;
        vk(vkCreateImageView(m_impl->device, &image_view_info, nullptr, &m_impl->screen_image_views[i]));
    }

    VkAttachmentDescription color_attachments;
    color_attachments.flags = 0;
    color_attachments.format = m_impl->surface_format.format;
    color_attachments.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachments.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachments.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription attachments[] = {
        color_attachments,
    };

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_references[] = {
        color_attachment_reference
    };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = color_attachment_references;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pResolveAttachments = nullptr;

    VkSubpassDependency subpass_deps[2]{};
    subpass_deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_deps[0].dstSubpass = 0;
    subpass_deps[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_deps[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpass_deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_deps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpass_deps[1].srcSubpass = 0;
    subpass_deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_deps[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_deps[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpass_deps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpass_deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_deps[0].dstSubpass = 0;
    subpass_deps[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // Both stages might have access the depth-buffer, so need both in src/dstStageMask;;
    subpass_deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_deps[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpass_deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    subpass_deps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpass_deps[1].srcSubpass = 0;
    subpass_deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_deps[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_deps[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpass_deps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo render_pass_info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    render_pass_info.attachmentCount = sizeof(attachments) / sizeof(*attachments);
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = subpass_deps;
    vk(vkCreateRenderPass(m_impl->device, &render_pass_info, nullptr, &m_impl->screen_render_pass));
}

graphics::~graphics() {
    vkDestroyRenderPass(m_impl->device, m_impl->screen_render_pass, nullptr);

    for (VkImageView image_view : m_impl->screen_image_views) {
        vkDestroyImageView(m_impl->device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(m_impl->device, m_impl->swapchain, nullptr);

    vmaDestroyAllocator(m_impl->allocator);
    vkDestroyDevice(m_impl->device, nullptr);
    vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
    vkDestroyInstance(m_impl->instance, nullptr);
}

void graphics::present() {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}
