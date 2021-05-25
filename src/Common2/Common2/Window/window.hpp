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

struct WindowNative
{
    inline static WindowHandle create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name);
    inline static void display_window(const WindowHandle p_window_handle);
    inline static void destroy_window(const WindowHandle p_window_handle);
    inline static void resize_window(const WindowHandle p_window_handle, const uint32 p_client_width, const uint32 p_client_height);
    inline static void get_window_client_dimensions(const WindowHandle p_window_handle, uint32* out_client_width, uint32* out_client_height);
    inline static void simulate_resize_appevent(const WindowHandle p_window, const uint32 p_width, const uint32 p_height);
    inline static void simulate_close_appevent(const WindowHandle p_window);
};

#ifdef _WIN32

inline WindowHandle WindowNative::create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name)
{
    DWORD l_windowStyle = WS_OVERLAPPEDWINDOW;

    RECT l_clientRect = {0};
    l_clientRect.right = p_client_width;
    l_clientRect.bottom = p_client_height;
    AdjustWindowRect(&l_clientRect, l_windowStyle, false);

    int l_windowWidth = l_clientRect.right - l_clientRect.left;
    int l_windowHeight = l_clientRect.bottom - l_clientRect.top;

    HWND hwnd = CreateWindowEx(0, (LPCSTR)AppNativeEvent::CLASS_NAME, p_display_name.Begin, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, l_windowWidth, l_windowHeight, NULL, NULL,
                               GetModuleHandle(NULL), NULL);
#if __DEBUG
    assert_true(hwnd != NULL);
#endif

    return (WindowHandle)hwnd;
};

inline void WindowNative::display_window(const WindowHandle p_window_handle)
{
    ShowWindow((HWND)p_window_handle, SW_SHOW);
};

inline void WindowNative::destroy_window(const WindowHandle p_window_handle)
{
    window_handle_native_return(DestroyWindow((HWND)p_window_handle));
};

inline void WindowNative::resize_window(const WindowHandle p_window_handle, const uint32 p_client_width, const uint32 p_client_height)
{
    RECT l_rect;
    GetWindowRect((HWND)p_window_handle, &l_rect);

    DWORD l_windowStyle = WS_OVERLAPPEDWINDOW;
    RECT l_clientRect = {0};
    l_clientRect.right = p_client_width;
    l_clientRect.bottom = p_client_height;
    AdjustWindowRect(&l_clientRect, l_windowStyle, false);

    int l_windowWidth = l_clientRect.right - l_clientRect.left;
    int l_windowHeight = l_clientRect.bottom - l_clientRect.top;

    window_handle_native_return(MoveWindow((HWND)p_window_handle, l_rect.left, l_rect.top, l_windowWidth, l_windowHeight, 0));
};

inline void WindowNative::get_window_client_dimensions(const WindowHandle p_window_handle, uint32* out_client_width, uint32* out_client_height)
{
    RECT l_rect;
    window_handle_native_return(GetClientRect((HWND)p_window_handle, &l_rect));
    *out_client_width = l_rect.right - l_rect.left;
    *out_client_height = l_rect.bottom - l_rect.top;
};

inline void WindowNative::simulate_resize_appevent(const WindowHandle p_window, const uint32 p_client_width, const uint32 p_client_height)
{
    LPARAM l_new_size = MAKELPARAM(p_client_width, p_client_height);
    WindowNative::resize_window(p_window, p_client_width, p_client_height);
};

inline void WindowNative::simulate_close_appevent(const WindowHandle p_window)
{
    // WindowNative::destroy_window(p_window);
    WindowProc((HWND)p_window, WM_CLOSE, 0, 0);
};

#else

inline WindowHandle WindowNative::create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name)
{
    /* create window */
    Window l_window = xlib_status_handle(XCreateSimpleWindow(g_display_info.display, RootWindow(g_display_info.display, g_display_info.screen), 10, 10, p_client_width, p_client_height, 1,
                                                             BlackPixel(g_display_info.display, g_display_info.screen), WhitePixel(g_display_info.display, g_display_info.screen)));

    Atom del_window = xlib_status_handle(XInternAtom(g_display_info.display, "WM_DELETE_WINDOW", 0));

    xlib_status_handle(XSetWMProtocols(g_display_info.display, l_window, &del_window, 1));
    xlib_status_handle(XSelectInput(g_display_info.display, l_window, ExposureMask | StructureNotifyMask | KeyPressMask));
    xlib_status_handle(XMapWindow(g_display_info.display, l_window));

    return (WindowHandle)l_window;
};

inline void WindowNative::display_window(const WindowHandle p_window_handle)
{
    xlib_status_handle(XMapWindow(g_display_info.display, (Window)p_window_handle));
};

