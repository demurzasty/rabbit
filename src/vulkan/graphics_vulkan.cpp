#include <rabbit/graphics.hpp>
#include <rabbit/format.hpp>
#include <rabbit/arena.hpp>
#include <rabbit/zone.hpp>

#include "shaders/gen/canvas.vert.spv.h"
#include "shaders/gen/canvas.frag.spv.h"

#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <mutex>
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

struct texture {
    VkImage image{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VkImageView image_view{ VK_NULL_HANDLE };
    VkSampler sampler{ VK_NULL_HANDLE };
};

enum class canvas_draw_command_type {
    clip,
    primitives
};

struct canvas_draw_command_clip {
    float left;
    float top;
    float width;
    float height;
};

struct canvas_draw_command_primitives {
    std::int32_t texture_index;
    std::uint32_t index_offset;
    std::uint32_t index_count;
    std::uint32_t vertex_offset;
};

struct canvas_draw_command {
    canvas_draw_command_type type;
    union {
        canvas_draw_command_clip clip;
        canvas_draw_command_primitives primitives;
    };
};

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

    std::mutex mutex;

    // GPU buffers

    VkBuffer canvas_vertex_buffer;
    VmaAllocation canvas_vertex_buffer_allocation;
    std::size_t canvas_vertex_buffer_offset = 0;

    VkBuffer canvas_index_buffer;
    VmaAllocation canvas_index_buffer_allocation;
    std::size_t canvas_index_buffer_offset = 0;

    // Drawing

    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;

    arena<texture> textures;

    std::vector<canvas_draw_command> canvas_draw_commands;
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
    color_attachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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

    m_impl->screen_framebuffers.resize(m_impl->screen_image_views.size());
    for (std::size_t i = 0; i < m_impl->screen_framebuffers.size(); ++i) {
        VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebufferInfo.renderPass = m_impl->screen_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_impl->screen_image_views[i];
        framebufferInfo.width = m_impl->swapchain_extent.width;
        framebufferInfo.height = m_impl->swapchain_extent.height;
        framebufferInfo.layers = 1;
        vk(vkCreateFramebuffer(m_impl->device, &framebufferInfo, nullptr, &m_impl->screen_framebuffers[i]));
    }

    VkCommandPoolCreateInfo command_pool_info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    command_pool_info.queueFamilyIndex = m_impl->graphics_family;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ?
    vk(vkCreateCommandPool(m_impl->device, &command_pool_info, nullptr, &m_impl->command_pool));

    m_impl->command_buffers.resize(m_impl->screen_image_views.size());

    VkCommandBufferAllocateInfo command_buffer_alloc_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    command_buffer_alloc_info.commandPool = m_impl->command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_alloc_info.commandBufferCount = std::uint32_t(m_impl->command_buffers.size());
    vk(vkAllocateCommandBuffers(m_impl->device, &command_buffer_alloc_info, m_impl->command_buffers.data()));

    VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vk(vkCreateSemaphore(m_impl->device, &semaphore_info, VK_NULL_HANDLE, &m_impl->render_semaphore));
    vk(vkCreateSemaphore(m_impl->device, &semaphore_info, VK_NULL_HANDLE, &m_impl->present_semaphore));

    m_impl->fences.resize(m_impl->screen_image_views.size());

    VkFenceCreateInfo fence_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (auto& fence : m_impl->fences) {
        vk(vkCreateFence(m_impl->device, &fence_info, nullptr, &fence));
    }

    VkBufferCreateInfo canvas_vertex_buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    canvas_vertex_buffer_info.size = sizeof(vertex2d) * 0x10000;
    canvas_vertex_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    canvas_vertex_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    canvas_vertex_buffer_info.queueFamilyIndexCount = 0;
    canvas_vertex_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo canvas_vertex_buffer_allocation_info{};
    canvas_vertex_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    canvas_vertex_buffer_allocation_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vk(vmaCreateBuffer(m_impl->allocator, &canvas_vertex_buffer_info, &canvas_vertex_buffer_allocation_info, &m_impl->canvas_vertex_buffer, &m_impl->canvas_vertex_buffer_allocation, nullptr));

    VkBufferCreateInfo canvas_index_buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    canvas_index_buffer_info.size = sizeof(std::uint32_t) * 0x10000;
    canvas_index_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    canvas_index_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    canvas_index_buffer_info.queueFamilyIndexCount = 0;
    canvas_index_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo canvas_index_buffer_allocation_info{};
    canvas_index_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    canvas_index_buffer_allocation_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vk(vmaCreateBuffer(m_impl->allocator, &canvas_index_buffer_info, &canvas_index_buffer_allocation_info, &m_impl->canvas_index_buffer, &m_impl->canvas_index_buffer_allocation, nullptr));

