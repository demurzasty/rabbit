#include "utils_vulkan.hpp"

#include "shaders/gen/canvas.vert.spv.h"
#include "shaders/gen/canvas.frag.spv.h"

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
    app_info.apiVersion = VK_API_VERSION_1_2;

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
    }

    for (std::uint32_t i = 0; i < queue_family_count; ++i) {
        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(data->physical_device, i, data->surface, &present_support);

        if (data->graphics_family != i && present_support == VK_TRUE) {
            data->present_family = i;
            break;
        }
    }

    // Fill queue priorities array.
    float queue_priorities[] = { 1.0f };

    // Fill graphics device queue create informations.
    VkDeviceQueueCreateInfo device_graphics_queue_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    device_graphics_queue_info.queueFamilyIndex = data->graphics_family;
    device_graphics_queue_info.queueCount = 1;
    device_graphics_queue_info.pQueuePriorities = queue_priorities;

    // Fill present queue create informations.
    VkDeviceQueueCreateInfo device_present_queue_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    device_present_queue_info.queueFamilyIndex = data->present_family;
    device_present_queue_info.queueCount = 1;
    device_present_queue_info.pQueuePriorities = queue_priorities;

    VkDeviceQueueCreateInfo device_queue_infos[] = {
        device_graphics_queue_info,
        device_present_queue_info
    };

    // Fill logical device extensions array.
    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Need swapchain to present render result onto a screen.
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        // VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, // For resolve multisampled depth
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
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(data->physical_device, data->surface, &surface_capabilities));

    // Store swapchain extent
    data->swapchain_extent = surface_capabilities.currentExtent;

    // Get window size.
    const auto window_size = window.size();

    std::uint32_t present_mode_count = 0;
    vk(vkGetPhysicalDeviceSurfacePresentModesKHR(data->physical_device, data->surface, &present_mode_count, nullptr));

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vk(vkGetPhysicalDeviceSurfacePresentModesKHR(data->physical_device, data->surface, &present_mode_count, present_modes.data()));

    data->present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // TODO: Select present mode if vertical synchronization is enabled.

    std::uint32_t min_image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && min_image_count > surface_capabilities.maxImageCount) {
        min_image_count = surface_capabilities.maxImageCount;
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
    swapchain_info.preTransform = surface_capabilities.currentTransform;
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
        VkFramebufferCreateInfo framebuffer_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        framebuffer_info.renderPass = data->screen_render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = &data->screen_image_views[i];
        framebuffer_info.width = data->swapchain_extent.width;
        framebuffer_info.height = data->swapchain_extent.height;
        framebuffer_info.layers = 1;
        vk(vkCreateFramebuffer(data->device, &framebuffer_info, nullptr, &data->screen_framebuffers[i]));
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


    VkBufferCreateInfo canvas_vertex_buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    canvas_vertex_buffer_info.size = sizeof(vertex2d) * 0x10000;
    canvas_vertex_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    canvas_vertex_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    canvas_vertex_buffer_info.queueFamilyIndexCount = 0;
    canvas_vertex_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo canvas_vertex_buffer_allocation_info{};
    canvas_vertex_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    canvas_vertex_buffer_allocation_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vk(vmaCreateBuffer(data->allocator, &canvas_vertex_buffer_info, &canvas_vertex_buffer_allocation_info, &data->canvas_vertex_buffer, &data->canvas_vertex_buffer_allocation, nullptr));

    VkBufferCreateInfo canvas_index_buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    canvas_index_buffer_info.size = sizeof(std::uint32_t) * 0x10000;
    canvas_index_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    canvas_index_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    canvas_index_buffer_info.queueFamilyIndexCount = 0;
    canvas_index_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo canvas_index_buffer_allocation_info{};
    canvas_index_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    canvas_index_buffer_allocation_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vk(vmaCreateBuffer(data->allocator, &canvas_index_buffer_info, &canvas_index_buffer_allocation_info, &data->canvas_index_buffer, &data->canvas_index_buffer_allocation, nullptr));

    // Create bindless descriptor pool
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 + 1 },
    };

    // Update after bind is needed here, for each binding and in the descriptor set layout creation.
    VkDescriptorPoolCreateInfo descriptor_pool_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    descriptor_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    descriptor_pool_info.maxSets = 1024 + 1;
    descriptor_pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(*pool_sizes);
    descriptor_pool_info.pPoolSizes = pool_sizes;
    vk(vkCreateDescriptorPool(data->device, &descriptor_pool_info, nullptr, &data->main_descriptor_pool));


    VkDescriptorSetLayoutBinding layout_bindings[1];
    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_bindings[0].descriptorCount = 1024;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    layout_bindings[0].pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags binding_flags[1]{
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendend_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT };
    extendend_info.bindingCount = 1;
    extendend_info.pBindingFlags = binding_flags;

    VkDescriptorSetLayoutCreateInfo descriptor_layout_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    descriptor_layout_info.pNext = &extendend_info;
    descriptor_layout_info.bindingCount = 1;
    descriptor_layout_info.pBindings = layout_bindings;
    descriptor_layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    vk(vkCreateDescriptorSetLayout(data->device, &descriptor_layout_info, nullptr, &data->main_descriptor_set_layout));

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
    count_info.descriptorSetCount = 1;
    // This number is the max allocatable count
    std::uint32_t max_sampler_binding = 1023;
    count_info.pDescriptorCounts = &max_sampler_binding;

    VkDescriptorSetAllocateInfo descriptor_set_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptor_set_info.pNext = &count_info;
    descriptor_set_info.descriptorPool = data->main_descriptor_pool;
    descriptor_set_info.descriptorSetCount = 1;
    descriptor_set_info.pSetLayouts = &data->main_descriptor_set_layout;
    vk(vkAllocateDescriptorSets(data->device, &descriptor_set_info, &data->main_descriptor_set));

    VkPushConstantRange push_constant_range;
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(int);

    VkPipelineLayoutCreateInfo pipeline_layout_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &data->main_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;
    vk(vkCreatePipelineLayout(data->device, &pipeline_layout_info, nullptr, &data->pipeline_layout));

    VkShaderModule shader_modules[2];

    VkShaderModuleCreateInfo vertex_shader_module_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    vertex_shader_module_info.codeSize = sizeof(canvas_vert_spv);
    vertex_shader_module_info.pCode = (const std::uint32_t*)canvas_vert_spv;
    vk(vkCreateShaderModule(data->device, &vertex_shader_module_info, nullptr, &shader_modules[0]));

    VkShaderModuleCreateInfo fragment_shader_module_info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    fragment_shader_module_info.codeSize = sizeof(canvas_frag_spv);
    fragment_shader_module_info.pCode = (const std::uint32_t*)canvas_frag_spv;
    vk(vkCreateShaderModule(data->device, &fragment_shader_module_info, nullptr, &shader_modules[1]));

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
    viewport.width = float(data->swapchain_extent.width);
    viewport.height = float(data->swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = data->swapchain_extent;

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
    pipeline_info.layout = data->pipeline_layout;
    pipeline_info.renderPass = data->screen_render_pass;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pDynamicState = &dynamic_state_info;
    vk(vkCreateGraphicsPipelines(data->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &data->pipeline));

    vkDestroyShaderModule(data->device, shader_modules[1], nullptr);
    vkDestroyShaderModule(data->device, shader_modules[0], nullptr);
}

