#pragma once

#include "../engine/system.hpp"
#include "../graphics/fwd.hpp"
#include "../math/mat4.hpp"
#include "../math/vec3.hpp"

#include <memory>
#include <unordered_map>

namespace rb {
    class renderer : public system {
    public:
        renderer(graphics_device& graphics_device);

        ~renderer();

        void initialize(registry& registry);

        void draw(registry& registry, graphics_device& graphics_device) override;

    private:
        struct camera_data {
            mat4f proj;
            mat4f view;
        };

        struct local_data {
            mat4f world;
        };

        struct material_data {
            vec3f base_color;
            float roughness;
            float metallic;
        };

        struct entity_local_data {
            local_data local_data;
            std::shared_ptr<buffer> local_buffer;
            std::shared_ptr<buffer> material_buffer;
            std::shared_ptr<resource_heap> resource_heap;
        };

    private:
        graphics_device& _graphics_device;
        std::shared_ptr<buffer> _camera_buffer;
        std::shared_ptr<shader> _forward_shader;
        std::unordered_map<entity, entity_local_data> _entity_local_data;
        std::shared_ptr<command_buffer> _command_buffer;
    };
}
