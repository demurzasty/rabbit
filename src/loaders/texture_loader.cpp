#include <rabbit/loaders/texture_loader.hpp>
#include <rabbit/graphics/image.hpp>

using namespace rb;

texture_loader::texture_loader(renderer& renderer)
    : m_renderer(renderer) {
}

texture texture_loader::operator()(std::string_view path) const {
    image image = image::from(path, true);

    texture texture(m_renderer, image.size(), texture_filter::nearest, pixel_format::rgba8);
    texture.update(image.pixels().data());
    return texture;
}
