#include <rabbit/systems/canvas.hpp>
#include <rabbit/components/transform.hpp>
#include <rabbit/components/sprite.hpp>

using namespace rb;

canvas::canvas(painter& painter)
    : m_painter(painter) {
}

void canvas::process(registry& registry, float time_step) {
    registry.view<transform, sprite>().each([this, time_step](transform& transform, sprite& sprite) {
        if (sprite.texture) {
            uvec2 texture_size = sprite.texture->size();

            uvec2 frame_size = { texture_size.x / sprite.hframes, texture_size.y / sprite.vframes };

            ivec4 source;
            source.x = (sprite.frame % sprite.hframes) * frame_size.x;
            source.y = (sprite.frame / sprite.hframes) * frame_size.y;
            source.z = frame_size.x;
            source.w = frame_size.y;

            vec4 destination;
            destination.x = transform.position.x - sprite.offset.x;
            destination.y = transform.position.y - sprite.offset.y;
            destination.z = float(frame_size.x);
            destination.w = float(frame_size.y);

            m_painter.draw(*sprite.texture, source, destination, sprite.color);
        }
    });
}
