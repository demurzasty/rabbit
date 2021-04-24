#pragma once

#include <rabbit/graphics/resource_heap.hpp>

#include <volk.h>

#include <vector>

namespace rb {
    class resource_heap_vulkan : public resource_heap {
    public:
        resource_heap_vulkan(VkDevice device, const resource_heap_desc& desc);

        ~resource_heap_vulkan();

        VkDescriptorSet descriptor_set() const;

    private:
        std::vector<VkDescriptorPoolSize> _create_pool_sizes(const resource_heap_desc& desc, const std::vector<shader_binding_desc>& bindings);

        void _create_descriptor_pool(const resource_heap_desc& desc, const std::vector<shader_binding_desc>& bindings);

        void _create_descriptor_sets(const resource_heap_desc& desc);

        void _update_descriptor_sets(const resource_heap_desc& desc, const std::vector<shader_binding_desc>& bindings);

    private:
        VkDevice _device;
        VkDescriptorPool _descriptor_pool;
        VkDescriptorSet _descriptor_set;
    };
}
