#pragma once

#include <cstdint>

namespace rb {
    enum class buffer_type;
    struct buffer_desc;
    class buffer;

    enum class builtin_shader;
    struct builtin_shaders;

    struct graphics_device_desc;
    class graphics_device;

    class graphics_device_manager;

    class command_buffer;

    struct shader_stage_flags;
    enum class shader_binding_type;
    struct shader_binding_desc;
    struct shader_desc;
    class shader;

    enum class texture_format;
    struct texture_desc;
    class texture;

    enum class vertex_format_type : std::uint8_t;
    enum class vertex_attribute : std::uint8_t;
    struct vertex_format;
    struct vertex_element;
    class vertex_desc;
}
