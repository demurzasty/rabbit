#include "graphics_device_vulkan.hpp"
#include "render_pass_vulkan.hpp"
#include "command_buffer_vulkan.hpp"
#include "buffer_vulkan.hpp"
#include "texture_vulkan.hpp"
#include "shader_vulkan.hpp"
#include "resource_heap_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/engine/settings.hpp>

#if RB_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif

using namespace rb;

// TODO: Move surface creation into separate classes for every platform.

namespace {
#if _DEBUG
    const char* validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
#endif

    VKAPI_ATTR
    VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::fprintf(stderr, "%s\n", pCallbackData->pMessage);
        return VK_FALSE;
    }
}

graphics_device_vulkan::graphics_device_vulkan(settings& settings, window& window)
    : graphics_device(settings, window) {
    _initialize_volk(settings);
    _create_instance(settings);
    _choose_physical_device(settings);
    _create_surface(settings);
    _create_device(settings);
    _create_allocator(settings);
    _query_surface(settings);
    _create_swapchain(settings);
    _create_command_pool(settings);
    _create_synchronization_objects(settings);
}

graphics_device_vulkan::~graphics_device_vulkan() {
    vkQueueWaitIdle(_graphics_queue);
    vkQueueWaitIdle(_present_queue);
    vkDeviceWaitIdle(_device);

    vkDestroySemaphore(_device, _present_semaphore, nullptr);
    vkDestroySemaphore(_device, _render_semaphore, nullptr);
    vkDestroyCommandPool(_device, _command_pool, nullptr);
    vkDestroyRenderPass(_device, _render_pass, nullptr);

    for (auto framebuffer : _framebuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }

    for (auto image_view : _image_views) {
        vkDestroyImageView(_device, image_view, nullptr);
    }

    vkDestroyImageView(_device, _depth_image_view, nullptr);
    vmaDestroyImage(_allocator, _depth_image, _depth_image_allocation);

    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    vmaDestroyAllocator(_allocator);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
}

std::shared_ptr<render_pass> graphics_device_vulkan::create_render_pass(const render_pass_desc& desc) {
    return std::make_shared<render_pass_vulkan>(_device, desc);
}

std::shared_ptr<command_buffer> graphics_device_vulkan::create_command_buffer() {
    return std::make_shared<command_buffer_vulkan>(_device, _command_pool);
}

std::shared_ptr<buffer> graphics_device_vulkan::create_buffer(const buffer_desc& desc) {
    return std::make_shared<buffer_vulkan>(_device, _allocator, desc);
}

std::shared_ptr<texture> graphics_device_vulkan::create_texture(const texture_desc& desc) {
    return std::make_shared<texture_vulkan>(_device, _graphics_queue, _command_pool, _allocator, desc);
}

std::shared_ptr<shader> graphics_device_vulkan::create_shader(const shader_desc& desc) {
    const auto native_render_pass = std::static_pointer_cast<render_pass_vulkan>(desc.render_pass);
    const auto render_pass = native_render_pass ? native_render_pass->render_pass() : _render_pass;
    return std::make_shared<shader_vulkan>(_device, render_pass, _swapchain_extent, desc);
}

std::shared_ptr<resource_heap> graphics_device_vulkan::create_resource_heap(const resource_heap_desc& desc) {
    return std::make_shared<resource_heap_vulkan>(_device, desc);
}

void graphics_device_vulkan::submit(const std::shared_ptr<command_buffer>& command_buffer) {
    auto native_command_buffer = std::static_pointer_cast<command_buffer_vulkan>(command_buffer);

    auto vulkan_command_buffer = native_command_buffer->command_buffer();
    auto vulkan_fence = native_command_buffer->fence();

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = nullptr;
	submit_info.pWaitDstStageMask = nullptr;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vulkan_command_buffer;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = nullptr;

    RB_MAYBE_UNUSED const auto result = vkQueueSubmit(_graphics_queue, 1, &submit_info, vulkan_fence);
    RB_ASSERT(result == VK_SUCCESS, "Failed to queue submit");
}

