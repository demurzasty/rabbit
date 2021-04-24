#pragma once

#include <rabbit/graphics/command_buffer.hpp>

#include <volk.h>

namespace rb {
    class command_buffer_vulkan : public command_buffer {
    public:
        command_buffer_vulkan(VkDevice device, VkCommandPool command_pool);

        ~command_buffer_vulkan();

        void begin() override;

        void end() override;

        void update_buffer(const std::shared_ptr<buffer>& buffer, const void* data, std::size_t offset, std::size_t size) override;

        void begin_render_pass(const std::shared_ptr<graphics_device>& graphics_device) override;

        void end_render_pass() override;

        void set_shader(const std::shared_ptr<shader>& shader) override;

        void set_resource_heap(const std::shared_ptr<resource_heap>& resource_heap) override;

        void set_vertex_buffer(const std::shared_ptr<buffer>& vertex_buffer) override;

        void draw(std::size_t vertex_count, std::size_t instance_count, std::size_t first_vertex, std::size_t first_instance) override;

        VkCommandBuffer command_buffer() const;

        VkFence fence() const;

    private:
        VkDevice _device;
        VkCommandPool _command_pool;
        VkCommandBuffer _command_buffer;
        VkFence _fence;
    };
}
