#include <rabbit/graphics/builtin_shaders.hpp>
#include <rabbit/graphics/shader.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

#include <rabbit/generated/shaders/skybox.vert.spv.h>
#include <rabbit/generated/shaders/skybox.frag.spv.h>

#include <rabbit/generated/shaders/brdf.vert.spv.h>
#include <rabbit/generated/shaders/brdf.frag.spv.h>

#include <rabbit/generated/shaders/irradiance.vert.spv.h>
#include <rabbit/generated/shaders/irradiance.frag.spv.h>

#include <rabbit/generated/shaders/prefilter.vert.spv.h>
#include <rabbit/generated/shaders/prefilter.frag.spv.h>

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
        case builtin_shader::skybox:
            desc.vertex_layout  = {
                { vertex_attribute::position, vertex_format::vec3f() }
            };
            desc.vertex_bytecode = skybox_vert;
            desc.fragment_bytecode = skybox_frag;
            desc.bindings = {
                { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 0, 1 },
                { shader_binding_type::texture, shader_stage_flags::fragment, 1, 1 },
            };
            desc.depth_stencil_state.compare_operator = compare_operator::less_equal;
            break;
        case builtin_shader::brdf:
            desc.vertex_layout  = {
                { vertex_attribute::position, vertex_format::vec2f() }
            };
            desc.vertex_bytecode = brdf_vert;
            desc.fragment_bytecode = brdf_frag;
            break;
        case builtin_shader::irradiance:
            desc.vertex_layout  = {
                { vertex_attribute::position, vertex_format::vec2f() }
            };
            desc.vertex_bytecode = irradiance_vert;
            desc.fragment_bytecode = irradiance_frag;
            break;
        case builtin_shader::prefilter:
            desc.vertex_layout  = {
                { vertex_attribute::position, vertex_format::vec2f() }
            };
            desc.vertex_bytecode = prefilter_vert;
            desc.fragment_bytecode = prefilter_frag;
            desc.bindings = {
                { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 0, 1 },
                { shader_binding_type::texture, shader_stage_flags::fragment, 1, 1 },
            };
            break;
    }

    return desc;
}
