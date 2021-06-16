#pragma once


#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <sysinfoapi.h>

static const char* CLASS_NAME = "windows";

inline uimax dword_lowhigh_to_uimax(const DWORD p_low, const DWORD p_high)
{
    ULARGE_INTEGER ul;
    ul.LowPart = p_low;
    ul.HighPart = p_high;
    return ul.QuadPart;
};

inline uint64 FILETIME_to_mics(FILETIME& p_filetime)
{
    return dword_lowhigh_to_uimax(p_filetime.dwLowDateTime, p_filetime.dwHighDateTime) / 10;
};

// Clock - BEGIN

time_t clock_native_currenttime_mics()
{
    FILETIME l_currentTime;
    GetSystemTimeAsFileTime(&l_currentTime);
    return FILETIME_to_mics(l_currentTime);
};

// Clock - END

// Mutex - BEGIN

mutex_native mutex_native_allocate(){
    mutex_native l_mutex;
    l_mutex.ptr = CreateMutex(NULL, 0, NULL);
#if __DEBUG
    assert_true(l_mutex.ptr != NULL);
#endif
    return l_mutex;
};
void mutex_native_lock(const mutex_native p_mutex){
    DWORD l_wait = WaitForSingleObject(p_mutex.ptr, INFINITE);
#if __DEBUG
    assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
};

void mutex_native_unlock(const mutex_native p_mutex){
    BOOL l_rm = ReleaseMutex(p_mutex.ptr);
#if __DEBUG
    assert_true(l_rm);
#endif
};

void mutex_native_free(mutex_native& p_mutex){
    BOOL l_ch = CloseHandle(p_mutex.ptr);
#if __DEBUG
    assert_true(l_ch);
#endif
    p_mutex.ptr = 0;
};

// Mutex - END

// Thread - BEGIN

thread_native thread_native_spawn_thread(void* p_function, void* p_parameter)
{
    thread_native l_thread;
    DWORD l_id;
    l_thread.ptr = (int8*)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)p_function, (LPVOID)p_parameter, 0, &l_id);

#if __DEBUG
    assert_true(l_thread.ptr != NULL);
#endif
    return l_thread;
};

thread_native thread_native_get_current_thread()
{
    thread_native l_thread;
    l_thread.ptr = (int8*)GetCurrentThread();
    return l_thread;
};

int8 thread_native_wait_for_end(const thread_native p_thread, const uimax p_time_in_ms)
{
#if __DEBUG
    assert_true(
#endif
        WaitForSingleObject(p_thread.ptr, (DWORD)p_time_in_ms)
        #if __DEBUG
        != WAIT_FAILED)
#endif
        ;

    DWORD l_exit_code;
    assert_true(GetExitCodeThread(p_thread.ptr, &l_exit_code));
    return (int8)l_exit_code;
};

void thread_native_sleep(const thread_native p_thread, const uimax p_time_in_ms){
    // TODO
};

void thread_native_terminate(const thread_native p_thread)
{
    TerminateThread(p_thread.ptr, 0);
};
void thread_native_kill(const thread_native p_thread)
{
    thread_native_terminate(p_thread);
};

// Thread - END

// Backtrace - BEGIN

void backtrace_capture(void* p_backtrace_array, const uimax p_backtrace_array_size)
{
    CaptureStackBackTrace(0, (DWORD)p_backtrace_array_size, (PVOID*)p_backtrace_array, NULL);
};

// Backtrace - END

// File - BEGIN

file_native file_native_create_file_unchecked(const Slice<int8>& p_path)
{
    file_native l_file;
    l_file.ptr = (uimax)CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    return l_file;
};

file_native file_native_open_file_unchecked(const Slice<int8>& p_path)
{
    file_native l_file;
    l_file.ptr = (uimax)CreateFile(p_path.Begin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return l_file;
};

void file_native_close_file_unchecked(file_native p_file_handle)
{
#if __DEBUG
    assert_true(
#endif
        CloseHandle((HANDLE)p_file_handle.ptr)
#if __DEBUG
    )
#endif
        ;
};

void file_native_delete_file_unchecked(const Slice<int8>& p_path)
{
#if __DEBUG
    assert_true(
#endif
        DeleteFile(p_path.Begin)
#if __DEBUG
    )
#endif
        ;
};

void file_native_set_file_pointer(file_native p_file_handle, uimax p_pointer)
{
#if __DEBUG
    assert_true(
#endif
        SetFilePointer((HANDLE)p_file_handle.ptr, (LONG)p_pointer, 0, FILE_BEGIN)
        #if __DEBUG
        != INVALID_SET_FILE_POINTER)
#endif
        ;
};

uimax file_native_get_file_size(file_native p_file_handle)
{
    DWORD l_file_size_high;
    DWORD l_return = GetFileSize((HANDLE)p_file_handle.ptr, &l_file_size_high);
#if __DEBUG
    assert_true(l_return != INVALID_FILE_SIZE);
#endif
    return dword_lowhigh_to_uimax(l_return, l_file_size_high);
};

uimax file_native_get_modification_ts(file_native p_file_handle)
{
    FILETIME l_creation_time, l_last_access_time, l_last_write_time;
    BOOL l_filetime_return = GetFileTime((HANDLE)p_file_handle.ptr, &l_creation_time, &l_last_access_time, &l_last_write_time);
#if __DEBUG
    assert_true(l_filetime_return);
#endif
    uimax l_ct = FILETIME_to_mics(l_creation_time);
    uimax l_at = FILETIME_to_mics(l_last_access_time);
    uimax l_wt = FILETIME_to_mics(l_last_write_time);

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
        ReadFile((HANDLE)p_file_handle.ptr, in_out_buffer->Begin, (DWORD)in_out_buffer->Size, NULL, NULL)
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
        WriteFile((HANDLE)p_file_handle.ptr, p_buffer.Begin, (DWORD)p_buffer.Size, NULL, NULL)
#if __DEBUG
    )
#endif
        ;
};

