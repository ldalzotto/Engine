#pragma once


struct EWindow
{
    window_native handle;
    uint32 client_width;
    uint32 client_height;

    int8 is_closing;

    struct ResizeEvent
    {
        int8 ask;
        uint32 new_frame_width;
        uint32 new_frame_height;

        static ResizeEvent build_default();
    } resize_event;

    static EWindow build(const window_native p_handle, const uint32 p_client_width, const uint32 p_client_height);

    void close();

    int8 asks_for_resize() const;
    void on_resized(const uint32 p_frame_width, const uint32 p_frame_height);
    void consume_resize_event();
};

GLOBAL_STATIC Pool<EWindow> g_app_windows;

struct NativeWindow_to_Window
{
    window_native native_window;
    Pool<EWindow>::sToken window;
};

GLOBAL_STATIC Vector<NativeWindow_to_Window> g_native_to_window;

struct WindowAllocator
{
    static void initialize();
    static void finalize();

    static Pool<EWindow>::sToken allocate(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_name);
    static void free(const Pool<EWindow>::sToken p_window);
    static void free_headless(const Pool<EWindow>::sToken p_window);

    static EWindow& get_window(const Pool<EWindow>::sToken p_window);

    static void on_appevent(window_native p_window_handle, AppEventType* p_appevent);
};

inline EWindow::ResizeEvent EWindow::ResizeEvent::build_default()
{
    return ResizeEvent{0, 0, 0};
};

inline EWindow EWindow::build(const window_native p_handle, const uint32 p_client_width, const uint32 p_client_height)
{
    return EWindow{p_handle, p_client_width, p_client_height, 0, ResizeEvent::build_default()};
};

inline void EWindow::close()
{
    this->is_closing = 1;
};

inline int8 EWindow::asks_for_resize() const
{
    return this->resize_event.ask;
};

inline void EWindow::on_resized(const uint32 p_frame_width, const uint32 p_frame_height)
{
    this->resize_event.ask = 1;
    this->resize_event.new_frame_width = p_frame_width;
    this->resize_event.new_frame_height = p_frame_height;
};

inline void EWindow::consume_resize_event()
{
    this->resize_event.ask = 0;

    window_native_get_window_client_dimensions(this->handle, &this->client_width, &this->client_height);
};

inline void WindowAllocator::initialize()
{
    g_app_windows = Pool<EWindow>::allocate(0);
    g_native_to_window = Vector<NativeWindow_to_Window>::allocate(0);
};
inline void WindowAllocator::finalize()
{
#if __DEBUG
    assert_true(!g_app_windows.has_allocated_elements());
    assert_true(g_native_to_window.empty());
#endif
    g_app_windows.free();
    g_native_to_window.free();
};

inline Pool<EWindow>::sToken WindowAllocator::allocate(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_name)
{
    if (g_appevent_listeners.Size == 0)
    {
        AppNativeEvent::initialize();
    }
    window_native l_window = window_native_create_window(p_client_width, p_client_height, p_name);
    window_native_display_window(l_window);
    AppNativeEvent::add_appevent_listener(AppEventQueueListener{l_window, WindowAllocator::on_appevent});

    if (g_native_to_window.empty())
    {
        WindowAllocator::initialize();
    }

    uint32 l_native_client_width, l_native_client_height;
    window_native_get_window_client_dimensions(l_window, &l_native_client_width, &l_native_client_height);
    Pool<EWindow>::sToken l_allocated_window = g_app_windows.alloc_element(EWindow::build(l_window, l_native_client_width, l_native_client_height));
    g_native_to_window.push_back_element(NativeWindow_to_Window{l_window, l_allocated_window});

    return l_allocated_window;
};

inline void WindowAllocator::free(const Pool<EWindow>::sToken p_window)
{
    free_headless(p_window);

    for (loop(i, 0, g_native_to_window.Size))
    {
        if (token_equals(g_native_to_window.get(i).window, p_window))
        {
            window_native_destroy_window(g_native_to_window.get(i).native_window);
            // AppNativeEvent::
            AppNativeEvent::remove_appevent_listener(g_native_to_window.get(i).native_window);
            g_native_to_window.erase_element_at_always(i);
        }
    }

    if (g_native_to_window.empty())
    {
        WindowAllocator::finalize();
        AppNativeEvent::finalize();
    };
};

inline void WindowAllocator::free_headless(const Pool<EWindow>::sToken p_window)
{
    g_app_windows.release_element(p_window);

    if (!g_app_windows.has_allocated_elements())
    {
        g_app_windows.free();
    };
};

inline EWindow& WindowAllocator::get_window(const Pool<EWindow>::sToken p_window)
{
    return g_app_windows.get(p_window);
};

inline void WindowAllocator::on_appevent(window_native p_window_handle, AppEventType* p_appevent)
{
    for (loop(i, 0, g_native_to_window.Size))
    {
        if (g_native_to_window.get(i).native_window.ptr == p_window_handle.ptr)
        {
            switch ((*p_appevent))
            {
            case AppEventType::WINDOW_CLOSE:
            {
                g_app_windows.get(g_native_to_window.get(i).window).close();
            }
            break;
            case AppEventType::WINDOW_RESIZE:
            {
                AppWindowResizeEvent* l_resize_event = (AppWindowResizeEvent*)p_appevent;
                g_app_windows.get(g_native_to_window.get(i).window).on_resized(l_resize_event->width, l_resize_event->height);
            }
            break;
            }
        }
    }
};

#undef window_handle_native_return