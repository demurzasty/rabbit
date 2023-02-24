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

            irect source;
            source.position.x = (sprite.frame % sprite.hframes) * frame_size.x;
            source.position.y = (sprite.frame / sprite.hframes) * frame_size.y;
            source.size.x = frame_size.x;
            source.size.y = frame_size.y;

            rect destination;
            destination.position.x = transform.position.x - sprite.offset.x * transform.scale.x;
            destination.position.y = transform.position.y - sprite.offset.y * transform.scale.y;
            destination.size.x = float(frame_size.x) * transform.scale.x;
            destination.size.y = float(frame_size.y) * transform.scale.y;

            m_painter.draw(*sprite.texture, source, destination, sprite.color, sprite.offset * transform.scale, transform.rotation);
        }
    });
}
