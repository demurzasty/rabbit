#include <rabbit/window.hpp>

#include <Windows.h>

using namespace rb;

static LRESULT CALLBACK window_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);

struct window::impl {
    HWND hwnd = nullptr;
    bool open = true;
};

static int s_window_count = 0;

window::window()
    : m_impl(std::make_shared<impl>()) {

    if (s_window_count++ == 0) {
        WNDCLASS wc{};
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = &window_proc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "RabBit";
        RegisterClass(&wc);
    }

    HDC screen_dc = GetDC(NULL);
    int window_width = 1280;
    int window_height = 720;

    int screen_width = GetDeviceCaps(screen_dc, HORZRES);
    int screen_height = GetDeviceCaps(screen_dc, VERTRES);

    int left = (screen_width - window_width) / 2;
    int top = (screen_height - window_height) / 2;
    ReleaseDC(NULL, screen_dc);

    RECT rect{ 0, 0, window_width, window_height };

    const DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE;

    int width = window_width;
    int height = window_height;

    if (AdjustWindowRect(&rect, style, FALSE)) {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    m_impl->hwnd = CreateWindow("RabBit", "RabBit", style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), &m_dispatcher);
}

window::~window() {
    DestroyWindow(m_impl->hwnd);

    if (--s_window_count == 0) {
        UnregisterClass("RabBit", GetModuleHandle(NULL));
    }
}

bool window::dispatch_events() {
    MSG message;
    while (PeekMessage(&message, m_impl->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    m_dispatcher.update();
    return false;
}

void* window::handle() const {
    return m_impl->hwnd;
}

LRESULT CALLBACK window_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam) {
    if (p_msg == WM_CREATE) {
        SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)(p_lparam))->lpCreateParams);
    }

    dispatcher* dispatcher = p_hwnd ? reinterpret_cast<rb::dispatcher*>(GetWindowLongPtr(p_hwnd, GWLP_USERDATA)) : nullptr;
    if (dispatcher) {
        switch (p_msg) {
            case WM_CLOSE:
                dispatcher->enqueue<window::close_event>();
                break;
        }
    }

    if (p_msg == WM_CLOSE) {
        return 0;
    }

    if ((p_msg == WM_SYSCOMMAND) && (p_wparam == SC_KEYMENU)) {
        return 0;
    }

    if (!p_hwnd) {
        return 0;
    }

    return DefWindowProc(p_hwnd, p_msg, p_wparam, p_lparam);
}