    VkPipelineLayoutCreateInfo pipeline_layout_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;
    vk(vkCreatePipelineLayout(m_impl->device, &pipeline_layout_info, nullptr, &m_impl->pipeline_layout));

    VkShaderModule shader_modules[2];

    VkShaderModuleCreateInfo vertex_shader_module_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    vertex_shader_module_info.codeSize = sizeof(canvas_vert_spv);
    vertex_shader_module_info.pCode = (const std::uint32_t*)canvas_vert_spv;
    vk(vkCreateShaderModule(m_impl->device, &vertex_shader_module_info, nullptr, &shader_modules[0]));

    VkShaderModuleCreateInfo fragment_shader_module_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    fragment_shader_module_info.codeSize = sizeof(canvas_frag_spv);
    fragment_shader_module_info.pCode = (const std::uint32_t*)canvas_frag_spv;
    vk(vkCreateShaderModule(m_impl->device, &fragment_shader_module_info, nullptr, &shader_modules[1]));

    VkVertexInputBindingDescription vertex_input_binding_desc;
    vertex_input_binding_desc.binding = 0;
    vertex_input_binding_desc.stride = sizeof(vertex2d);
    vertex_input_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_attributes[3]{
        { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex2d, position) },
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex2d, texcoord) },
        { 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(vertex2d, color) },
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &vertex_input_binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = 3;
    vertex_input_info.pVertexAttributeDescriptions = vertex_attributes;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = float(m_impl->swapchain_extent.width);
    viewport.height = float(m_impl->swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = m_impl->swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state_info{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer_state_info{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer_state_info.depthClampEnable = VK_FALSE;
    rasterizer_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_state_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_state_info.cullMode = VK_CULL_MODE_NONE;
    rasterizer_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_state_info.depthBiasEnable = VK_FALSE;
    rasterizer_state_info.depthBiasConstantFactor = 0.0f;
    rasterizer_state_info.depthBiasClamp = 0.0f;
    rasterizer_state_info.depthBiasSlopeFactor = 0.0f;
    rasterizer_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state_info{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // VK_SAMPLE_COUNT_1_BIT;
    multisample_state_info.sampleShadingEnable = VK_FALSE;
    multisample_state_info.minSampleShading = 0.0f;
    multisample_state_info.pSampleMask = nullptr;
    multisample_state_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_info.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depth_stencil_info.depthTestEnable = VK_FALSE;
    depth_stencil_info.depthWriteEnable = VK_FALSE;
    depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_info.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_info{};
    color_blend_attachment_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_info.blendEnable = VK_TRUE;
    color_blend_attachment_info.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_info.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_info.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state_info{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    color_blend_state_info.logicOpEnable = VK_FALSE;
    color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_info.attachmentCount = 1;
    color_blend_state_info.pAttachments = &color_blend_attachment_info;
    color_blend_state_info.blendConstants[0] = 0.0f;
    color_blend_state_info.blendConstants[1] = 0.0f;
    color_blend_state_info.blendConstants[2] = 0.0f;
    color_blend_state_info.blendConstants[3] = 0.0f;

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = shader_modules[0];
    vertex_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = shader_modules[1];
    fragment_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_shader_stage_info,
        fragment_shader_stage_info
    };

    VkDynamicState dynamic_states[]{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic_state_info.dynamicStateCount = 2;
    dynamic_state_info.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo pipeline_info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterizer_state_info;
    pipeline_info.pMultisampleState = &multisample_state_info;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_info;
    pipeline_info.layout = m_impl->pipeline_layout;
    pipeline_info.renderPass = m_impl->screen_render_pass;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pDynamicState = &dynamic_state_info;
    vk(vkCreateGraphicsPipelines(m_impl->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_impl->pipeline));

    vkDestroyShaderModule(m_impl->device, shader_modules[1], nullptr);
    vkDestroyShaderModule(m_impl->device, shader_modules[0], nullptr);
}

graphics::~graphics() {
    for (auto& fence : m_impl->fences) {
        vkWaitForFences(m_impl->device, 1, &fence, VK_TRUE, 1000000000);
        vkDestroyFence(m_impl->device, fence, nullptr);
    }
    vkFreeCommandBuffers(m_impl->device, m_impl->command_pool, std::uint32_t(m_impl->command_buffers.size()), m_impl->command_buffers.data());

    vkQueueWaitIdle(m_impl->graphics_queue);
    vkQueueWaitIdle(m_impl->present_queue);
    vkDeviceWaitIdle(m_impl->device);

    vkDestroyPipeline(m_impl->device, m_impl->pipeline, nullptr);
    vkDestroyPipelineLayout(m_impl->device, m_impl->pipeline_layout, nullptr);

    vmaDestroyBuffer(m_impl->allocator, m_impl->canvas_index_buffer, m_impl->canvas_index_buffer_allocation);
    vmaDestroyBuffer(m_impl->allocator, m_impl->canvas_vertex_buffer, m_impl->canvas_vertex_buffer_allocation);

    vkDestroySemaphore(m_impl->device, m_impl->present_semaphore, nullptr);
    vkDestroySemaphore(m_impl->device, m_impl->render_semaphore, nullptr);

    vkDestroyCommandPool(m_impl->device, m_impl->command_pool, nullptr);

    for (VkFramebuffer framebuffer : m_impl->screen_framebuffers) {
        vkDestroyFramebuffer(m_impl->device, framebuffer, nullptr);
    }

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

id_type graphics::create_texture() {
    std::unique_lock lock{ m_impl->mutex };

    const auto id = m_impl->textures.create();

    return id;
}

void graphics::destroy_texture(id_type p_id) {
    std::unique_lock lock{ m_impl->mutex };

    assert(m_impl->textures.valid(p_id));

    auto& data = m_impl->textures[p_id];

    if (data.image) {
        vkDestroySampler(m_impl->device, data.sampler, nullptr);
        vkDestroyImageView(m_impl->device, data.image_view, nullptr);
        vmaDestroyImage(m_impl->allocator, data.image, data.allocation);
    }
}

void graphics::set_texture_data(id_type p_id, int p_width, int p_height, const void* p_pixels) {
    std::unique_lock lock{ m_impl->mutex };

    assert(m_impl->textures.valid(p_id));

    auto& data = m_impl->textures[p_id];

    VkImageCreateInfo image_info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = { std::uint32_t(p_width), std::uint32_t(p_height), 1 };
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = 0;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocation_info{};
    allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vk(vmaCreateImage(m_impl->allocator, &image_info, &allocation_info, &data.image, &data.allocation, nullptr));

    // Create staging buffer.
    VkBufferCreateInfo buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    buffer_info.size = p_width * p_height * 4;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = 0;
    buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo buffer_allocation_info{};
    buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_allocation;
    vk(vmaCreateBuffer(m_impl->allocator, &buffer_info, &buffer_allocation_info, &staging_buffer, &staging_buffer_allocation, nullptr));

    // Transfer pixels into buffer.
    void* ptr;
    vk(vmaMapMemory(m_impl->allocator, staging_buffer_allocation, &ptr));
    memcpy(ptr, p_pixels, buffer_info.size);
    vmaUnmapMemory(m_impl->allocator, staging_buffer_allocation);

    // Create temporary buffer
    VkCommandBufferAllocateInfo command_buffer_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_info.commandPool = m_impl->command_pool;
    command_buffer_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_impl->device, &command_buffer_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    // Begin registering commands
    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = data.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = image_info.extent;
    vkCmdCopyBufferToImage(command_buffer, staging_buffer, data.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = data.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // End registering commands
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(m_impl->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_impl->graphics_queue);

    vkFreeCommandBuffers(m_impl->device, m_impl->command_pool, 1, &command_buffer);

    vmaDestroyBuffer(m_impl->allocator, staging_buffer, staging_buffer_allocation);

    VkImageViewCreateInfo image_view_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    image_view_info.image = data.image;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;
    vk(vkCreateImageView(m_impl->device, &image_view_info, nullptr, &data.image_view));

    VkSamplerCreateInfo sampler_info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampler_info.magFilter = VK_FILTER_LINEAR; // TODO: Cubic filtering.
    sampler_info.minFilter = sampler_info.magFilter;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // TODO: Do mapping.
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = sampler_info.addressModeU;
    sampler_info.addressModeW = sampler_info.addressModeV;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 0.0f;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    vk(vkCreateSampler(m_impl->device, &sampler_info, nullptr, &data.sampler));
}

void graphics::push_canvas_clip(float p_left, float p_top, float p_width, float p_height) {
    std::unique_lock lock{ m_impl->mutex };

    canvas_draw_command command;
    command.type = canvas_draw_command_type::clip;
    command.clip.left = p_left;
    command.clip.top = p_top;
    command.clip.width = p_width;
    command.clip.height = p_height;
    m_impl->canvas_draw_commands.push_back(command);
}

void graphics::push_canvas_primitives(const span<const vertex2d>& p_vertices, const span<const std::uint32_t>& p_indices) {
    std::unique_lock lock{ m_impl->mutex };

    void* ptr;

    vk(vmaMapMemory(m_impl->allocator, m_impl->canvas_vertex_buffer_allocation, &ptr));
    memcpy(static_cast<vertex2d*>(ptr) + m_impl->canvas_vertex_buffer_offset, p_vertices.data(), p_vertices.size_bytes());
    vmaUnmapMemory(m_impl->allocator, m_impl->canvas_vertex_buffer_allocation);

    vk(vmaMapMemory(m_impl->allocator, m_impl->canvas_index_buffer_allocation, &ptr));
    memcpy(static_cast<std::uint32_t*>(ptr) + m_impl->canvas_index_buffer_offset, p_indices.data(), p_indices.size_bytes());
    vmaUnmapMemory(m_impl->allocator, m_impl->canvas_index_buffer_allocation);

    canvas_draw_command command;
    command.type = canvas_draw_command_type::primitives;
    command.primitives.texture_index = -1;
    command.primitives.index_offset = std::uint32_t(m_impl->canvas_index_buffer_offset);
    command.primitives.index_count = std::uint32_t(p_indices.size());
    command.primitives.vertex_offset = std::uint32_t(m_impl->canvas_vertex_buffer_offset);
    m_impl->canvas_draw_commands.push_back(command);

    m_impl->canvas_vertex_buffer_offset += p_vertices.size();
    m_impl->canvas_index_buffer_offset += p_indices.size();
}

void graphics::present() {
    std::unique_lock lock{ m_impl->mutex };

    vk(vkAcquireNextImageKHR(m_impl->device, m_impl->swapchain, UINT64_MAX, m_impl->present_semaphore, VK_NULL_HANDLE, &m_impl->image_index));

    vkWaitForFences(m_impl->device, 1, &m_impl->fences[m_impl->image_index], VK_FALSE, UINT64_MAX);
    vkResetFences(m_impl->device, 1, &m_impl->fences[m_impl->image_index]);

    vkResetCommandBuffer(m_impl->command_buffers[m_impl->image_index], 0);

    VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(m_impl->command_buffers[m_impl->image_index], &begin_info);

    VkClearValue clear_values[1];
    clear_values[0].color = { 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f };

    VkRenderPassBeginInfo render_pass_begin_info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    render_pass_begin_info.renderPass = m_impl->screen_render_pass;
    render_pass_begin_info.framebuffer = m_impl->screen_framebuffers[m_impl->image_index];
    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.renderArea.extent = m_impl->swapchain_extent;
    render_pass_begin_info.clearValueCount = sizeof(clear_values) / sizeof(*clear_values);
    render_pass_begin_info.pClearValues = clear_values;
    vkCmdBeginRenderPass(m_impl->command_buffers[m_impl->image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{ 0.0f, 0.0f, float(m_impl->swapchain_extent.width), float(m_impl->swapchain_extent.height), 0.0f, 1.0f };
    vkCmdSetViewport(m_impl->command_buffers[m_impl->image_index], 0, 1, &viewport);

    VkRect2D scissor{ { 0, 0 }, { m_impl->swapchain_extent.width, m_impl->swapchain_extent.height } };
    vkCmdSetScissor(m_impl->command_buffers[m_impl->image_index], 0, 1, &scissor);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_impl->command_buffers[m_impl->image_index], 0, 1, &m_impl->canvas_vertex_buffer, &offset);
    vkCmdBindIndexBuffer(m_impl->command_buffers[m_impl->image_index], m_impl->canvas_index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindPipeline(m_impl->command_buffers[m_impl->image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->pipeline);

    for (auto& command : m_impl->canvas_draw_commands) {
        switch (command.type) {
        case canvas_draw_command_type::clip: {
            VkRect2D rect;
            rect.offset.x = std::int32_t(command.clip.left);
            rect.offset.y = std::int32_t(command.clip.top);
            rect.extent.width = std::uint32_t(command.clip.width);
            rect.extent.height = std::uint32_t(command.clip.height);
            vkCmdSetScissor(m_impl->command_buffers[m_impl->image_index], 0, 1, &rect);
            break;
        }
        case canvas_draw_command_type::primitives:
            vkCmdDrawIndexed(m_impl->command_buffers[m_impl->image_index],
                command.primitives.index_count,
                1,
                command.primitives.index_offset,
                command.primitives.vertex_offset,
                0);
            break;
        }
    }

    vkCmdEndRenderPass(m_impl->command_buffers[m_impl->image_index]);

    vkEndCommandBuffer(m_impl->command_buffers[m_impl->image_index]);

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &m_impl->present_semaphore;
    submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_impl->command_buffers[m_impl->image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_impl->render_semaphore;
    vkQueueSubmit(m_impl->graphics_queue, 1, &submit_info, m_impl->fences[m_impl->image_index]);

    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_impl->render_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_impl->swapchain;
    present_info.pImageIndices = &m_impl->image_index;
    vkQueuePresentKHR(m_impl->present_queue, &present_info);
    vkQueueWaitIdle(m_impl->present_queue);

    m_impl->canvas_vertex_buffer_offset = 0;
    m_impl->canvas_index_buffer_offset = 0;
    m_impl->canvas_draw_commands.clear();
}
