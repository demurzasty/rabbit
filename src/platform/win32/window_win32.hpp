#pragma once 

#include <rabbit/platform/window.hpp>

#include <Windows.h>

namespace rb {
    struct window::data {
        HWND hwnd = nullptr;
        bool open = true;
        bool fullscreen = false;
    };
}
