#pragma once

#ifdef _WIN32
struct HWND__;
namespace rb {
    using window_handle = HWND__*;
}
#else
namespace rb {
    using window_handle = void*;
}
#endif
