#pragma once

#include <rabbit/graphics/mesh.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class mesh_vulkan : public mesh {
    public:
        mesh_vulkan(const mesh_desc& desc);
    };
}
