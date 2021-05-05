
#include "resource_heap_vulkan.hpp"
#include "buffer_vulkan.hpp"
#include "shader_vulkan.hpp"
#include "texture_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <map>
#include <memory>
#include <algorithm>

using namespace rb;

resource_heap_vulkan::resource_heap_vulkan(VkDevice device, const resource_heap_desc& desc)
    : resource_heap(desc)
    , _device(device) {
    _create_descriptor_pool(desc);
    _create_descriptor_sets(desc);
    _update_descriptor_sets(desc);
}

resource_heap_vulkan::~resource_heap_vulkan() {
    vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
}

VkDescriptorSet resource_heap_vulkan::descriptor_set() const {
    return _descriptor_set;
}

std::vector<VkDescriptorPoolSize> resource_heap_vulkan::_create_pool_sizes(const resource_heap_desc& desc) {
    std::map<VkDescriptorType, std::uint32_t> map;
    for (auto& binding : desc.shader->bindings()) {
        map[utils_vulkan::descriptor_type(binding.type)]++;
    }

    std::vector<VkDescriptorPoolSize> output;
    std::transform(map.begin(), map.end(), std::back_inserter(output), [](const auto& pair) {
        return VkDescriptorPoolSize{ pair.first, pair.second };
    });
    return output;
}

void resource_heap_vulkan::_create_descriptor_pool(const resource_heap_desc& desc) {
    auto pool_sizes = _create_pool_sizes(desc);

    VkDescriptorPoolCreateInfo descriptor_pool_info;
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = nullptr;
    descriptor_pool_info.flags = 0;
    descriptor_pool_info.maxSets = 1;
    descriptor_pool_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    descriptor_pool_info.pPoolSizes = pool_sizes.data();

    RB_MAYBE_UNUSED auto result = vkCreateDescriptorPool(_device, &descriptor_pool_info, nullptr, &_descriptor_pool);
    RB_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan descriptol pool");
}

void resource_heap_vulkan::_create_descriptor_sets(const resource_heap_desc& desc) {
    auto native_shader = std::static_pointer_cast<shader_vulkan>(shader());

    VkDescriptorSetLayout layout = native_shader->descriptor_set_layout();

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.pNext = nullptr;
    descriptor_set_allocate_info.descriptorPool = _descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = 1; // static_cast<std::uint32_t>(_descriptor_sets.size());
    descriptor_set_allocate_info.pSetLayouts = &layout;

    RB_MAYBE_UNUSED auto result = vkAllocateDescriptorSets(_device, &descriptor_set_allocate_info, &_descriptor_set);
    RB_ASSERT(result == VK_SUCCESS, "Failed to allocate Vulkan descriptor sets");
}

void resource_heap_vulkan::_update_descriptor_sets(const resource_heap_desc& desc) {
    for (auto& binding : desc.shader->bindings()) {
        if (desc.resources.find(binding.slot) == desc.resources.end()) {
            continue;
        }

        const auto& parameter = desc.resources.at(binding.slot);

        VkWriteDescriptorSet write_info;
        VkDescriptorBufferInfo buffer_info;
        VkDescriptorImageInfo image_info;

        write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_info.pNext = nullptr;
        write_info.dstBinding = static_cast<std::uint32_t>(binding.slot);
        write_info.dstArrayElement = 0;
        write_info.descriptorCount = 1;
        write_info.descriptorType = utils_vulkan::descriptor_type(binding.type);
        write_info.pBufferInfo = nullptr;
        write_info.pImageInfo = nullptr;
        write_info.pTexelBufferView = nullptr;
        write_info.dstSet = _descriptor_set;

        switch (binding.type) {
            case shader_binding_type::uniform_buffer: {
                auto buffer = std::get<std::shared_ptr<rb::buffer>>(parameter);

                buffer_info.buffer = std::static_pointer_cast<buffer_vulkan>(buffer)->buffer();
                buffer_info.offset = 0;
                buffer_info.range = buffer->size();

                write_info.pBufferInfo = &buffer_info;
                break;
            }
            case shader_binding_type::texture: {
                auto texture = std::get<std::shared_ptr<rb::texture>>(parameter);

                image_info.sampler = std::static_pointer_cast<texture_vulkan>(texture)->sampler();
                image_info.imageView = std::static_pointer_cast<texture_vulkan>(texture)->image_view();
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                write_info.pImageInfo = &image_info;
                break;
            }
        }
        vkUpdateDescriptorSets(_device, 1, &write_info, 0, nullptr);
    }
}
