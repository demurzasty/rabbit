#include <rabbit/loaders/texture_loader.hpp>
#include <rabbit/core/config.hpp>
#include <rabbit/graphics/texture.hpp>
#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/core/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <fstream>

using namespace rb;

namespace {
    std::array<const char*, 6> faces = {
        "right",
        "left",
        "top",
        "bottom",
        "front",
        "back"
    };
}

texture_loader::texture_loader(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
}

std::shared_ptr<void> texture_loader::load(const std::string& filename, const json& metadata) {
    const auto file_extension = filename.substr(filename.find_last_of("."));

    texture_desc desc;
    if (file_extension == ".json") {
        // Try to open file.
        std::fstream stream{ filename, std::ios::in };
        RB_ASSERT(stream.is_open(), "Cannot open file");

        // Load json from file.
        json json;
        stream >> json;

        // We do not need open stream anymore.
        stream.close();

        // Get individual faces texture filenames.
        std::array<std::string, 6> filenames;
        for (std::size_t index{ 0 }; index < 6; ++index) {
            filenames[index] = json[faces[index]];
        }

        std::map<std::size_t, stbi_uc*> data;
        for (std::size_t index{ 0 }; index < 6; ++index) {
            int width, height, components;
            data[index] = stbi_load(filenames[index].c_str(), &width, &height, &components, STBI_rgb_alpha);
            desc.size = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 6 };
        }

        auto buffer = std::make_unique<stbi_uc[]>(desc.size.x * desc.size.y * 4 * 6);
        for (auto& [index, pixels] : data) {
            std::memcpy(buffer.get() + index * (desc.size.x * desc.size.y * 4), pixels, desc.size.x * desc.size.y * 4);
            stbi_image_free(pixels);
        }

        desc.data = buffer.get();
        desc.format = texture_format::rgba8;
        desc.filter = texture_filter::anisotropic;
        desc.wrap = texture_wrap::repeat;
        desc.mipmaps = 1;
        desc.layers = 6;
        desc.type = texture_type::texture_cube;
        return _graphics_device.create_texture(desc);
    } else {
        int width, height, components;
        std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels{
            stbi_load(filename.c_str(), &width, &height, &components, STBI_rgb_alpha),
            &stbi_image_free
        };

        RB_ASSERT(pixels, "Cannot load image");

        desc.data = pixels.get();
        desc.size = { static_cast<unsigned int>(width), static_cast<unsigned int>(height), 1 };
        desc.format = texture_format::rgba8;
        desc.filter = texture_filter::anisotropic;
        desc.wrap = texture_wrap::repeat;
        desc.mipmaps = 0;
        desc.type = texture_type::texture_2d;
        return _graphics_device.create_texture(desc);
    }
}
