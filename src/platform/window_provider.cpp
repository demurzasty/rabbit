#include <rabbit/platform/window_provider.hpp>

#if RB_WINDOWS
#   include "win32/window_win32.hpp"
    using window_impl = rb::window_win32;
#endif

using namespace rb;

std::shared_ptr<window> window_provider::operator()(injector& injector) const {
    return injector.resolve<window_impl>();
}
