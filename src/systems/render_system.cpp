#include <rabbit/systems/render_system.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

using namespace rb;

render_system::render_system(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
    _command_buffer = graphics_device.create_command_buffer();
    _create_forward_shader();
}

void render_system::draw(registry& registry) {
    _command_buffer->begin();

    _command_buffer->begin_render_pass(_graphics_device);

    _command_buffer->end_render_pass();

    _command_buffer->end();

    _graphics_device.submit(_command_buffer);
}

void render_system::_create_forward_shader() {
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
    _forward_shader = _graphics_device.create_shader(shader_desc);
}