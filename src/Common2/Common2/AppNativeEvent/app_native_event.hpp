#pragma once

#ifndef APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS
#define APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS 0
#endif

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

Vector<AppEventQueueListener> g_appevent_listeners;

#if APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS
// to trigger resize events
struct AppWindowDimensionsBuffer
{
    window_native window;
    // We store last window dimensions to trigger resize events.
    uint32 last_client_width;
    uint32 last_client_height;
};

Vector<AppWindowDimensionsBuffer> g_appevent_windowdimensions;
#endif

namespace AppNativeEvent
{
inline void initialize()
{
    app_native_loop_initialize();
    g_appevent_listeners = Vector<AppEventQueueListener>::allocate(0);
#if APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS
    g_appevent_windowdimensions = Vector<AppWindowDimensionsBuffer>::allocate(0);
#endif
};
inline void finalize()
{
#if __DEBUG
    assert_true(g_appevent_listeners.empty());
#endif
    g_appevent_listeners.free();

#if APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS
#if __DEBUG
    assert_true(g_appevent_windowdimensions.empty());
#endif
    g_appevent_windowdimensions.free();
#endif

    app_native_loop_finalize();
};
inline void poll_events()
{
    app_native_loop_pool_events();
};

inline void add_appevent_listener(const AppEventQueueListener& p_listener)
{
    g_appevent_listeners.push_back_element(p_listener);

#if APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS
    window_native l_root_window = p_listener.window;
    uint32 l_width, l_height;
    window_native_get_window_client_dimensions(l_root_window, &l_width, &l_height);
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

#if APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS
    for (loop(i, 0, g_appevent_windowdimensions.Size))
    {
        if (g_appevent_windowdimensions.get(i).window.ptr == p_window.ptr)
        {
            g_appevent_windowdimensions.erase_element_at_always(i);
            break;
        }
    }
#endif
};

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
