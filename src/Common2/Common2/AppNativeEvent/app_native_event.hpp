#pragma once

enum class AppEventType
{
    WINDOW_CLOSE = 0,
    WINDOW_RESIZE = WINDOW_CLOSE + 1
};

struct AppWindowCloseEvent
{
    AppEventType type;
    window_native window;
};

struct AppWindowResizeEvent
{
    AppEventType type;
    window_native window;
    uint32 width;
    uint32 height;
};

struct AppEventQueueListener
{
    window_native window;
    void (*func)(window_native, AppEventType*);
};

GLOBAL_STATIC Vector<AppEventQueueListener> g_appevent_listeners;

namespace AppNativeEvent
{
inline void initialize()
{
    app_native_loop_initialize();
    g_appevent_listeners = Vector<AppEventQueueListener>::allocate(0);
};
inline void finalize()
{
#if __DEBUG
    assert_true(g_appevent_listeners.empty());
#endif
    g_appevent_listeners.free();

    app_native_loop_finalize();
};
inline void poll_events()
{
    app_native_loop_pool_events();
};

inline void add_appevent_listener(const AppEventQueueListener& p_listener)
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

inline void remove_appevent_listener(const window_native p_window)
{
    for (loop(i, 0, g_appevent_listeners.Size))
    {
        if (g_appevent_listeners.get(i).window.ptr == p_window.ptr)
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

inline void _test_wait_for_resize_event(const window_native p_window, const int32 p_width, const int32 p_height);
}; // namespace AppNativeEvent

inline void AppEvent_Broadcast(window_native p_window, AppEventType* p_app_event)
{
    for (loop(i, 0, g_appevent_listeners.Size))
    {
        if (g_appevent_listeners.get(i).window.ptr == p_window.ptr)
        {
            g_appevent_listeners.get(i).func(p_window, p_app_event);
        }
    }
};
