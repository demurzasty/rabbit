#include <rabbit/image.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace rb;

image image::from(std::string_view p_path) {
    int width, height, comp;
    stbi_uc* pixels = stbi_load(std::string(p_path).c_str(), &width, &height, &comp, 4);
    assert(pixels);
    return { pixels, width, height };
}
