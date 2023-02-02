#include <rabbit/core/reference.hpp>

using namespace rb;

void reference::retain() {
    ++m_count;
}

void reference::release() {
    if (--m_count == 0) {
        delete this;
    }
}