inline void WindowNative::destroy_window(const WindowHandle p_window_handle)
{
    xlib_status_handle(XDestroyWindow(g_display_info.display, (Window)p_window_handle));
};

inline void WindowNative::resize_window(const WindowHandle p_window_handle, const uint32 p_client_width, const uint32 p_client_height)
{
    int32 l_return = xlib_status_handle(XResizeWindow(g_display_info.display, (Window)p_window_handle, p_client_width, p_client_height));

#if __DEBUG
    assert_true(l_return != BadValue && l_return != BadWindow);
#endif
};

inline void WindowNative::get_window_client_dimensions(const WindowHandle p_window_handle, uint32* out_client_width, uint32* out_client_height)
{
    Window l_root_window;
    int32 l_position_x, l_position_y;
    uint32 l_border_widht, l_depth;

    xlib_status_handle(XGetGeometry(g_display_info.display, (Window)p_window_handle, &l_root_window, &l_position_x, &l_position_y, out_client_width, out_client_height, &l_border_widht, &l_depth));
};

inline void WindowNative::simulate_resize_appevent(const WindowHandle p_window, const uint32 p_client_width, const uint32 p_client_height)
{
    WindowNative::resize_window(p_window, p_client_width, p_client_height);
    AppNativeEvent::_test_wait_for_resize_event(p_window, p_client_width, p_client_height);
};

inline void WindowNative::simulate_close_appevent(const WindowHandle p_window)
{
    XEvent l_close_event;
    l_close_event.type = DestroyNotify;
    l_close_event.xdestroywindow.display = g_display_info.display;
    l_close_event.xdestroywindow.window = (Window)p_window;
    LinuxProc(l_close_event);
};

#endif

struct EWindow
{
    WindowHandle handle;
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

    static EWindow build(const WindowHandle p_handle, const uint32 p_client_width, const uint32 p_client_height);

    void close();

    int8 asks_for_resize() const;
    void on_resized(const uint32 p_frame_width, const uint32 p_frame_height);
    void consume_resize_event();
};

Pool<EWindow> g_app_windows;

struct NativeWindow_to_Window
{
    WindowHandle native_window;
    Token<EWindow> window;
};

Vector<NativeWindow_to_Window> g_native_to_window;

struct WindowAllocator
{
    static void initialize();
    static void finalize();

    static Token<EWindow> allocate(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_name);
    static void free(const Token<EWindow> p_window);
    static void free_headless(const Token<EWindow> p_window);

    static EWindow& get_window(const Token<EWindow> p_window);

    static void on_appevent(WindowHandle p_window_handle, AppEventType* p_appevent);
};

inline EWindow::ResizeEvent EWindow::ResizeEvent::build_default()
{
    return ResizeEvent{0, 0, 0};
};

inline EWindow EWindow::build(const WindowHandle p_handle, const uint32 p_client_width, const uint32 p_client_height)
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

    WindowNative::get_window_client_dimensions(this->handle, &this->client_width, &this->client_height);
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

inline Token<EWindow> WindowAllocator::allocate(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_name)
{
    if (g_appevent_listeners.Size == 0)
    {
        AppNativeEvent::initialize();
    }
    WindowHandle l_window = WindowNative::create_window(p_client_width, p_client_height, p_name);
    WindowNative::display_window(l_window);
    AppNativeEvent::add_appevent_listener(AppEventQueueListener{l_window, WindowAllocator::on_appevent});

    if (g_native_to_window.empty())
    {
        WindowAllocator::initialize();
    }

    uint32 l_native_client_width, l_native_client_height;
    WindowNative::get_window_client_dimensions(l_window, &l_native_client_width, &l_native_client_height);
    Token<EWindow> l_allocated_window = g_app_windows.alloc_element(EWindow::build(l_window, l_native_client_width, l_native_client_height));
    g_native_to_window.push_back_element(NativeWindow_to_Window{l_window, l_allocated_window});

    return l_allocated_window;
};

inline void WindowAllocator::free(const Token<EWindow> p_window)
{
    free_headless(p_window);

    for (loop(i, 0, g_native_to_window.Size))
    {
        if (token_equals(g_native_to_window.get(i).window, p_window))
        {
            WindowNative::destroy_window(g_native_to_window.get(i).native_window);
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

inline void WindowAllocator::free_headless(const Token<EWindow> p_window)
{
    g_app_windows.release_element(p_window);

    if (!g_app_windows.has_allocated_elements())
    {
        g_app_windows.free();
    };
};

inline EWindow& WindowAllocator::get_window(const Token<EWindow> p_window)
{
    return g_app_windows.get(p_window);
};

inline void WindowAllocator::on_appevent(WindowHandle p_window_handle, AppEventType* p_appevent)
{
    for (loop(i, 0, g_native_to_window.Size))
    {
        if (g_native_to_window.get(i).native_window == p_window_handle)
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