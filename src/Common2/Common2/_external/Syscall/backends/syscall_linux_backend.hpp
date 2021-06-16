#pragma once

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xmd.h>

template <class ParameterType> inline ParameterType xlib_status_handle(const ParameterType p_function_return)
{
#if __DEBUG
    if (p_function_return == 0)
    {
        abort();
    }
#endif
    return p_function_return;
};

template <class ParameterType> inline ParameterType xlib_error_handle(const ParameterType p_function_return)
{
#if __DEBUG
    if (p_function_return != 0)
    {
        abort();
    }
#endif
    return p_function_return;
};

// Clock - BEGIN

time_t clock_native_currenttime_mics()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return round(spec.tv_nsec / 1000);
};

inline timespec miliseconds_to_timespec(const uimax p_miliseconds)
{
    timespec l_ts;
    l_ts.tv_sec = p_miliseconds / 1000;
    l_ts.tv_nsec = (p_miliseconds % 1000) * 1000000;
    return l_ts;
};

// Clock - END

// Mutex - BEGIN

mutex_native mutex_native_allocate()
{
    mutex_native l_mutex;
    int l_return = pthread_mutex_init((pthread_mutex_t*)&l_mutex, NULL);
#if __DEBUG
    assert_true(l_return == 0);
#endif
    return l_mutex;
};
void mutex_native_lock(const mutex_native& p_mutex)
{
    timespec l_wait = miliseconds_to_timespec(-1);
    int l_return = pthread_mutex_timedlock((pthread_mutex_t*)&p_mutex, &l_wait);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

void mutex_native_unlock(const mutex_native& p_mutex)
{
    int l_return = pthread_mutex_unlock((pthread_mutex_t*)&p_mutex);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

void mutex_native_free(mutex_native& p_mutex)
{
    int l_return = pthread_mutex_destroy((pthread_mutex_t*)&p_mutex);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

// Mutex - END

// Thread - BEGIN

thread_native thread_native_spawn_thread(void* p_function, void* p_parameter)
{
    using _thread_func = void* (*)(void*);
    thread_native l_thread;
    pthread_create((pthread_t*)&l_thread.ptr, NULL, (_thread_func)p_function, p_parameter);

#if __DEBUG
    assert_true(l_thread.ptr != 0);
#endif

    return l_thread;
};

thread_native thread_native_get_current_thread()
{
    thread_native l_thread;
    l_thread.ptr = (int8*)pthread_self();
    return l_thread;
};

int8 thread_native_wait_for_end(const thread_native p_thread, const uimax p_time_in_ms)
{
    timespec l_time = miliseconds_to_timespec(p_time_in_ms);
    int8* l_return;
    if (pthread_timedjoin_np((pthread_t)p_thread.ptr, (void**)&l_return, &l_time) == 0)
    {
        int8 l_return_local = *l_return;
        heap_free((int8*)l_return);
        return l_return_local;
    }
    return 1;
};

void thread_native_sleep(const thread_native p_thread, const uimax p_time_in_ms)
{
    usleep(p_time_in_ms * 1000);
};

void thread_native_terminate(const thread_native p_thread){
    // /!\ For linux, termination is ensured by the join operation when waiting for end
};
void thread_native_kill(const thread_native p_thread)
{
    int l_return = pthread_cancel((pthread_t)p_thread.ptr);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

void thread_native_on_main_function_finished(const int8 p_return)
{
    Span<int8> l_return_heap = Span<int8>::allocate_slice(Slice<int8>::build_memory_elementnb((int8*)&p_return, 1));
    pthread_exit(l_return_heap.Memory);
};

    // Thread - END

    // Backtrace - BEGIN

#include <execinfo.h>

void backtrace_capture(void* p_backtrace_array, const uimax p_backtrace_array_size)
{
    backtrace((void**)p_backtrace_array, p_backtrace_array_size);
};

    // Backtrace - END

    // File - BEGIN

#include <fcntl.h>
#include <sys/stat.h>

file_native file_native_create_file_unchecked(const Slice<int8>& p_path)
{
    file_native l_file;
    l_file.ptr = (uimax)open(p_path.Begin, O_RDWR | O_CREAT, S_IRWXU);
    return l_file;
};

file_native file_native_open_file_unchecked(const Slice<int8>& p_path)
{
    file_native l_file;
    l_file.ptr = (uimax)open(p_path.Begin, O_RDWR | O_CREAT, S_IRWXU);
    return l_file;
};

void file_native_close_file_unchecked(file_native p_file_handle)
{
#if __DEBUG
    assert_true(
#endif
        close((int)p_file_handle.ptr)
#if __DEBUG
        == 0)
#endif
        ;
};

void file_native_delete_file_unchecked(const Slice<int8>& p_path)
{
#if __DEBUG
    assert_true(
#endif
        remove(p_path.Begin)
#if __DEBUG
        == 0)
#endif
        ;
};

void file_native_set_file_pointer(file_native p_file_handle, uimax p_pointer)
{
#if __DEBUG
    assert_true(
#endif
        lseek(p_file_handle.ptr, p_pointer, SEEK_SET)
#if __DEBUG
        == 0)
#endif
        ;
};

uimax file_native_get_file_size(file_native p_file_handle)
{
    uint32 l_old_seek = lseek(p_file_handle.ptr, 0, SEEK_CUR);
    uint32 l_size = lseek(p_file_handle.ptr, 0, SEEK_END);
    lseek(p_file_handle.ptr, l_old_seek, SEEK_SET);
#if __DEBUG
    assert_true(l_size != -1);
#endif
    return l_size;
};

uimax file_native_get_modification_ts(file_native p_file_handle)
{
    struct stat l_stat;
    int l_ftat_return = fstat(p_file_handle.ptr, &l_stat);
#if __DEBUG
    assert_true(l_ftat_return == 0);
#endif

    uimax l_ct = l_stat.st_atime;
    uimax l_at = l_stat.st_mtime;
    uimax l_wt = l_stat.st_ctime;

    uimax l_return = l_ct;
    if (l_at >= l_return)
    {
        l_return = l_at;
    }
    if (l_wt >= l_return)
    {
        l_return = l_wt;
    }
    return l_return;
};

void file_native_read_buffer(file_native p_file_handle, Slice<int8>* in_out_buffer)
{
#if __DEBUG
    assert_true(
#endif
        read(p_file_handle.ptr, in_out_buffer->Begin, in_out_buffer->Size)
#if __DEBUG
    )
#endif
        ;
};

void file_native_write_buffer(file_native p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer)
{
#if __DEBUG
    assert_true(
#endif
        write(p_file_handle.ptr, p_buffer.Begin, p_buffer.Size)
#if __DEBUG
    )
#endif
        ;
};

int8 file_native_handle_is_valid(file_native p_file_handle)
{
    return (int)(p_file_handle.ptr) != -1;
};

// File - END

// EventLoop - BEGIN

struct DisplayInfo
{
    Display* display;
    int32 screen;
} g_display_info;

inline void LinuxProc(const XEvent& p_event)
{
    switch (p_event.type)
    {
    case DestroyNotify:
    {
        XDestroyWindowEvent l_event = p_event.xdestroywindow;

        window_native l_window;
        l_window.ptr = (int8*)l_event.window;

        AppWindowCloseEvent l_windowEvent = AppWindowCloseEvent{AppEventType::WINDOW_CLOSE, l_window};
        AppEvent_Broadcast(l_window, &l_windowEvent.type);
        break;
    }
    case ConfigureNotify:
    {
        XConfigureEvent l_event = p_event.xconfigure;

        window_native l_window;
        l_window.ptr = (int8*)l_event.window;

        for (loop(i, 0, g_appevent_windowdimensions.Size))
        {
            AppWindowDimensionsBuffer& l_window_size_buffer = g_appevent_windowdimensions.get(i);
            if (l_window_size_buffer.window.ptr == l_window.ptr)
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

void app_native_loop_initialize()
{
    g_display_info.display = XOpenDisplay(NULL);

#if __DEBUG
    assert_true(g_display_info.display != NULL);
#endif
#if __MEMLEAK
    push_ptr_to_tracked((int8*)g_display_info.display);
#endif

    g_display_info.screen = DefaultScreen(g_display_info.display);
};

void app_native_loop_finalize()
{
#if __MEMLEAK
    remove_ptr_to_tracked((int8*)g_display_info.display);
#endif

    XCloseDisplay(g_display_info.display);
};

void app_native_loop_pool_events()
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

    // EventLoop - END

    // Window - BEGIN

#if __DEBUG
#define window_handle_native_return(Code) assert_true((Code) == 0)
#else
#define window_handle_native_return(Code) Code
#endif

window_native window_native_create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name)
{
    window_native l_window;
    l_window.ptr = (int8*)xlib_status_handle(XCreateSimpleWindow(g_display_info.display, RootWindow(g_display_info.display, g_display_info.screen), 10, 10, p_client_width, p_client_height, 1,
                                                                 BlackPixel(g_display_info.display, g_display_info.screen), WhitePixel(g_display_info.display, g_display_info.screen)));

    Atom del_window = xlib_status_handle(XInternAtom(g_display_info.display, "WM_DELETE_WINDOW", 0));

    xlib_status_handle(XSetWMProtocols(g_display_info.display, (Window)l_window.ptr, &del_window, 1));
    xlib_status_handle(XSelectInput(g_display_info.display, (Window)l_window.ptr, ExposureMask | StructureNotifyMask | KeyPressMask));
    xlib_status_handle(XMapWindow(g_display_info.display, (Window)l_window.ptr));

    return l_window;
};
void window_native_display_window(const window_native p_window)
{
    xlib_status_handle(XMapWindow(g_display_info.display, (Window)p_window.ptr));
};

void window_native_destroy_window(const window_native p_window)
{
    xlib_status_handle(XDestroyWindow(g_display_info.display, (Window)p_window.ptr));
};

void window_native_resize_window(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height)
{
    int32 l_return = xlib_status_handle(XResizeWindow(g_display_info.display, (Window)p_window.ptr, p_client_width, p_client_height));

#if __DEBUG
    assert_true(l_return != BadValue && l_return != BadWindow);
#endif
};

void window_native_get_window_client_dimensions(const window_native p_window_handle, uint32* out_client_width, uint32* out_client_height)
{
    Window l_root_window;
    int32 l_position_x, l_position_y;
    uint32 l_border_widht, l_depth;

    xlib_status_handle(XGetGeometry(g_display_info.display, (Window)p_window_handle.ptr, &l_root_window, &l_position_x, &l_position_y, out_client_width, out_client_height, &l_border_widht, &l_depth));
};

inline void _window_native_test_wait_for_resize_event(const window_native p_window, const int32 p_width, const int32 p_height){
    XEvent l_event;

    while (1)
    {
        XPending(g_display_info.display);
        xlib_error_handle(XNextEvent(g_display_info.display, &l_event));
        LinuxProc(l_event);

        if (l_event.type == ConfigureNotify)
        {
            XConfigureEvent l_configure_event = l_event.xconfigure;

            window_native l_window;
            l_window.ptr = (int8*)l_configure_event.window;

            if (l_window.ptr == p_window.ptr && l_configure_event.width == p_width && l_configure_event.height == p_height)
            {
                return;
            }
        }
    }
};

void window_native_simulate_resize_appevent(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height)
{
    window_native_resize_window(p_window, p_client_width, p_client_height);
    _window_native_test_wait_for_resize_event(p_window, p_client_width, p_client_height);
};
void window_native_simulate_close_appevent(const window_native p_window)
{
    XEvent l_close_event;
    l_close_event.type = DestroyNotify;
    l_close_event.xdestroywindow.display = g_display_info.display;
    l_close_event.xdestroywindow.window = (Window)p_window.ptr;
    LinuxProc(l_close_event);
};

    // Window - END

    // Socket - BEGIN

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

void socket_context_startup(){
    // Nothing for linux
};
void socket_context_cleanup(){
    // Nothing for linux
};

int32 socket_native_get_last_error()
{
    return errno;
};

template <class ReturnType> inline static ReturnType socket_native_error_handler(const ReturnType p_return)
{
#if __DEBUG
    if (p_return != 0)
    {
        abort();
    }
#endif
    return p_return;
};

int8 socket_native::is_valid()
{
    return ((uimax)this->ptr) != INVALID_SOCKET;
};

void socket_native::invalidate()
{
    this->ptr = (int8*)INVALID_SOCKET;
};

socket_addrinfo socket_addrinfo_get(const socket_native_type p_type, const int32 p_port)
{
    socket_addrinfo l_addr_info;

    SliceN<int8, ToString::int32str_size> p_port_str_raw;
    Slice<int8> l_port_str = slice_from_slicen(&p_port_str_raw);
    ToString::aint32(p_port, l_port_str);

    addrinfo l_typed_addr_info{};
    switch (p_type)
    {
    case socket_native_type::TCP_SERVER:
    {
        l_typed_addr_info.ai_family = AF_INET;
        l_typed_addr_info.ai_socktype = SOCK_STREAM;
        l_typed_addr_info.ai_protocol = IPPROTO_TCP;
        l_typed_addr_info.ai_flags = AI_PASSIVE;
    }
    break;
    case socket_native_type::TCP_CLIENT:
    {
        l_typed_addr_info.ai_family = AF_INET;
        l_typed_addr_info.ai_socktype = SOCK_STREAM;
        l_typed_addr_info.ai_protocol = IPPROTO_TCP;
    }
    break;
    }

    socket_native_error_handler(getaddrinfo(NULL, l_port_str.Begin, &l_typed_addr_info, (addrinfo**)&l_addr_info.ptr));

    return l_addr_info;
};

socket_addrinfo socket_addrinfo_next(socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    return socket_addrinfo{(int8*)l_addr_info};
};

void socket_addrinfo_free(socket_addrinfo p_addrinfo)
{
    freeaddrinfo((addrinfo*)p_addrinfo.ptr);
};

socket_native socket_native_allocate(socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;

    socket_native l_socket;
    l_socket.ptr = (int8*)socket(l_addr_info->ai_family, l_addr_info->ai_socktype, l_addr_info->ai_protocol);

#if __DEBUG
    assert_true((uimax)l_socket.ptr != INVALID_SOCKET);
#endif

    return l_socket;
};

void socket_native_set_address_reusable(socket_native p_socket)
{
    int32 l_reuse_addres = 1;
    socket_native_error_handler(setsockopt((uimax)p_socket.ptr, SOL_SOCKET, SO_REUSEADDR, (const int8*)&l_reuse_addres, sizeof(l_reuse_addres)));
};

void socket_native_set_non_blocking(socket_native p_socket)
{
    socket_native_error_handler(fcntl((uimax)p_socket.ptr, F_SETFL, O_NONBLOCK));
};

void socket_native_bind(socket_native p_socket, socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    socket_native_error_handler(bind((uimax)p_socket.ptr, l_addr_info->ai_addr, (int32)l_addr_info->ai_addrlen));
};

void socket_native_listen(socket_native p_socket)
{
    socket_native_error_handler(listen((uimax)p_socket.ptr, SOMAXCONN));
};

int8 socket_native_connect(socket_native p_socket, socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    return connect((uimax)p_socket.ptr, l_addr_info->ai_addr, (int32)l_addr_info->ai_addrlen) != 0;
};

socket_native socket_native_accept(socket_native p_socket)
{
    return socket_native{(int8*)accept((uimax)p_socket.ptr, NULL, NULL)};
};

int32 socket_native_send(socket_native p_socket, const Slice<int8>& p_memory)
{
    return send((uimax)p_socket.ptr, p_memory.Begin, (int32)p_memory.Size, 0);
};

int32 socket_native_recv(socket_native p_socket, const Slice<int8>& p_memory)
{
    return recv((uimax)p_socket.ptr, p_memory.Begin, (int32)p_memory.Size, 0);
};

void socket_native_close(socket_native p_socket)
{
    close((uimax)p_socket.ptr);
};

int8 socket_native_is_error_is_blocking_type(const int32 p_error)
{
    return (p_error == EWOULDBLOCK || p_error == EAGAIN);
};

int8 socket_native_is_error_is_msgsize_type(const int32 p_error)
{
    // for linux, such error doesn't exists, so we always return 0 indicating that it is not a msgsize type error.
    return 0;
};
