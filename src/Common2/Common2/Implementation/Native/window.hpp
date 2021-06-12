#pragma once

#if __DEBUG
#if _WIN32
#define window_handle_native_return(Code) assert_true(Code)
#else
#define window_handle_native_return(Code) assert_true((Code) == 0)
#endif
#else
#define window_handle_native_return(Code) Code
#endif

window_native window_native_create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name)
{
    DWORD l_windowStyle = WS_OVERLAPPEDWINDOW;

    RECT l_clientRect = {0};
    l_clientRect.right = p_client_width;
    l_clientRect.bottom = p_client_height;
    AdjustWindowRect(&l_clientRect, l_windowStyle, false);

    int l_windowWidth = l_clientRect.right - l_clientRect.left;
    int l_windowHeight = l_clientRect.bottom - l_clientRect.top;

    HWND hwnd = CreateWindowEx(0, (LPCSTR)CLASS_NAME, p_display_name.Begin, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, l_windowWidth, l_windowHeight, NULL, NULL, GetModuleHandle(NULL), NULL);
#if __DEBUG
    assert_true(hwnd != NULL);
#endif

    window_native l_window;
    l_window.ptr = (int8*)hwnd;
    return l_window;
};
void window_native_display_window(const window_native p_window)
{
    ShowWindow((HWND)p_window.ptr, SW_SHOW);
};

void window_native_destroy_window(const window_native p_window)
{
    window_handle_native_return(DestroyWindow((HWND)p_window.ptr));
};

void window_native_resize_window(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height)
{
    RECT l_rect;
    GetWindowRect((HWND)p_window.ptr, &l_rect);

    DWORD l_windowStyle = WS_OVERLAPPEDWINDOW;
    RECT l_clientRect = {0};
    l_clientRect.right = p_client_width;
    l_clientRect.bottom = p_client_height;
    AdjustWindowRect(&l_clientRect, l_windowStyle, false);

    int l_windowWidth = l_clientRect.right - l_clientRect.left;
    int l_windowHeight = l_clientRect.bottom - l_clientRect.top;

    window_handle_native_return(MoveWindow((HWND)p_window.ptr, l_rect.left, l_rect.top, l_windowWidth, l_windowHeight, 0));
};

void window_native_get_window_client_dimensions(const window_native p_window_handle, uint32* out_client_width, uint32* out_client_height)
{
    RECT l_rect;
    window_handle_native_return(GetClientRect((HWND)p_window_handle.ptr, &l_rect));
    *out_client_width = l_rect.right - l_rect.left;
    *out_client_height = l_rect.bottom - l_rect.top;
};
void window_native_simulate_resize_appevent(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height)
{
    LPARAM l_new_size = MAKELPARAM(p_client_width, p_client_height);
    window_native_resize_window(p_window, p_client_width, p_client_height);
};
void window_native_simulate_close_appevent(const window_native p_window)
{
    // WindowNative::destroy_window(p_window);
    WindowProc((HWND)p_window.ptr, WM_CLOSE, 0, 0);
};
