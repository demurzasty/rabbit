#include <rabbit/loaders/font_loader.hpp>
#include <rabbit/graphics/image.hpp>

#include <vector>
#include <fstream>

using namespace rb;

font_loader::font_loader(renderer& renderer)
    : m_renderer(renderer) {
}

font font_loader::operator()(std::string_view path) const {
    std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer((std::size_t)size);
    file.read((char*)buffer.data(), size);

    return { m_renderer, buffer };
}
