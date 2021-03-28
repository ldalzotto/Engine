#pragma once

#if __DEBUG
#define window_handle_native_return(Code) assert_true(Code)
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

inline void WindowNative::simulate_close_appevent(const WindowHandle p_window){
    // WindowNative::destroy_window(p_window);
    WindowProc((HWND)p_window, WM_CLOSE, 0, 0);
};

#endif

struct Window
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

    static Window build(const WindowHandle p_handle, const uint32 p_client_width, const uint32 p_client_height);

    void close();

    int8 asks_for_resize() const;
    void on_resized(const uint32 p_frame_width, const uint32 p_frame_height);
    void consume_resize_event();
};

Pool<Window> g_app_windows;

struct NativeWindow_to_Window
{
    WindowHandle native_window;
    Token(Window) window;
};

Vector<NativeWindow_to_Window> g_native_to_window;

struct WindowAllocator
{
    static void initialize();
    static void finalize();

    static Token(Window) allocate(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_name);
    static void free(const Token(Window) p_window);
    static void free_headless(const Token(Window) p_window);

    static Window& get_window(const Token(Window) p_window);

    static void on_appevent(WindowHandle p_window_handle, AppEventType* p_appevent);
};

inline Window::ResizeEvent Window::ResizeEvent::build_default()
{
    return ResizeEvent{0, 0, 0};
};

inline Window Window::build(const WindowHandle p_handle, const uint32 p_client_width, const uint32 p_client_height)
{
    return Window{p_handle, p_client_width, p_client_height, 0, ResizeEvent::build_default()};
};

inline void Window::close()
{
    this->is_closing = 1;
};

inline int8 Window::asks_for_resize() const
{
    return this->resize_event.ask;
};

inline void Window::on_resized(const uint32 p_frame_width, const uint32 p_frame_height)
{
    this->resize_event.ask = 1;
    this->resize_event.new_frame_width = p_frame_width;
    this->resize_event.new_frame_height = p_frame_height;
};

inline void Window::consume_resize_event()
{
    this->resize_event.ask = 0;

    WindowNative::get_window_client_dimensions(this->handle, &this->client_width, &this->client_height);
};

inline void WindowAllocator::initialize()
{
    g_app_windows = Pool<Window>::allocate(0);
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

inline Token(Window) WindowAllocator::allocate(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_name)
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
    Token(Window) l_allocated_window = g_app_windows.alloc_element(Window::build(l_window, l_native_client_width, l_native_client_height));
    g_native_to_window.push_back_element(NativeWindow_to_Window{l_window, l_allocated_window});

    return l_allocated_window;
};

inline void WindowAllocator::free(const Token(Window) p_window)
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

inline void WindowAllocator::free_headless(const Token(Window) p_window)
{
    g_app_windows.release_element(p_window);

    if (!g_app_windows.has_allocated_elements())
    {
        g_app_windows.free();
    };
};

inline Window& WindowAllocator::get_window(const Token(Window) p_window)
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