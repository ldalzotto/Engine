#pragma once

using WindowHandle = int8*;

enum class AppEventType
{
    WINDOW_CLOSE = 0,
    WINDOW_RESIZE = WINDOW_CLOSE + 1
};

struct AppWindowCloseEvent
{
    AppEventType type;
    WindowHandle window;
};

struct AppWindowResizeEvent
{
    AppEventType type;
    WindowHandle window;
    uint32 width;
    uint32 height;
};

struct AppEventQueueListener
{
    WindowHandle window;
    void (*func)(WindowHandle, AppEventType*);
};

namespace AppNativeEvent
{
const int8* CLASS_NAME = "window";
inline void initialize();
inline void finalize();
inline void poll_events();
inline void add_appevent_listener(const AppEventQueueListener& p_listener);
inline void remove_appevent_listener(const WindowHandle p_window);

inline void _test_wait_for_resize_event(const WindowHandle p_window, const int32 p_width, const int32 p_height);
}; // namespace AppNativeEvent

#if __linux__

struct DisplayInfo
{
    Display* display;
    int32 screen;
} g_display_info;

// to trigger resize events
struct AppWindowDimensionsBuffer
{
    WindowHandle window;
    // We store last window dimensions to trigger resize events. X11 events doesn't natively support resize like windows.
    uint32 last_client_width;
    uint32 last_client_height;
};

Vector<AppWindowDimensionsBuffer> g_appevent_windowdimensions;

#endif

Vector<AppEventQueueListener> g_appevent_listeners;

inline void AppEvent_Broadcast(WindowHandle p_window, AppEventType* p_app_event)
{
    for (loop(i, 0, g_appevent_listeners.Size))
    {
        if (g_appevent_listeners.get(i).window == p_window)
        {
            g_appevent_listeners.get(i).func(p_window, p_app_event);
        }
    }
};

inline void AppNativeEvent::add_appevent_listener(const AppEventQueueListener& p_listener)
{
    g_appevent_listeners.push_back_element(p_listener);

#if __linux__
    Window l_root_window;
    int32 l_x, l_y;
    uint32 l_width, l_height, l_border_width, l_depth;
    xlib_status_handle(XGetGeometry(g_display_info.display, (Window)p_listener.window, &l_root_window, &l_x, &l_y, &l_width, &l_height, &l_border_width, &l_depth));
    g_appevent_windowdimensions.push_back_element(AppWindowDimensionsBuffer{p_listener.window, l_width, l_height});
#endif
};

inline void AppNativeEvent::remove_appevent_listener(const WindowHandle p_window)
{
    for (loop(i, 0, g_appevent_listeners.Size))
    {
        if (g_appevent_listeners.get(i).window == p_window)
        {
            g_appevent_listeners.erase_element_at_always(i);
            break;
        }
    }

#if __linux__
    for (loop(i, 0, g_appevent_windowdimensions.Size))
    {
        if (g_appevent_windowdimensions.get(i).window == p_window)
        {
            g_appevent_windowdimensions.erase_element_at_always(i);
            break;
        }
    }
#endif
};

#ifdef _WIN32

inline LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
    case WM_CLOSE:
    {
        AppWindowCloseEvent l_windowEvent = AppWindowCloseEvent{AppEventType::WINDOW_CLOSE, (WindowHandle)hwnd};
        AppEvent_Broadcast((WindowHandle)hwnd, &l_windowEvent.type);
        return 0;
    }
    case WM_SIZE:
    {
        uint32 l_width = LOWORD(lParam);
        uint32 l_height = HIWORD(lParam);

        AppWindowResizeEvent l_windowResizeEvent = AppWindowResizeEvent{AppEventType::WINDOW_RESIZE, (WindowHandle)hwnd, l_width, l_height};
        AppEvent_Broadcast((WindowHandle)hwnd, &l_windowResizeEvent.type);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};

