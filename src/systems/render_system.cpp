#include <rabbit/systems/render_system.hpp>
#include <rabbit/components/camera.hpp>
#include <rabbit/math/math.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

using namespace rb;

render_system::render_system(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
    _command_buffer = graphics_device.create_command_buffer();
    _create_forward_shader();
    _create_camera_buffer();
}

void render_system::draw(registry& registry) {
    _command_buffer->begin();

    registry.view<transform, camera>().each([this](transform& transform, camera& camera) {
        camera_data data;
        data.projection = mat4f::perspective(deg2rad(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
        data.view = invert(mat4f::translation(transform.position));
        _command_buffer->update_buffer(_camera_buffer, &data, 0, sizeof(data));
    });

    registry.view<transform, geometry>().each([this](const entity entity, transform& transform, geometry& geometry) {
        auto& geometry_data = _geometry_data[entity];
        if (!geometry_data.local_buffer) {
            _create_geometry_data(transform, geometry, geometry_data);
        }

        // TODO: Update on transform change

        local_data data;
        data.world = mat4f::rotation(transform.rotation) * mat4f::translation(transform.position);
        _command_buffer->update_buffer(geometry_data.local_buffer, &data, 0, sizeof(data));
    });

    _command_buffer->begin_render_pass(_graphics_device);

    _command_buffer->set_shader(_forward_shader);
    registry.view<transform, geometry>().each([this](const entity entity, transform& transform, geometry& geometry) {
        auto& geometry_data = _geometry_data[entity];

        _command_buffer->set_resource_heap(geometry_data.resource_heap);
        _command_buffer->set_vertex_buffer(geometry.mesh->vertex_buffer());
        _command_buffer->draw(geometry.mesh->vertex_buffer()->count(), 1, 0, 0);
    });

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

void render_system::_create_camera_buffer() {
    buffer_desc desc;
    desc.type = buffer_type::uniform;
    desc.size = desc.stride = sizeof(camera_data);
    _camera_buffer = _graphics_device.create_buffer(desc);
}

void render_system::_create_geometry_data(transform& transform, geometry& geometry, geometry_data& geometry_data) {
    buffer_desc buffer_desc;
    buffer_desc.type = buffer_type::uniform;
    buffer_desc.size = buffer_desc.stride = sizeof(local_data);
    geometry_data.local_buffer = _graphics_device.create_buffer(buffer_desc);

    resource_heap_desc resource_heap_desc;
    resource_heap_desc.shader = _forward_shader;
    resource_heap_desc.resources = {
        { 0, _camera_buffer },
        { 1, geometry_data.local_buffer },
        { 2, geometry.material },
        { 3, geometry.material->albedo_map() }
    };
    geometry_data.resource_heap = _graphics_device.create_resource_heap(resource_heap_desc);
}
