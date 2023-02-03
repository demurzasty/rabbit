#include <rabbit/graphics/rect_pack.hpp>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

using namespace rb;

struct rect_pack::data {
    stbrp_context context = {};
    std::unique_ptr<stbrp_node[]> nodes;
};

rect_pack::rect_pack(const uvec2& size)
    : m_data(new data()) {
    m_data->nodes = std::make_unique<stbrp_node[]>(size.x);
    
    stbrp_init_target(&m_data->context, int(size.x), int(size.y), m_data->nodes.get(), int(size.x));
}

rect_pack::rect_pack(rect_pack&& rect_pack) noexcept
    : m_data(std::move(rect_pack.m_data)) {
}

rect_pack::~rect_pack() {
    // For this specific implementation we do not need
    // this destructor at all. 
}

rect_pack& rect_pack::operator=(rect_pack&& rect_pack) noexcept {
    m_data = std::move(rect_pack.m_data);
    return *this;
}

ivec4 rect_pack::pack(const uvec2& size) {
    stbrp_rect rect;
    rect.w = int(size.x + 2);
    rect.h = int(size.y + 2);
    stbrp_pack_rects(&m_data->context, &rect, 1);

    return { rect.x + 1, rect.y + 1, int(size.x), int(size.y) };
}