void vku::quit(std::unique_ptr<renderer::data>& data) {
    vkWaitForFences(data->device, std::uint32_t(data->fences.size()), data->fences.data(), VK_TRUE, UINT64_MAX);
    for (auto& fence : data->fences) {
        vkDestroyFence(data->device, fence, nullptr);
    }

    vkFreeCommandBuffers(data->device, data->command_pool, std::uint32_t(data->command_buffers.size()), data->command_buffers.data());

    vkQueueWaitIdle(data->graphics_queue);
    vkQueueWaitIdle(data->present_queue);
    vkDeviceWaitIdle(data->device);

    cleanup(data);

    data->textures.each([&data](handle id, texture_data& texture) {
        cleanup_texture(data, texture);
    });

    vkDestroyPipeline(data->device, data->pipeline, nullptr);
    vkDestroyPipelineLayout(data->device, data->pipeline_layout, nullptr);

    vkDestroyDescriptorSetLayout(data->device, data->main_descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(data->device, data->main_descriptor_pool, nullptr);

    vmaDestroyBuffer(data->allocator, data->canvas_index_buffer, data->canvas_index_buffer_allocation);
    vmaDestroyBuffer(data->allocator, data->canvas_vertex_buffer, data->canvas_vertex_buffer_allocation);

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

void vku::cleanup(std::unique_ptr<renderer::data>& data) {
    // Ready to cleanup.
    while (!data->textures_to_delete.empty()) {
        cleanup_texture(data, data->textures_to_delete.front());
        data->textures_to_delete.pop();
    }
}

void vku::begin(std::unique_ptr<renderer::data>& data) {
    vk(vkAcquireNextImageKHR(data->device, data->swapchain, UINT64_MAX, data->present_semaphore, VK_NULL_HANDLE, &data->image_index));

    vkWaitForFences(data->device, 1, &data->fences[data->image_index], VK_FALSE, UINT64_MAX);
    vkResetFences(data->device, 1, &data->fences[data->image_index]);

    vkResetCommandBuffer(data->command_buffers[data->image_index], 0);

    VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(data->command_buffers[data->image_index], &begin_info);
}

void vku::end(std::unique_ptr<renderer::data>& data) {
    vk(vkEndCommandBuffer(data->command_buffers[data->image_index]));

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT |
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &data->present_semaphore;
    submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &data->command_buffers[data->image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &data->render_semaphore;
    vk(vkQueueSubmit(data->graphics_queue, 1, &submit_info, data->fences[data->image_index]));

    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &data->render_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &data->swapchain;
    present_info.pImageIndices = &data->image_index;
    vk(vkQueuePresentKHR(data->present_queue, &present_info));
    vk(vkQueueWaitIdle(data->present_queue));
}

VkDeviceSize vku::get_bits_per_pixel(pixel_format format) {
    switch (format) {
        case pixel_format::r8: return 8;
        case pixel_format::rg8: return 16;
        case pixel_format::rgba8: return 32;
        case pixel_format::bc1: return 4;
        case pixel_format::bc3: return 8;
    }

    assert(0);
    return 0;
}

VkFormat vku::get_pixel_format(pixel_format format) {
    switch (format) {
        case pixel_format::r8: return VK_FORMAT_R8_UNORM;
        case pixel_format::rg8: return VK_FORMAT_R8G8_UNORM;
        case pixel_format::rgba8: return VK_FORMAT_R8G8B8A8_UNORM;
        case pixel_format::bc1: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case pixel_format::bc3: return VK_FORMAT_BC3_UNORM_BLOCK;
    }

    assert(0);
    return VK_FORMAT_UNDEFINED;
}

VkFilter vku::get_filter(texture_filter filter) {
    switch (filter) {
        case texture_filter::nearest: return VK_FILTER_NEAREST;
        case texture_filter::linear: return VK_FILTER_LINEAR;
    }

    assert(0);
    return VK_FILTER_MAX_ENUM;
}

texture_data vku::create_texture(std::unique_ptr<renderer::data>& data, const uvec2& size, texture_filter filter, pixel_format format) {
    texture_data texture;

    VkImageCreateInfo image_info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = get_pixel_format(format);
    image_info.extent = { size.x, size.y, 1 };
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
    vk(vmaCreateImage(data->allocator, &image_info, &allocation_info, &texture.image, &texture.allocation, nullptr));

    VkImageViewCreateInfo image_view_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    image_view_info.image = texture.image;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = image_info.format;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;
    vk(vkCreateImageView(data->device, &image_view_info, nullptr, &texture.image_view));

    VkSamplerCreateInfo sampler_info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampler_info.magFilter = get_filter(filter);
    sampler_info.minFilter = sampler_info.magFilter;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
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
    vk(vkCreateSampler(data->device, &sampler_info, nullptr, &texture.sampler));

    texture.size = size;
    texture.format = format;
    return texture;
}

void vku::update_texture(std::unique_ptr<renderer::data>& data, texture_data& texture, const void* pixels) {
    // Create staging buffer.
    VkBufferCreateInfo buffer_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    buffer_info.size = VkDeviceSize(texture.size.x) * texture.size.y * get_bits_per_pixel(texture.format) / 8;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = 0;
    buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo buffer_allocation_info{};
    buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_allocation;
    vk(vmaCreateBuffer(data->allocator, &buffer_info, &buffer_allocation_info, &staging_buffer, &staging_buffer_allocation, nullptr));

    // Transfer pixels into buffer.
    void* ptr;
    vk(vmaMapMemory(data->allocator, staging_buffer_allocation, &ptr));
    memcpy(ptr, pixels, buffer_info.size);
    vmaUnmapMemory(data->allocator, staging_buffer_allocation);

    // Create temporary buffer
    VkCommandBufferAllocateInfo command_buffer_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_info.commandPool = data->command_pool;
    command_buffer_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(data->device, &command_buffer_info, &command_buffer);

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
    barrier.image = texture.image;
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
    region.imageExtent = { texture.size.x, texture.size.y, 1 };
    vkCmdCopyBufferToImage(command_buffer, staging_buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.image;
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

    vkQueueSubmit(data->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(data->graphics_queue);

    vkFreeCommandBuffers(data->device, data->command_pool, 1, &command_buffer);

    vmaDestroyBuffer(data->allocator, staging_buffer, staging_buffer_allocation);
}

void vku::cleanup_texture(std::unique_ptr<renderer::data>& data, texture_data& texture) {
    if (texture.image) {
        vkDestroySampler(data->device, texture.sampler, nullptr);
        vkDestroyImageView(data->device, texture.image_view, nullptr);
        vmaDestroyImage(data->allocator, texture.image, texture.allocation);
    }

    texture = {};
}
