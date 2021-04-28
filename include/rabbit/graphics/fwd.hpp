#pragma once

namespace rb {
    enum class buffer_type;
    struct buffer_desc;
    class buffer;

    enum class builtin_shader;
    struct builtin_shaders;

    class command_buffer;

    enum class compare_operator;
    enum class cull_mode;
    enum class front_face;

    class graphics_device_factory;

    struct graphics_device_desc;
    class graphics_device;

    struct render_pass_desc;
    class render_pass;

    struct material_desc;
    class material;

    struct mesh_desc;
    class mesh;

    enum class polygon_mode;

    struct resource_heap_desc;
    class resource_heap;

    struct shader_stage_flags;
    enum class shader_binding_type;
    struct shader_binding_desc;
    struct shader_desc;
    class shader;

    enum class texture_filter;
    enum class texture_format;
    enum class texture_type;
    enum class texture_wrap;
    struct texture_desc;
    class texture;

    enum class topology;

    enum class vertex_format_type;
    enum class vertex_attribute;
    struct vertex_format;
    struct vertex_element;
    class vertex_desc;
}
