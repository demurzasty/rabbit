#include <rabbit/platform/window_manager.hpp>

#if RB_WINDOWS
#   include "win32/window_win32.hpp"
using window_impl = rb::window_win32;
#endif

using namespace rb;

// TODO: Add new management options.

std::shared_ptr<window> window_manager::create(const window_desc& desc) {
    return _windows.emplace_back(std::make_shared<window_impl>(desc));
}