void graphics_device_vulkan::present() {
    RB_MAYBE_UNUSED VkResult result;

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &_present_semaphore;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.commandBufferCount = 0;
	submit_info.pCommandBuffers = nullptr;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &_render_semaphore;

    result = vkQueueSubmit(_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    RB_ASSERT(result == VK_SUCCESS, "Failed to queue submit");

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;

    // We should wait for rendering execution.
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_semaphore;

    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_swapchain;

    present_info.pImageIndices = &_image_index;

    present_info.pResults = nullptr;

    result = vkQueuePresentKHR(_graphics_queue, &present_info);
    RB_ASSERT(result == VK_SUCCESS, "Failed to queue present");

    result = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _present_semaphore, nullptr, &_image_index);
    RB_ASSERT(result == VK_SUCCESS, "Failed to reset acquire next swapchain image");
}

VkRenderPass graphics_device_vulkan::render_pass() const {
    return _render_pass;
}

VkExtent2D graphics_device_vulkan::swapchain_extent() const {
    return _swapchain_extent;
}

VkFramebuffer graphics_device_vulkan::framebuffer() const {
    return _framebuffers[_image_index];
}

void graphics_device_vulkan::_initialize_volk(const settings& settings) {
    RB_MAYBE_UNUSED const auto result = volkInitialize();
    RB_ASSERT(result == VK_SUCCESS, "Cannot initialize volk library");
}

void graphics_device_vulkan::_create_instance(const settings& settings) {
    const auto [major, minor, patch] = settings.application_version;

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = settings.application_name.c_str();
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

#ifdef _DEBUG
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
#endif

    VkInstanceCreateInfo instance_info;
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef _DEBUG
    instance_info.pNext = &debug_info;
#else
    instance_info.pNext = nullptr;
#endif
    instance_info.flags = 0;
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

    RB_MAYBE_UNUSED const auto result = vkCreateInstance(&instance_info, nullptr, &_instance);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan instance");

    volkLoadInstance(_instance);
}

