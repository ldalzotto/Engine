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
}; // namespace AppNativeEvent

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
#if CONTAINER_BOUND_TEST
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

inline void AppNativeEvent::add_appevent_listener(const AppEventQueueListener& p_listener)
{
    g_appevent_listeners.push_back_element(p_listener);
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
};

#endif
