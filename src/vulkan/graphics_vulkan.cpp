#include <rabbit/graphics.hpp>

using namespace rb;

struct graphics::impl {

};

graphics::graphics(const window& p_window)
    : m_impl(std::make_shared<impl>()) {

}

graphics::~graphics() {
}

void graphics::present() {
}
