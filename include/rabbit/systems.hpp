#pragma once 

#include "components.hpp"

namespace rb::systems {
    void render_sprites(registry& p_registry, graphics& p_graphics) {
        p_registry.view<transform, sprite>().each([&p_graphics](transform& p_transform, sprite& p_sprite) {
            if (p_sprite.texture != null) {
                float width = float(p_graphics.get_texture_width(p_sprite.texture));
                float height = float(p_graphics.get_texture_height(p_sprite.texture));

                vertex2d vertices[]{
                    { { p_transform.position.x, p_transform.position.y }, { 0.0f, 0.0f }, colors::white },
                    { { p_transform.position.x, p_transform.position.y + height }, { 0.0f, 1.0f }, colors::white },
                    { { p_transform.position.x + width, p_transform.position.y + height }, { 1.0f, 1.0f }, colors::white },
                    { { p_transform.position.x + width, p_transform.position.y }, { 1.0f, 0.0f }, colors::white },
                };

                std::uint32_t indices[]{
                    0, 1, 2,
                    2, 3, 0
                };

                p_graphics.push_canvas_primitives(p_sprite.texture, vertices, indices);
            }
        });
    }
}