void graphics_device_vulkan::_choose_physical_device(const settings& desc) {
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

void graphics_device_vulkan::_create_surface(const settings& desc) {
#if RB_WINDOWS
    // Fill Win32 surface create informations.
    VkWin32SurfaceCreateInfoKHR surface_info;
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = nullptr;
    surface_info.flags = 0;
    surface_info.hinstance = GetModuleHandle(nullptr);
    surface_info.hwnd = associated_window().native_handle();

    // Create new Vulkan surface.
    RB_MAYBE_UNUSED const auto result = vkCreateWin32SurfaceKHR(_instance, &surface_info, nullptr, &_surface);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan surface");
#else
    RB_ASSERT(false, "Graphics for that platform is not implemented");
#endif
}

void graphics_device_vulkan::_create_device(const settings& desc) {
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
#ifdef _DEBUG
    device_info.enabledLayerCount = sizeof(validation_layers) / sizeof(*validation_layers);
    device_info.ppEnabledLayerNames = validation_layers;
#else
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = nullptr;
#endif
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

void graphics_device_vulkan::_create_allocator(const settings& desc) {
    // Create Vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.instance = _instance;
    allocator_info.physicalDevice = _physical_device;
    allocator_info.device = _device;

    RB_MAYBE_UNUSED const auto result = vmaCreateAllocator(&allocator_info, &_allocator);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan memory allocator");
}

void graphics_device_vulkan::_query_surface(const settings& desc) {
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

void graphics_device_vulkan::_create_swapchain(const settings& settings) {
    RB_MAYBE_UNUSED VkResult result;

    // Get window size.
    const auto window_size = associated_window().size();

    // Query surface capabilities.
    VkSurfaceCapabilitiesKHR surface_capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _surface, &surface_capabilities);
    RB_ASSERT(result == VK_SUCCESS, "Failed to retrieve physical device surface capabilities");

    std::uint32_t present_mode_count{ 0 };
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device, _surface, &present_mode_count, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to query present mode count");

    auto present_modes = std::make_unique<VkPresentModeKHR[]>(present_mode_count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device, _surface, &present_mode_count, present_modes.get());
    RB_ASSERT(result == VK_SUCCESS, "Failed to enumerate present mode count");

    _present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

    if (!settings.vsync) {
        _present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Search for better solution.
        for (std::uint32_t index{ 0 }; index < present_mode_count; ++index) {
            if (present_modes[index] == VK_PRESENT_MODE_MAILBOX_KHR) {
                _present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
    }

    auto image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }

    std::uint32_t queue_indices[] = { _graphics_family, _present_family };

    VkSwapchainCreateInfoKHR swapchain_info;
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.pNext = nullptr;
    swapchain_info.flags = 0;
    swapchain_info.surface = _surface;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = _surface_format.format;
    swapchain_info.imageColorSpace = _surface_format.colorSpace;
    swapchain_info.imageExtent = { window_size.x, window_size.y };
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (_graphics_family != _present_family) {
        swapchain_info.queueFamilyIndexCount = sizeof(queue_indices) / sizeof(*queue_indices);
        swapchain_info.pQueueFamilyIndices = queue_indices;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    } else {
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = nullptr;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchain_info.presentMode = _present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.preTransform = surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.oldSwapchain = nullptr;

    // Create Vulkan swapchain.
    result = vkCreateSwapchainKHR(_device, &swapchain_info, nullptr, &_swapchain);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create swapchain");

    // Query swapchain image count.
    result = vkGetSwapchainImagesKHR(_device, _swapchain, &image_count, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to query swapchain image count");

    // Get swapchain images list.
    _images.resize(image_count);
    result = vkGetSwapchainImagesKHR(_device, _swapchain, &image_count, _images.data());
    RB_ASSERT(result == VK_SUCCESS, "Failed to enumerate swapchain images");

    _image_views.resize(_images.size());
    for (std::size_t index{ 0 }; index < _images.size(); ++index) {
        VkImageViewCreateInfo image_view_info;
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.pNext = nullptr;
        image_view_info.flags = 0;
        image_view_info.image = _images[index];
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = _surface_format.format;
        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;

        result = vkCreateImageView(_device, &image_view_info, nullptr, &_image_views[index]);
        RB_ASSERT(result == VK_SUCCESS, "Failed to create image view");
    }

    const VkFormat depth_formats[] = {
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    VkImageCreateInfo depth_image_info{};
    depth_image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_image_info.imageType = VK_IMAGE_TYPE_2D;
    depth_image_info.extent.width = window_size.x;
    depth_image_info.extent.height = window_size.y;
    depth_image_info.extent.depth = 1;
    depth_image_info.mipLevels = 1;
    depth_image_info.arrayLayers = 1;
    depth_image_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo depth_allocation_info = {};
    depth_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    result = vmaCreateImage(_allocator, &depth_image_info, &depth_allocation_info, &_depth_image, &_depth_image_allocation, nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan depth image");

    VkImageViewCreateInfo depth_image_view_info{};
    depth_image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_image_view_info.image = _depth_image;
    depth_image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depth_image_view_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_image_view_info.subresourceRange.baseMipLevel = 0;
    depth_image_view_info.subresourceRange.levelCount = 1;
    depth_image_view_info.subresourceRange.baseArrayLayer = 0;
    depth_image_view_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(_device, &depth_image_view_info, nullptr, &_depth_image_view);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan depth image view");

    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = _surface_format.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference{};
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = { color_attachment, depth_attachment };

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = sizeof(attachments) / sizeof(*attachments);
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &subpass_dependency;

    result = vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create render pass");

    _framebuffers.resize(_image_views.size());
    for (std::size_t index{ 0 }; index < _images.size(); ++index) {
        VkImageView framebuffer_attachments[] = { _image_views[index], _depth_image_view };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = _render_pass;
        framebuffer_info.attachmentCount = sizeof(framebuffer_attachments) / sizeof(*framebuffer_attachments);
        framebuffer_info.pAttachments = framebuffer_attachments;
        framebuffer_info.width = window_size.x;
        framebuffer_info.height = window_size.y;
        framebuffer_info.layers = 1;

        result = vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_framebuffers[index]);
        RB_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer");
    }
}

void graphics_device_vulkan::_create_command_pool(const settings& desc) {
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = _graphics_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    RB_MAYBE_UNUSED const auto result = vkCreateCommandPool(_device, &pool_info, nullptr, &_command_pool);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create command pool");
}

void graphics_device_vulkan::_create_synchronization_objects(const settings& desc) {
    VkSemaphoreCreateInfo semaphore_info;
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;
	semaphore_info.flags = 0;

	auto result = vkCreateSemaphore(_device, &semaphore_info, nullptr, &_render_semaphore);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create render semaphore");

	result = vkCreateSemaphore(_device, &semaphore_info, nullptr, &_present_semaphore);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create present semaphore");

    result = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _present_semaphore, nullptr, &_image_index);
    RB_ASSERT(result == VK_SUCCESS, "Failed to reset acquire next swapchain image");
}
