#include <rabbit/systems/renderer.hpp>
#include <rabbit/graphics/buffer.hpp>
#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/engine/entity.hpp>
#include <rabbit/graphics/command_buffer.hpp>
#include <rabbit/math/math.hpp>
#include <rabbit/graphics/material.hpp>
#include <rabbit/graphics/mesh.hpp>
#include <rabbit/graphics/builtin_shaders.hpp>
#include <rabbit/graphics/resource_heap.hpp>
#include <rabbit/graphics/shader_desc.hpp>

#include <rabbit/components/camera.hpp>
#include <rabbit/components/geometry.hpp>
#include <rabbit/components/transform.hpp>

using namespace rb;

renderer::renderer(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
}

void renderer::initialize(registry& registry) {
    _command_buffer = _graphics_device.create_command_buffer();

    buffer_desc buffer_desc;
    buffer_desc.type = buffer_type::uniform;
    buffer_desc.size = sizeof(camera_data);
    buffer_desc.stride = sizeof(camera_data);
    _camera_buffer = _graphics_device.create_buffer(buffer_desc);

    _forward_shader = _graphics_device.create_shader(builtin_shaders::get(builtin_shader::forward));
}

void renderer::draw(registry& registry, graphics_device& graphics_device) {
    _command_buffer->begin();

    registry.view<transform, camera>().each([&](transform& transform, camera& camera) {
        camera_data camera_data;
        camera_data.proj = mat4f::perspective(deg2rad(camera.field_of_view), 1280.0f / 720.0f, 0.1f, 100.0f);
        camera_data.view = invert(mat4f::translation(transform.position));

        _command_buffer->update_buffer(_camera_buffer, &camera_data, 0, sizeof(camera_data));
    });

    registry.view<transform, geometry>().each([&](const entity entity, transform& transform, geometry& geometry) {
        auto& entity_local_data = _entity_local_data[entity];
        if (!entity_local_data.local_buffer) {
            buffer_desc buffer_desc;
            buffer_desc.type = buffer_type::uniform;
            buffer_desc.size = sizeof(local_data);
            buffer_desc.stride = sizeof(local_data);
            entity_local_data.local_buffer = _graphics_device.create_buffer(buffer_desc);

            material_data material_data;
            material_data.base_color = geometry.material->base_color();
            material_data.roughness = geometry.material->roughness();
            material_data.metallic = geometry.material->metallic();

            buffer_desc.type = buffer_type::uniform;
            buffer_desc.size = sizeof(material_data);
            buffer_desc.stride = sizeof(material_data);
            buffer_desc.data = &material_data;
            entity_local_data.material_buffer = _graphics_device.create_buffer(buffer_desc);

            resource_heap_desc resource_heap_desc;
            resource_heap_desc.shader = _forward_shader;
            resource_heap_desc.resources = {
                { 0, _camera_buffer },
                { 1, entity_local_data.local_buffer },
                { 2, entity_local_data.material_buffer },
                { 3, geometry.material->albedo_map() }
            };
            entity_local_data.resource_heap = _graphics_device.create_resource_heap(resource_heap_desc);
        }

        local_data local_data;
        local_data.world = mat4f::rotation(transform.rotation);
        _command_buffer->update_buffer(entity_local_data.local_buffer, &local_data, 0, sizeof(local_data));
    });

    _command_buffer->begin_render_pass(_graphics_device);

    _command_buffer->set_shader(_forward_shader);

    registry.view<transform, geometry>().each([&](const entity entity, transform& transform, geometry& geometry) {
        auto& entity_local_data = _entity_local_data[entity];
        if (!entity_local_data.local_buffer) {
            return;
        }

        _command_buffer->set_vertex_buffer(geometry.mesh->vertex_buffer());

        _command_buffer->set_resource_heap(entity_local_data.resource_heap);

        _command_buffer->draw(geometry.mesh->vertex_buffer()->count(), 1, 0, 0);
    });

    _command_buffer->end_render_pass();

    _command_buffer->end();

    _graphics_device.submit(_command_buffer);
}
