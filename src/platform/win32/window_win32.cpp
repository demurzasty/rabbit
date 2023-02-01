#include "window_win32.hpp"

using namespace rb;

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static int window_count = 0;

window::window(std::string_view title, const uvec2& size, bool fullscreen)
    : m_data(std::make_unique<window::data>()) {
    m_data->fullscreen = fullscreen;

    if (window_count++ == 0) {
        WNDCLASS wc{};
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = &window_proc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "RabBit";
        RegisterClass(&wc);
    }

    HDC screen_dc = GetDC(NULL);
    const auto window_width = size.x;
    const auto window_height = size.y;

    const auto screen_width = GetDeviceCaps(screen_dc, HORZRES);
    const auto screen_height = GetDeviceCaps(screen_dc, VERTRES);

    auto left = (screen_width - window_width) / 2;
    auto top = (screen_height - window_height) / 2;
    ReleaseDC(NULL, screen_dc);

    RECT rect{ 0, 0, LONG(window_width), LONG(window_height) };

    const DWORD style = fullscreen ?
        WS_VISIBLE | WS_POPUP :
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE;

    auto width = window_width;
    auto height = window_height;

    if (fullscreen) {
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

    m_data->hwnd = CreateWindow("RabBit", std::string(title).c_str(), style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), &m_dispatcher);
    assert(m_data->hwnd);
}

window::~window() {
    if (m_data->fullscreen) {
        ChangeDisplaySettings(nullptr, 0);
    }

    DestroyWindow(m_data->hwnd);

    if (--window_count == 0) {
        UnregisterClass("RabBit", GetModuleHandle(NULL));
    }
}

void window::close() {
    m_data->open = false;
}

bool window::is_open() const {
    return m_data->open;
}

void window::dispatch_events() {
    MSG message;
    while (PeekMessage(&message, m_data->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    m_dispatcher.update();
}

void window::wait_dispatch_events() {
    MSG message;
    if (GetMessage(&message, m_data->hwnd, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    m_dispatcher.update();
}

void window::set_title(std::string_view title) {
    SetWindowText(m_data->hwnd, std::string(title).c_str());
}

ivec2 window::position() const {
    RECT rect;
    GetWindowRect(m_data->hwnd, &rect);

    return { rect.left, rect.top };
}

uvec2 window::size() const {
    RECT rect;
    GetClientRect(m_data->hwnd, &rect);

    return { (unsigned int)(rect.right - rect.left), (unsigned int)(rect.bottom - rect.top) };
}

void* window::handle() const {
    return m_data->hwnd;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_CREATE) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)(lparam))->lpCreateParams);
    }

    dispatcher* dispatcher = hwnd ? reinterpret_cast<rb::dispatcher*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) : nullptr;
    if (dispatcher) {
        switch (msg) {
            case WM_CLOSE:
                dispatcher->enqueue<close_event>();
                break;
            case WM_MOUSEMOVE: {
                mouse_move_event event;
                event.position.x = LOWORD(lparam);
                event.position.y = HIWORD(lparam);
                dispatcher->enqueue(event);
                break;
            }
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP: {
                mouse_button_event event;
                event.position.x = LOWORD(lparam);
                event.position.y = HIWORD(lparam);
                event.button = mouse_button::left;
                event.pressed = msg == WM_LBUTTONDOWN;
                dispatcher->enqueue(event);
                break;
            }
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP: {
                mouse_button_event event;
                event.position.x = LOWORD(lparam);
                event.position.y = HIWORD(lparam);
                event.button = mouse_button::right;
                event.pressed = msg == WM_RBUTTONDOWN;
                dispatcher->enqueue(event);
                break;
            }
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP: {
                mouse_button_event event;
                event.position.x = LOWORD(lparam);
                event.position.y = HIWORD(lparam);
                event.button = mouse_button::middle;
                event.pressed = msg == WM_MBUTTONDOWN;
                dispatcher->enqueue(event);
                break;
            }
        }
    }

    if (msg == WM_CLOSE) {
        return 0;
    }

    if ((msg == WM_SYSCOMMAND) && (wparam == SC_KEYMENU)) {
        return 0;
    }

    if (!hwnd) {
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}