int8 file_native_handle_is_valid(file_native p_file_handle)
{
    return (void*)p_file_handle.ptr != INVALID_HANDLE_VALUE;
};

// File - END

// SharedLib - BEGIN

shared_lib shared_lib_load(const Slice<int8>& p_path)
{
    shared_lib l_shared_lib;
    l_shared_lib.ptr = (int8*)LoadLibrary(p_path.Begin);
    return l_shared_lib;
};

shared_lib_proc_address shared_lib_get_procaddress(const shared_lib p_shader_lib, const Slice<int8>& p_proc_name)
{
    shared_lib_proc_address l_proc_address;
    l_proc_address.ptr = (int8*)GetProcAddress((HMODULE)p_shader_lib.ptr, p_proc_name.Begin);
    return l_proc_address;
};

// SharedLib - END

// EventLoop - BEGIN

inline LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
    {
        AppWindowCloseEvent l_windowEvent = AppWindowCloseEvent{AppEventType::WINDOW_CLOSE, window_native{(int8*)hwnd}};
        AppEvent_Broadcast(window_native{(int8*)hwnd}, &l_windowEvent.type);
        return 0;
    }
    case WM_SIZE:
    {
        uint32 l_width = LOWORD(lParam);
        uint32 l_height = HIWORD(lParam);

        AppWindowResizeEvent l_windowResizeEvent = AppWindowResizeEvent{AppEventType::WINDOW_RESIZE, window_native{(int8*)hwnd}, l_width, l_height};
        AppEvent_Broadcast(window_native{(int8*)hwnd}, &l_windowResizeEvent.type);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};

void app_native_loop_initialize()
{
    WNDCLASS wc = {NULL};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(GetModuleHandle(NULL), IDC_ARROW);

    RegisterClass(&wc);
};

void app_native_loop_finalize(){
    // Nothing for windows
};

void app_native_loop_pool_events()
{
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    };
};

// EventLoop - END

// Window - BEGIN

#if __DEBUG
#define window_handle_native_return(Code) assert_true(Code)
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

// Window - END


// Socket - BEGIN


#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

void socket_context_startup()
{
    WSADATA l_winsock_data;
    SocketNative::error_handler(WSAStartup(MAKEWORD(2, 2), &l_winsock_data));
};
void socket_context_cleanup()
{
    WSACleanup();
};

int32 socket_native_get_last_error()
{
    return WSAGetLastError();
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
    return ((SOCKET)this->ptr) != INVALID_SOCKET;
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
    assert_true((SOCKET)l_socket.ptr != INVALID_SOCKET);
#endif

    return l_socket;
};

void socket_native_set_address_reusable(socket_native p_socket)
{
    int32 l_reuse_addres = 1;
    socket_native_error_handler(setsockopt((SOCKET)p_socket.ptr, SOL_SOCKET, SO_REUSEADDR, (const int8*)&l_reuse_addres, sizeof(l_reuse_addres)));
};

void socket_native_set_non_blocking(socket_native p_socket)
{
    uint32 l_mode = 1; // non-blocking
    socket_native_error_handler(ioctlsocket((SOCKET)p_socket.ptr, FIONBIO, (u_long*)&l_mode));
};

void socket_native_bind(socket_native p_socket, socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    socket_native_error_handler(bind((SOCKET)p_socket.ptr, l_addr_info->ai_addr, (int32)l_addr_info->ai_addrlen));
};

void socket_native_listen(socket_native p_socket)
{
    socket_native_error_handler(listen((SOCKET)p_socket.ptr, SOMAXCONN));
};

int8 socket_native_connect(socket_native p_socket, socket_addrinfo p_addrinfo)
{
    addrinfo* l_addr_info = (addrinfo*)p_addrinfo.ptr;
    return connect((SOCKET)p_socket.ptr, l_addr_info->ai_addr, (int32)l_addr_info->ai_addrlen) != 0;
};

socket_native socket_native_accept(socket_native p_socket)
{
    return socket_native{(int8*)accept((SOCKET)p_socket.ptr, NULL, NULL)};
};

int32 socket_native_send(socket_native p_socket, const Slice<int8>& p_memory)
{
    return send((SOCKET)p_socket.ptr, p_memory.Begin, (int32)p_memory.Size, 0);
};

int32 socket_native_recv(socket_native p_socket, const Slice<int8>& p_memory)
{
    return recv((SOCKET)p_socket.ptr, p_memory.Begin, (int32)p_memory.Size, 0);
};

void socket_native_close(socket_native p_socket)
{
    closesocket((SOCKET)p_socket.ptr);
};

int8 socket_native_is_error_is_blocking_type(const int32 p_error)
{
    return p_error == WSAEWOULDBLOCK;
};

int8 socket_native_is_error_is_msgsize_type(const int32 p_error)
{
    return p_error == WSAEMSGSIZE;
};



// Socket - END