#pragma once

#include <rabbit/graphics/render_pass.hpp>

#include <volk.h>

namespace rb {
    class render_pass_vulkan : public render_pass {
    public:
        render_pass_vulkan(VkDevice device, const render_pass_desc& desc);

        ~render_pass_vulkan();

        VkRenderPass render_pass() const;

    private:
        VkDevice _device;
        VkRenderPass _render_pass;
    };
}
