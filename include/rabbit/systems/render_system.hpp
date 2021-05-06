#pragma once

#include "../core/system.hpp"
#include "../graphics/graphics_device.hpp"
#include "../graphics/shader.hpp"
#include "../graphics/buffer.hpp"
#include "../graphics/texture.hpp"
#include "../graphics/mesh.hpp"
#include "../math/mat4.hpp"
#include "../components/transform.hpp"
#include "../components/geometry.hpp"

#include <unordered_map>

namespace rb {
    class render_system : public system {
    private:
        struct alignas(16) camera_data {
            mat4f projection;
            mat4f view;
        };

        struct alignas(16) local_data {
            mat4f world;
        };

        struct geometry_data {
            std::shared_ptr<buffer> local_buffer;
            std::shared_ptr<resource_heap> resource_heap;
        };

    public:
        render_system(graphics_device& graphics_device);

        void draw(registry& registry) override;

    private:
        void _create_forward_shader();

        void _create_skybox_shader();

        void _create_quad();

        void _create_brdf_texture();

        void _create_brdf_shader();

        void _bake_brdf_texture();

        void _create_camera_buffer();

        void _create_skybox(std::shared_ptr<texture> skybox);

        void _create_geometry_data(transform& transform, geometry& geometry, geometry_data& geometry_data);

    private:
        graphics_device& _graphics_device;
        std::shared_ptr<shader> _forward_shader;
        std::shared_ptr<buffer> _camera_buffer;

        std::shared_ptr<shader> _skybox_shader;
        std::shared_ptr<mesh> _skybox_mesh;
        std::shared_ptr<resource_heap> _skybox_resource_heap;

        std::shared_ptr<shader> _brdf_shader;
        std::shared_ptr<mesh> _quad;
        std::shared_ptr<texture> _brdf_texture;

        std::unordered_map<entity, geometry_data> _geometry_data;
        std::shared_ptr<command_buffer> _command_buffer;
    };
}
