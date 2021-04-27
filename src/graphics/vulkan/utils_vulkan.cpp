#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/graphics/buffer.hpp>
#include <rabbit/graphics/texture.hpp>
#include <rabbit/graphics/shader.hpp>
#include <rabbit/graphics/vertex_layout.hpp>
#include <rabbit/graphics/polygon_mode.hpp>
#include <rabbit/graphics/compare_operator.hpp>
#include <rabbit/graphics/polygon_mode.hpp>
#include <rabbit/graphics/cull_mode.hpp>
#include <rabbit/graphics/front_face.hpp>
#include <rabbit/graphics/topology.hpp>
#include <rabbit/graphics/texture_type.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using namespace rb;

VkBufferUsageFlags utils_vulkan::buffer_usage_flags(const buffer_type type) {
    switch (type) {
        case buffer_type::vertex: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case buffer_type::index: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case buffer_type::uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default: RB_ASSERT(false, "Missing mapping"); return 0;
    }
}

VkFormat utils_vulkan::format(const texture_format& format) {
    switch (format) {
        case texture_format::r8: return VK_FORMAT_R8_UNORM;
        case texture_format::rg8: return VK_FORMAT_R8G8_UNORM;
        case texture_format::rgba8: return VK_FORMAT_R8G8B8A8_UNORM;
        case texture_format::d24s8: return VK_FORMAT_D24_UNORM_S8_UINT;
    }

    RB_ASSERT(false, "Failed to map Vulkan format");
    return VK_FORMAT_UNDEFINED;
}

VkFormat utils_vulkan::format(const vertex_format& format) {
    switch (format.type) {
        case vertex_format_type::floating_point:
            switch (format.components) {
                case 1: return VK_FORMAT_R32_SFLOAT;
                case 2: return VK_FORMAT_R32G32_SFLOAT;
                case 3: return VK_FORMAT_R32G32B32_SFLOAT;
                case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            break;
        case vertex_format_type::integer:
            switch (format.components) {
                case 1: return VK_FORMAT_R32_UINT;
                case 2: return VK_FORMAT_R32G32_UINT;
                case 3: return VK_FORMAT_R32G32B32_UINT;
                case 4: return VK_FORMAT_R32G32B32A32_UINT;
            }
            break;
    }

    RB_ASSERT(false, "Failed to map Vulkan format");
    return VK_FORMAT_UNDEFINED;
}

VkDescriptorType utils_vulkan::descriptor_type(const shader_binding_type& type) {
    switch (type) {
        case shader_binding_type::texture:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case shader_binding_type::uniform_buffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }

    RB_ASSERT(false, "Failed to map Vulkan descriptor type");
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkShaderStageFlags utils_vulkan::stage_flags(const std::uint32_t flags) {
    VkShaderStageFlags bitfield = 0;

    if (flags & shader_stage_flags::vertex) {
        bitfield |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (flags & shader_stage_flags::fragment) {
        bitfield |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return bitfield;
}

VkPrimitiveTopology utils_vulkan::topology(rb::topology topology) {
    switch (topology) {
        case topology::lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case topology::line_strip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case topology::triangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case topology::triangle_strip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }

    RB_ASSERT(false, "Failed to map Vulkan primitive topology");
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkPolygonMode utils_vulkan::polygon_mode(rb::polygon_mode polygon_mode) {
    switch (polygon_mode) {
        case polygon_mode::fill:
            return VK_POLYGON_MODE_FILL;
        case polygon_mode::line:
            return VK_POLYGON_MODE_LINE;
        case polygon_mode::point:
            return VK_POLYGON_MODE_POINT;
    }

    RB_ASSERT(false, "Failed to map Vulkan polygon mode");
    return VK_POLYGON_MODE_FILL;
}

VkCullModeFlags utils_vulkan::cull_mode(rb::cull_mode cull_mode) {
    switch (cull_mode) {
        case cull_mode::none:
            return VK_CULL_MODE_NONE;
        case cull_mode::front:
            return VK_CULL_MODE_FRONT_BIT;
        case cull_mode::back:
            return VK_CULL_MODE_BACK_BIT;
        case cull_mode::front_and_back:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }

    RB_ASSERT(false, "Failed to map Vulkan cull mode");
    return VK_CULL_MODE_NONE;
}

VkFrontFace utils_vulkan::front_face(rb::front_face front_face) {
    switch (front_face) {
        case front_face::clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case front_face::counter_clockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    RB_ASSERT(false, "Failed to map Vulkan front face");
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

VkCompareOp utils_vulkan::compare_operator(rb::compare_operator compare_operator) {
    switch (compare_operator) {
        case compare_operator::always:
            return VK_COMPARE_OP_ALWAYS;
        case compare_operator::never:
            return VK_COMPARE_OP_NEVER;
        case compare_operator::less:
            return VK_COMPARE_OP_LESS;
        case compare_operator::less_equal:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case compare_operator::equal:
            return VK_COMPARE_OP_EQUAL;
        case compare_operator::greater_equal:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case compare_operator::greater:
            return VK_COMPARE_OP_GREATER;
        case compare_operator::not_equal:
            return VK_COMPARE_OP_NOT_EQUAL;
    }

    RB_ASSERT(false, "Failed to map Vulkan compare operator");
    return VK_COMPARE_OP_LESS;
}

VkImageType utils_vulkan::image_type(texture_type texture_type) {
    switch (texture_type) {
        case texture_type::texture_1d:
        case texture_type::texture_1d_array:
            return VK_IMAGE_TYPE_1D;
        case texture_type::texture_2d:
        case texture_type::texture_2d_array:
        case texture_type::texture_cube:
            return VK_IMAGE_TYPE_2D;
        case texture_type::texture_3d:
            return VK_IMAGE_TYPE_3D;
    }

    RB_ASSERT(false, "Failed to map Vulkan image type");
    return VK_IMAGE_TYPE_2D;
}

VkImageViewType utils_vulkan::image_view_type(texture_type texture_type) {
    switch (texture_type) {
        case texture_type::texture_1d:
            return VK_IMAGE_VIEW_TYPE_1D;
        case texture_type::texture_1d_array:
            return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case texture_type::texture_2d:
            return VK_IMAGE_VIEW_TYPE_2D;
        case texture_type::texture_2d_array:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case texture_type::texture_3d:
            return VK_IMAGE_VIEW_TYPE_3D;
        case texture_type::texture_cube:
            return VK_IMAGE_VIEW_TYPE_CUBE;
    }

    RB_ASSERT(false, "Failed to map Vulkan image view type");
    return VK_IMAGE_VIEW_TYPE_2D;
}

VkImageCreateFlags utils_vulkan::image_create_flags(texture_type texture_type) {
    VkImageCreateFlags flags{ 0 };

    if (texture_type == texture_type::texture_cube) {
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if (texture_type == texture_type::texture_2d_array) {
        flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    }

    return flags;
}

VkFilter utils_vulkan::filter(texture_filter texture_filter) {
    switch (texture_filter) {
        case texture_filter::nearest:
            return VK_FILTER_NEAREST;
        case texture_filter::linear:
        case texture_filter::anisotropic:
            return VK_FILTER_LINEAR;
    }

    RB_ASSERT(false, "Failed to map Vulkan filter");
    return VK_FILTER_NEAREST;
}

VkSamplerAddressMode utils_vulkan::sampler_address_mode(texture_wrap texture_wrap) {
    switch (texture_wrap) {
        case texture_wrap::repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case texture_wrap::clamp:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    RB_ASSERT(false, "Failed to map Vulkan sampler address mode");
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}
