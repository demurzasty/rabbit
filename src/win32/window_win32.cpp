#include <rabbit/window.hpp>

#include <Windows.h>

using namespace rb;

static LRESULT CALLBACK window_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);

struct window::impl {
    HWND hwnd = nullptr;
    bool open = true;
    bool fullscreen = false;
};

static int s_window_count = 0;

window::window(const std::string& p_title, int p_width, int p_height, bool p_fullscreen)
    : m_impl(std::make_shared<impl>()) {
    m_impl->fullscreen = p_fullscreen;

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
    int window_width = p_width;
    int window_height = p_height;

    int screen_width = GetDeviceCaps(screen_dc, HORZRES);
    int screen_height = GetDeviceCaps(screen_dc, VERTRES);

    int left = (screen_width - window_width) / 2;
    int top = (screen_height - window_height) / 2;
    ReleaseDC(NULL, screen_dc);

    RECT rect{ 0, 0, window_width, window_height };

    const DWORD style = p_fullscreen ?
        WS_VISIBLE | WS_POPUP : 
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE;

    int width = window_width;
    int height = window_height;

    if (p_fullscreen) {
        left = 0;
        top = 0;
        width = screen_width;
        height = screen_height;
    } else {
        if (AdjustWindowRect(&rect, style, FALSE)) {
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
        }
    }

    m_impl->hwnd = CreateWindow("RabBit", p_title.c_str(), style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), &m_dispatcher);
}

window::~window() {
    if (m_impl->fullscreen) {
        ChangeDisplaySettings(nullptr, 0);
    }

    DestroyWindow(m_impl->hwnd);

    if (--s_window_count == 0) {
        UnregisterClass("RabBit", GetModuleHandle(NULL));
    }
}

void window::dispatch_events() {
    MSG message;
    while (PeekMessage(&message, m_impl->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    m_dispatcher.update();
}

void* window::handle() const {
    return m_impl->hwnd;
}

int window::width() const {
    RECT rect;
    GetClientRect(m_impl->hwnd, &rect);
    return rect.right - rect.left;
}

int window::height() const {
    RECT rect;
    GetClientRect(m_impl->hwnd, &rect);
    return rect.bottom - rect.top;
}

LRESULT CALLBACK window_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam) {
    if (p_msg == WM_CREATE) {
        SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)(p_lparam))->lpCreateParams);
    }

    dispatcher* dispatcher = p_hwnd ? reinterpret_cast<rb::dispatcher*>(GetWindowLongPtr(p_hwnd, GWLP_USERDATA)) : nullptr;
    if (dispatcher) {
        switch (p_msg) {
            case WM_CLOSE:
                dispatcher->enqueue<close_event>();
                break;
            case WM_MOUSEMOVE: {
                mouse_move_event event;
                event.position.x = float(LOWORD(p_lparam));
                event.position.y = float(HIWORD(p_lparam));
                dispatcher->enqueue(event);
                break;
            }
            case WM_LBUTTONDOWN: {
                mouse_button_event event;
                event.position.x = float(LOWORD(p_lparam));
                event.position.y = float(HIWORD(p_lparam));
                event.button = mouse_button::left;
                event.pressed = true;
                dispatcher->enqueue(event);
                break;
            }
            case WM_LBUTTONUP: {
                mouse_button_event event;
                event.position.x = float(LOWORD(p_lparam));
                event.position.y = float(HIWORD(p_lparam));
                event.button = mouse_button::left;
                event.pressed = false;
                dispatcher->enqueue(event);
                break;
            }
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
