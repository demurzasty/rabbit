#include <rabbit/loaders/texture_loader.hpp>
#include <rabbit/core/config.hpp>
#include <rabbit/graphics/texture.hpp>
#include <rabbit/graphics/graphics_device.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace rb;

texture_loader::texture_loader(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
}

std::shared_ptr<void> texture_loader::load(const std::string& filename, const json& metadata) {
    int width, height, components;
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels{
        stbi_load(filename.c_str(), &width, &height, &components, STBI_rgb_alpha),
        &stbi_image_free
    };

    RB_ASSERT(pixels, "Cannot load image");

    texture_desc desc;
    desc.data = pixels.get();
    desc.size = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 0 };
    desc.format = texture_format::rgba8;
    desc.filter = texture_filter::anisotropic;
    desc.wrap = texture_wrap::repeat;
    desc.mipmaps = 0;
    return _graphics_device.create_texture(desc);
}
