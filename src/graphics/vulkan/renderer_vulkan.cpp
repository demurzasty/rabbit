#include "renderer_vulkan.hpp"
#include "utils_vulkan.hpp"

using namespace rb;

renderer::renderer(window& window)
    : m_window(window), m_data(std::make_unique<data>()) {
    vku::setup(m_data, window);
}

renderer::~renderer() {
    vku::quit(m_data);
}

handle renderer::create_texture() {
    return null;
}

void renderer::destroy_texture(handle id) {
}

void renderer::set_texture_data(handle id, const uvec2& size, pixel_format format, const void* pixels) {
}

uvec2 renderer::get_texture_size(handle id) const {
    return { 0, 0 };
}

pixel_format renderer::get_texture_format(handle id) const {
    return pixel_format::undefined;
}

void renderer::display() {
}