inline void AppNativeEvent::initialize()
{
    WNDCLASS wc = {NULL};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = AppNativeEvent::CLASS_NAME;
    wc.hCursor = LoadCursor(GetModuleHandle(NULL), IDC_ARROW);

    RegisterClass(&wc);

    g_appevent_listeners = Vector<AppEventQueueListener>::allocate(0);
};

inline void AppNativeEvent::finalize()
{
#if __DEBUG
    assert_true(g_appevent_listeners.empty());
#endif
    g_appevent_listeners.free();
};

inline void AppNativeEvent::poll_events()
{
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    };
};

#else

inline void LinuxProc(const XEvent& p_event)
{
    switch (p_event.type)
    {
    case DestroyNotify:
    {
        XDestroyWindowEvent l_event = p_event.xdestroywindow;
        AppWindowCloseEvent l_windowEvent = AppWindowCloseEvent{AppEventType::WINDOW_CLOSE, (WindowHandle)l_event.window};
        AppEvent_Broadcast((WindowHandle)l_event.window, &l_windowEvent.type);
        break;
    }
    case ConfigureNotify:
    {
        XConfigureEvent l_event = p_event.xconfigure;

        WindowHandle l_window = (WindowHandle)l_event.window;
        for (loop(i, 0, g_appevent_windowdimensions.Size))
        {
            AppWindowDimensionsBuffer& l_window_size_buffer = g_appevent_windowdimensions.get(i);
            if (l_window_size_buffer.window == l_window)
            {
                if (l_event.width != l_window_size_buffer.last_client_width || l_event.height != l_window_size_buffer.last_client_height)
                {
                    l_window_size_buffer.last_client_width = l_event.width;
                    l_window_size_buffer.last_client_height = l_event.height;

                    AppWindowResizeEvent l_windowResizeEvent = AppWindowResizeEvent{AppEventType::WINDOW_RESIZE, l_window, (uint32)l_event.width, (uint32)l_event.height};
                    AppEvent_Broadcast(l_window, &l_windowResizeEvent.type);
                };
                break;
            }
        }
        break;
    }
    }
};

inline void AppNativeEvent::initialize()
{
    g_display_info.display = XOpenDisplay(NULL);

#if __DEBUG
    assert_true(g_display_info.display != NULL);
#endif
#if __MEMLEAK
    push_ptr_to_tracked((int8*)g_display_info.display);
#endif

    g_display_info.screen = DefaultScreen(g_display_info.display);

    g_appevent_listeners = Vector<AppEventQueueListener>::allocate(0);
    g_appevent_windowdimensions = Vector<AppWindowDimensionsBuffer>::allocate(0);
};

inline void AppNativeEvent::finalize()
{
#if __DEBUG
    assert_true(g_appevent_listeners.empty());
    assert_true(g_appevent_windowdimensions.empty());
#endif
#if __MEMLEAK
    remove_ptr_to_tracked((int8*)g_display_info.display);
#endif

    g_appevent_listeners.free();
    g_appevent_windowdimensions.free();

    XCloseDisplay(g_display_info.display);
};

inline void AppNativeEvent::poll_events()
{
    XEvent l_event;

    while (1)
    {
        if (XPending(g_display_info.display) == 0)
        {
            return;
        }

        xlib_error_handle(XNextEvent(g_display_info.display, &l_event));
        LinuxProc(l_event);
    }
};

inline void AppNativeEvent::_test_wait_for_resize_event(const WindowHandle p_window, const int32 p_width, const int32 p_height)
{
    XEvent l_event;

    while (1)
    {
        XPending(g_display_info.display);
        xlib_error_handle(XNextEvent(g_display_info.display, &l_event));
        LinuxProc(l_event);

        if (l_event.type == ConfigureNotify)
        {
            XConfigureEvent l_configure_event = l_event.xconfigure;
            if ((WindowHandle)l_configure_event.window == p_window && l_configure_event.width == p_width && l_configure_event.height == p_height)
            {
                return;
            }
        }
    }
};

#endif
