
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

        void begin_render_pass(graphics_device& graphics_device) override;

        void begin_render_pass(const std::shared_ptr<texture>& render_target) override;

        void end_render_pass() override;

        void set_viewport(const vec4f& viewport) override;

        void draw(const std::shared_ptr<mesh>& mesh, const std::shared_ptr<material>& material) override;

        VkCommandBuffer command_buffer() const;

        VkFence fence() const;

    private:
        VkDevice _device;
        VkCommandPool _command_pool;
        VkCommandBuffer _command_buffer;
        VkFence _fence;
    };
}
