#include "utils_vulkan.hpp"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <cstdio>

using namespace rb;

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
    std::printf("%s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

void vku::setup(std::unique_ptr<renderer::data>& data, window& window) {
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
    vk(vkCreateInstance(&instance_info, nullptr, &data->instance));

    volkLoadInstance(data->instance);
    
#if _WIN32
    VkWin32SurfaceCreateInfoKHR surface_info{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surface_info.hinstance = GetModuleHandle(NULL);
    surface_info.hwnd = (HWND)window.handle();
    vk(vkCreateWin32SurfaceKHR(data->instance, &surface_info, nullptr, &data->surface));
#else
    assert(0);
#endif

    std::uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(data->instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(data->instance, &physical_device_count, physical_devices.data());

    data->physical_device = physical_devices[0];

    vkGetPhysicalDeviceProperties(data->physical_device, &data->physical_device_properties);


    std::uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(data->physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(data->physical_device, &queue_family_count, queue_family_properties.data());

    for (std::uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            data->graphics_family = i;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(data->physical_device, i, data->surface, &present_support);

        if (present_support == VK_TRUE) {
            data->present_family = i;
        }
    }


    // Fill queue priorities array.
    float queuePriorities[] = { 1.0f };

    // Fill graphics device queue create informations.
    VkDeviceQueueCreateInfo device_graphics_queue_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    device_graphics_queue_info.queueFamilyIndex = data->graphics_family;
    device_graphics_queue_info.queueCount = 1;
    device_graphics_queue_info.pQueuePriorities = queuePriorities;

    // Fill present queue create informations.
    VkDeviceQueueCreateInfo device_present_queue_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    device_present_queue_info.queueFamilyIndex = data->present_family;
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
    vkGetPhysicalDeviceFeatures2(data->physical_device, &advance_features);

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
    vk(vkCreateDevice(data->physical_device, &device_info, nullptr, &data->device));

    // Gets logical device queues.
    vkGetDeviceQueue(data->device, data->graphics_family, 0, &data->graphics_queue);
    vkGetDeviceQueue(data->device, data->present_family, 0, &data->present_queue);

    // Create Vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.instance = data->instance;
    allocator_info.physicalDevice = data->physical_device;
    allocator_info.device = data->device;
    vk(vmaCreateAllocator(&allocator_info, &data->allocator));

    // Query surface format count of picked physical device.
    std::uint32_t surface_format_count = 0;
    vk(vkGetPhysicalDeviceSurfaceFormatsKHR(data->physical_device, data->surface, &surface_format_count, nullptr));

    // Enumarate all surface formats.
    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vk(vkGetPhysicalDeviceSurfaceFormatsKHR(data->physical_device, data->surface, &surface_format_count, surface_formats.data()));

    // Choose surface color format.
    if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        data->surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        data->surface_format.format = surface_formats[0].format;
    }

    data->surface_format.colorSpace = surface_formats[0].colorSpace;

    // Query surface capabilities.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(data->physical_device, data->surface, &surfaceCapabilities));

    // Store swapchain extent
    data->swapchain_extent = surfaceCapabilities.currentExtent;

    // Get window size.
    const auto window_size = window.size();

    std::uint32_t present_mode_count = 0;
    vk(vkGetPhysicalDeviceSurfacePresentModesKHR(data->physical_device, data->surface, &present_mode_count, nullptr));

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vk(vkGetPhysicalDeviceSurfacePresentModesKHR(data->physical_device, data->surface, &present_mode_count, present_modes.data()));

    data->present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // TODO: Select present mode if vertical synchronization is enabled.

    std::uint32_t min_image_count = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && min_image_count > surfaceCapabilities.maxImageCount) {
        min_image_count = surfaceCapabilities.maxImageCount;
    }

    std::uint32_t queue_indices[] = { data->graphics_family, data->present_family };

    VkSwapchainCreateInfoKHR swapchain_info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchain_info.surface = data->surface;
    swapchain_info.minImageCount = min_image_count;
    swapchain_info.imageFormat = data->surface_format.format;
    swapchain_info.imageColorSpace = data->surface_format.colorSpace;
    swapchain_info.imageExtent = { window_size.x, window_size.y };
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (data->graphics_family != data->present_family) {
        swapchain_info.queueFamilyIndexCount = sizeof(queue_indices) / sizeof(*queue_indices);
        swapchain_info.pQueueFamilyIndices = queue_indices;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    } else {
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices = nullptr;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchain_info.presentMode = data->present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.preTransform = surfaceCapabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.oldSwapchain = VK_NULL_HANDLE;

    // Create Vulkan swapchain.
    vk(vkCreateSwapchainKHR(data->device, &swapchain_info, nullptr, &data->swapchain));

    std::uint32_t image_count;
    vk(vkGetSwapchainImagesKHR(data->device, data->swapchain, &image_count, nullptr));

    // Get swapchain images list.
    data->screen_images.resize(image_count);
    vk(vkGetSwapchainImagesKHR(data->device, data->swapchain, &image_count, data->screen_images.data()));

    data->screen_image_views.resize(data->screen_images.size());
    for (std::size_t i = 0; i < data->screen_images.size(); ++i) {
        VkImageViewCreateInfo image_view_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        image_view_info.image = data->screen_images[i];
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = data->surface_format.format;
        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;
        vk(vkCreateImageView(data->device, &image_view_info, nullptr, &data->screen_image_views[i]));
    }

    VkAttachmentDescription color_attachments;
    color_attachments.flags = 0;
    color_attachments.format = data->surface_format.format;
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
    vk(vkCreateRenderPass(data->device, &render_pass_info, nullptr, &data->screen_render_pass));

    data->screen_framebuffers.resize(data->screen_image_views.size());
    for (std::size_t i = 0; i < data->screen_framebuffers.size(); ++i) {
        VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebufferInfo.renderPass = data->screen_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &data->screen_image_views[i];
        framebufferInfo.width = data->swapchain_extent.width;
        framebufferInfo.height = data->swapchain_extent.height;
        framebufferInfo.layers = 1;
        vk(vkCreateFramebuffer(data->device, &framebufferInfo, nullptr, &data->screen_framebuffers[i]));
    }

    VkCommandPoolCreateInfo command_pool_info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    command_pool_info.queueFamilyIndex = data->graphics_family;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ?
    vk(vkCreateCommandPool(data->device, &command_pool_info, nullptr, &data->command_pool));

    data->command_buffers.resize(data->screen_image_views.size());

    VkCommandBufferAllocateInfo command_buffer_alloc_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    command_buffer_alloc_info.commandPool = data->command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_alloc_info.commandBufferCount = std::uint32_t(data->command_buffers.size());
    vk(vkAllocateCommandBuffers(data->device, &command_buffer_alloc_info, data->command_buffers.data()));

    VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vk(vkCreateSemaphore(data->device, &semaphore_info, VK_NULL_HANDLE, &data->render_semaphore));
    vk(vkCreateSemaphore(data->device, &semaphore_info, VK_NULL_HANDLE, &data->present_semaphore));

    data->fences.resize(data->screen_image_views.size());

    VkFenceCreateInfo fence_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (auto& fence : data->fences) {
        vk(vkCreateFence(data->device, &fence_info, nullptr, &fence));
    }
}

void vku::quit(std::unique_ptr<renderer::data>& data) {
    for (auto& fence : data->fences) {
        vkWaitForFences(data->device, 1, &fence, VK_TRUE, 1000000000);
        vkDestroyFence(data->device, fence, nullptr);
    }
    vkFreeCommandBuffers(data->device, data->command_pool, std::uint32_t(data->command_buffers.size()), data->command_buffers.data());

    vkQueueWaitIdle(data->graphics_queue);
    vkQueueWaitIdle(data->present_queue);
    vkDeviceWaitIdle(data->device);


    vkDestroySemaphore(data->device, data->present_semaphore, nullptr);
    vkDestroySemaphore(data->device, data->render_semaphore, nullptr);

    vkDestroyCommandPool(data->device, data->command_pool, nullptr);

    for (VkFramebuffer framebuffer : data->screen_framebuffers) {
        vkDestroyFramebuffer(data->device, framebuffer, nullptr);
    }

    vkDestroyRenderPass(data->device, data->screen_render_pass, nullptr);

    for (VkImageView image_view : data->screen_image_views) {
        vkDestroyImageView(data->device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(data->device, data->swapchain, nullptr);

    vmaDestroyAllocator(data->allocator);
    vkDestroyDevice(data->device, nullptr);
    vkDestroySurfaceKHR(data->instance, data->surface, nullptr);
    vkDestroyInstance(data->instance, nullptr);
}

void vku::begin(std::unique_ptr<renderer::data>& data) {
    vk(vkAcquireNextImageKHR(data->device, data->swapchain, UINT64_MAX, data->present_semaphore, VK_NULL_HANDLE, &data->image_index));

    vkWaitForFences(data->device, 1, &data->fences[data->image_index], VK_FALSE, UINT64_MAX);
    vkResetFences(data->device, 1, &data->fences[data->image_index]);

    vkResetCommandBuffer(data->command_buffers[data->image_index], 0);

    VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(data->command_buffers[data->image_index], &begin_info);
}

void vku::end(std::unique_ptr<renderer::data>& data) {
    vkEndCommandBuffer(data->command_buffers[data->image_index]);

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &data->present_semaphore;
    submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &data->command_buffers[data->image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &data->render_semaphore;
    vkQueueSubmit(data->graphics_queue, 1, &submit_info, data->fences[data->image_index]);

    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &data->render_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &data->swapchain;
    present_info.pImageIndices = &data->image_index;
    vkQueuePresentKHR(data->present_queue, &present_info);
    vkQueueWaitIdle(data->present_queue);
}
