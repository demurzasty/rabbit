#include <rabbit/graphics/builtin_shaders.hpp>
#include <rabbit/graphics/shader.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

using namespace rb;

shader_desc builtin_shaders::get(builtin_shader shader) {
    shader_desc desc;

    switch (shader) {
        case builtin_shader::forward:
            desc.vertex_layout  = {
                { vertex_attribute::position, vertex_format::vec3f() },
                { vertex_attribute::texcoord, vertex_format::vec2f() },
                { vertex_attribute::normal, vertex_format::vec3f() },
            };
            desc.vertex_bytecode = forward_vert;
            desc.fragment_bytecode = forward_frag;
            desc.bindings = {
                { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 0, 1 },
                { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 1, 1 },
                { shader_binding_type::uniform_buffer, shader_stage_flags::fragment, 2, 1 },
                { shader_binding_type::texture, shader_stage_flags::fragment, 3, 1 },
            };
            break;
    }

    return desc;
}
