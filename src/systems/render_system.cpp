#include <rabbit/systems/render_system.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

using namespace rb;

render_system::render_system(graphics_device& graphics_device) {
    shader_desc shader_desc;
    shader_desc.vertex_layout = {
        { vertex_attribute::position, vertex_format::vec3f() },
        { vertex_attribute::texcoord, vertex_format::vec2f() },
        { vertex_attribute::normal, vertex_format::vec3f() }
    };
    shader_desc.vertex_bytecode = forward_vert;
    shader_desc.fragment_bytecode = forward_frag;
    shader_desc.bindings = {
        { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 0, 1 },
        { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 1, 1 },
        { shader_binding_type::uniform_buffer, shader_stage_flags::fragment, 2, 1 },
        { shader_binding_type::texture, shader_stage_flags::fragment, 3, 1 }
    };
    _forward_shader = graphics_device.create_shader(shader_desc);
}

void render_system::draw(registry& registry) {

}
