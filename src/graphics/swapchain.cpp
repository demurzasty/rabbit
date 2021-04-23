#include <rabbit/graphics/swapchain.hpp>

using namespace rb;

swapchain::swapchain(const swapchain_desc& desc)
    : _size(desc.size) {
}

const vec2u& swapchain::size() const noexcept {
    return _size;
}
