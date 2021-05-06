#include <rabbit/systems/render_system.hpp>
#include <rabbit/components/camera.hpp>
#include <rabbit/math/math.hpp>
#include <rabbit/math/vec2.hpp>

#include <rabbit/generated/shaders/forward.vert.spv.h>
#include <rabbit/generated/shaders/forward.frag.spv.h>

#include <rabbit/generated/shaders/skybox.vert.spv.h>
#include <rabbit/generated/shaders/skybox.frag.spv.h>

#include <rabbit/generated/shaders/brdf.vert.spv.h>
#include <rabbit/generated/shaders/brdf.frag.spv.h>

#include <rabbit/generated/shaders/irradiance.vert.spv.h>
#include <rabbit/generated/shaders/irradiance.frag.spv.h>

using namespace rb;

render_system::render_system(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
    _command_buffer = graphics_device.create_command_buffer();
    _create_forward_shader();
    _create_skybox_shader();
    _create_camera_buffer();
    _create_quad();
    _create_brdf_texture();
    _create_brdf_shader();
    _bake_brdf_texture();
    _create_irradiance();
}

void render_system::draw(registry& registry) {
    _command_buffer->begin();

    registry.view<transform, camera>().each([this](transform& transform, camera& camera) {
        camera_data data;
        data.projection = mat4f::perspective(deg2rad(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
        data.view = invert(mat4f::translation(transform.position));
        _command_buffer->update_buffer(_camera_buffer, &data, 0, sizeof(data));

        if (!_skybox_mesh && camera.skybox) {
            _create_skybox(camera.skybox);
            _skybox_texture = camera.skybox;

            _bake_irradiance_texture(camera.skybox);
        }
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

    if (_skybox_mesh) {
        _command_buffer->set_shader(_skybox_shader);

        _command_buffer->set_resource_heap(_skybox_resource_heap);
        _command_buffer->set_vertex_buffer(_skybox_mesh->vertex_buffer());
        _command_buffer->set_index_buffer(_skybox_mesh->index_buffer());
        _command_buffer->draw_indexed(_skybox_mesh->index_buffer()->count(), 1, 0, 0, 0);
    }

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
        { shader_binding_type::texture, shader_stage_flags::fragment, 3, 1 },
        { shader_binding_type::texture, shader_stage_flags::fragment, 4, 1 },
        { shader_binding_type::texture, shader_stage_flags::fragment, 5, 1 }
    };
    _forward_shader = _graphics_device.create_shader(shader_desc);
}

void render_system::_create_skybox_shader() {
    shader_desc shader_desc;
    shader_desc.vertex_layout = { { vertex_attribute::position, vertex_format::vec3f() } };
    shader_desc.vertex_bytecode = skybox_vert;
    shader_desc.fragment_bytecode = skybox_frag;
    shader_desc.bindings = {
        { shader_binding_type::uniform_buffer, shader_stage_flags::vertex, 0, 1 },
        { shader_binding_type::texture, shader_stage_flags::fragment, 1, 1 }
    };
    shader_desc.depth_compare_operator = compare_operator::less_equal;
    _skybox_shader = _graphics_device.create_shader(shader_desc);
}

void render_system::_create_quad() {
    const vec2f vertices[] = {
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        {  1.0f,  1.0f },
        { -1.0f, 1.0f },
    };

    const std::uint16_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    buffer_desc buffer_desc;
    buffer_desc.type = buffer_type::vertex;
    buffer_desc.data = vertices;
    buffer_desc.stride = sizeof(vec2f);
    buffer_desc.size = sizeof(vertices);
    auto vertex_buffer = _graphics_device.create_buffer(buffer_desc);

    buffer_desc.type = buffer_type::index;
    buffer_desc.data = indices;
    buffer_desc.stride = sizeof(std::uint16_t);
    buffer_desc.size = sizeof(indices);
    auto index_buffer = _graphics_device.create_buffer(buffer_desc);

    mesh_desc mesh_desc;
    mesh_desc.vertex_layout = { { vertex_attribute::position, vertex_format::vec2f() } };
    mesh_desc.vertex_buffer = vertex_buffer;
    mesh_desc.index_buffer = index_buffer;
    _quad = _graphics_device.create_mesh(mesh_desc);
}

void render_system::_create_brdf_texture() {
    texture_desc texture_desc;
    texture_desc.type = texture_type::texture_2d;
    texture_desc.size = { 512, 512, 0 };
    texture_desc.format = texture_format::rg8;
    texture_desc.filter = texture_filter::linear;
    texture_desc.wrap = texture_wrap::clamp;
    texture_desc.mipmaps = 1;
    texture_desc.layers = 1;
    texture_desc.is_render_target = true;
    _brdf_texture = _graphics_device.create_texture(texture_desc);
}

void render_system::_create_brdf_shader() {
    shader_desc shader_desc;
    shader_desc.vertex_layout = { { vertex_attribute::position, vertex_format::vec2f() } };
    shader_desc.vertex_bytecode = brdf_vert;
    shader_desc.fragment_bytecode = brdf_frag;
    shader_desc.render_target = _brdf_texture;
    _brdf_shader = _graphics_device.create_shader(shader_desc);
}

void render_system::_bake_brdf_texture() {
    auto command_buffer = _graphics_device.create_command_buffer();

    command_buffer->begin();

    command_buffer->set_shader(_brdf_shader);

    command_buffer->begin_render_pass(_brdf_texture, 0);

    command_buffer->set_vertex_buffer(_quad->vertex_buffer());
    command_buffer->set_index_buffer(_quad->index_buffer());
    command_buffer->draw_indexed(6, 1, 0, 0, 0);

    command_buffer->end_render_pass();

    command_buffer->end();

    _graphics_device.submit(command_buffer);
}

void render_system::_create_camera_buffer() {
    buffer_desc desc;
    desc.type = buffer_type::uniform;
    desc.size = desc.stride = sizeof(camera_data);
    _camera_buffer = _graphics_device.create_buffer(desc);
}

void render_system::_create_skybox(std::shared_ptr<texture> skybox) {
    const vec3f cube_vertices[24] = {
		{ -1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },

		{ -1.0f, 1.0f, -1.0f },
		{ -1.0f, 1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f },

		{ 1.0f, 1.0f, -1.0f },
		{ -1.0f, 1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f },

		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, 1.0f },

		{ -1.0f, 1.0f, -1.0f },
		{ 1.0f, 1.0f, -1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f },

		{ -1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
    };

    const std::uint16_t cube_indices[36] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };

    buffer_desc buffer_desc;
    buffer_desc.type = buffer_type::vertex;
    buffer_desc.data = cube_vertices;
    buffer_desc.stride = sizeof(vec3f);
    buffer_desc.size = sizeof(cube_vertices);
    auto cube_vertex_buffer = _graphics_device.create_buffer(buffer_desc);

    buffer_desc.type = buffer_type::index;
    buffer_desc.data = cube_indices;
    buffer_desc.stride = sizeof(std::uint16_t);
    buffer_desc.size = sizeof(cube_indices);
    auto cube_index_buffer = _graphics_device.create_buffer(buffer_desc);

    mesh_desc mesh_desc;
    mesh_desc.vertex_layout = { { vertex_attribute::position, vertex_format::vec3f() } };
    mesh_desc.vertex_buffer = cube_vertex_buffer;
    mesh_desc.index_buffer = cube_index_buffer;
    _skybox_mesh = _graphics_device.create_mesh(mesh_desc);

    resource_heap_desc resource_heap_desc;
    resource_heap_desc.shader = _skybox_shader;
    resource_heap_desc.resources = {
        { 0, _camera_buffer },
        { 1, skybox }
    };
    _skybox_resource_heap = _graphics_device.create_resource_heap(resource_heap_desc);
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
        { 3, geometry.material->albedo_map() },
        { 4, _skybox_texture },
        { 5, _irradiance_texture }
    };
    geometry_data.resource_heap = _graphics_device.create_resource_heap(resource_heap_desc);
}

void render_system::_create_irradiance() {
    texture_desc texture_desc;
    texture_desc.type = texture_type::texture_cube;
    texture_desc.size = { 64, 64, 0 };
    texture_desc.format = texture_format::rgba8;
    texture_desc.filter = texture_filter::linear;
    texture_desc.wrap = texture_wrap::clamp;
    texture_desc.mipmaps = 1;
    texture_desc.layers = 6;
    texture_desc.is_render_target = true;
    _irradiance_texture = _graphics_device.create_texture(texture_desc);

    shader_desc shader_desc;
    shader_desc.vertex_layout = { { vertex_attribute::position, vertex_format::vec2f() } };
    shader_desc.vertex_bytecode = irradiance_vert;
    shader_desc.fragment_bytecode = irradiance_frag;
    shader_desc.bindings = {
        { shader_binding_type::uniform_buffer, shader_stage_flags::fragment, 0, 1 },
        { shader_binding_type::texture, shader_stage_flags::fragment, 1, 1 }
    };
    shader_desc.depth_test_enable = false;
    shader_desc.depth_write_enable = false;
    shader_desc.render_target = _irradiance_texture;
    _irradiance_shader = _graphics_device.create_shader(shader_desc);

    buffer_desc buffer_desc;
    buffer_desc.type = buffer_type::uniform;
    buffer_desc.size = buffer_desc.stride = sizeof(irradiance_data);
    _irradiance_buffer = _graphics_device.create_buffer(buffer_desc);
}

void render_system::_bake_irradiance_texture(std::shared_ptr<texture> skybox) {
    resource_heap_desc resource_heap_desc;
    resource_heap_desc.shader = _irradiance_shader;
    resource_heap_desc.resources = {
        { 0, _irradiance_buffer },
        { 1, skybox }
    };
    _irradiance_resource_heap = _graphics_device.create_resource_heap(resource_heap_desc);

    auto command_buffer = _graphics_device.create_command_buffer();

    command_buffer->begin();

    command_buffer->set_shader(_irradiance_shader);
    command_buffer->set_resource_heap(_irradiance_resource_heap);

    irradiance_data data;
    for (std::size_t layer{ 0 }; layer < 6; ++layer) {
        data.cube_face = static_cast<std::uint32_t>(layer);
        command_buffer->update_buffer(_irradiance_buffer, &data, 0, sizeof(data));

        command_buffer->begin_render_pass(_irradiance_texture, layer);

        command_buffer->set_vertex_buffer(_quad->vertex_buffer());
        command_buffer->set_index_buffer(_quad->index_buffer());
        command_buffer->draw_indexed(6, 1, 0, 0, 0);

        command_buffer->end_render_pass();
    }

    command_buffer->end();

    _graphics_device.submit(command_buffer);
}
