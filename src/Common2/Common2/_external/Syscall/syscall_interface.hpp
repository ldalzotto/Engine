#pragma once

#if _WIN32

#define APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS 0
#define __mutex_native_v2_size sizeof(char*)

#elif __linux__

#define APP_NATIVE_EVENT_STORE_WINDOW_DIMENSIONS 1
#define __mutex_native_v2_size sizeof(pthread_mutex_t)

#endif

// Forward declaration

template <class ElementType> struct Slice;

// Clock - BEGIN

time_t clock_native_currenttime_mics();

// Clock - END

// Mutex - BEGIN

struct mutex_native
{
    union
    {
        int8 memory[__mutex_native_v2_size];
        int8* ptr;
    };
};

mutex_native mutex_native_allocate();
void mutex_native_lock(const mutex_native& p_mutex);
void mutex_native_unlock(const mutex_native& p_mutex);
void mutex_native_free(mutex_native& p_mutex);

// Mutex - END

// Thread - BEGIN

struct thread_native
{
    int8* ptr;
};

thread_native thread_native_spawn_thread(void* p_function, void* p_parameter);
thread_native thread_native_get_current_thread();
int8 thread_native_wait_for_end(const thread_native p_thread, const uimax p_time_in_ms);
void thread_native_sleep(const thread_native p_thread, const uimax p_time_in_ms);
void thread_native_terminate(const thread_native p_thread);
void thread_native_kill(const thread_native p_thread);
void thread_native_on_main_function_finished(const int8 p_return);

// Thread - END

// Backtrace - BEGIN

void backtrace_capture(void* p_backtrace_array, const uimax p_backtrace_array_size);

// Backtrace - END

// File - BEGIN

struct file_native
{
    uimax ptr;
};

file_native file_native_create_file_unchecked(const Slice<int8>& p_path);
file_native file_native_open_file_unchecked(const Slice<int8>& p_path);
void file_native_close_file_unchecked(file_native p_file_handle);
void file_native_delete_file_unchecked(const Slice<int8>& p_path);
void file_native_set_file_pointer(file_native p_file_handle, uimax p_pointer);
uimax file_native_get_file_size(file_native p_file_handle);
uimax file_native_get_modification_ts(file_native p_file_handle);
void file_native_read_buffer(file_native p_file_handle, Slice<int8>* in_out_buffer);
void file_native_write_buffer(file_native p_file_handle, const uimax p_offset, const Slice<int8>& p_buffer);
int8 file_native_handle_is_valid(file_native p_file_handle);

// File - END

// SharedLib - BEGIN

struct shared_lib
{
    int8* ptr;
};
struct shared_lib_proc_address
{
    int8* ptr;
};

shared_lib shared_lib_load(const Slice<int8>& p_path);
shared_lib_proc_address shared_lib_get_procaddress(const shared_lib p_shader_lib, const Slice<int8>& p_proc_name);

// SharedLib - END

// EventLoop - BEGIN

void app_native_loop_initialize();
void app_native_loop_finalize();
void app_native_loop_pool_events();

// EventLoop - END

// Window - BEGIN

struct window_native
{
    int8* ptr;
};

window_native window_native_create_window(const uint32 p_client_width, const uint32 p_client_height, const Slice<int8>& p_display_name);
void window_native_display_window(const window_native p_window);
void window_native_destroy_window(const window_native p_window);
void window_native_resize_window(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height);
void window_native_get_window_client_dimensions(const window_native p_window_handle, uint32* out_client_width, uint32* out_client_height);
void window_native_simulate_resize_appevent(const window_native p_window, const uint32 p_client_width, const uint32 p_client_height);
void window_native_simulate_close_appevent(const window_native p_window);

// Window - END

// Socket - BEGIN

enum class socket_native_type : int8
{
    TCP_SERVER = 0,
    TCP_CLIENT = 1
};

struct socket_native
{
    int8* ptr;

    int8 is_valid();
    void invalidate();
};

struct socket_addrinfo
{
    int8* ptr;

    int8 is_null()
    {
        return this->ptr == NULL;
    };
};

void socket_context_startup();
void socket_context_cleanup();

int32 socket_native_get_last_error();

socket_addrinfo socket_addrinfo_get(const socket_native_type p_type, const int32 p_port);
socket_addrinfo socket_addrinfo_next(socket_addrinfo p_addrinfo);
void socket_addrinfo_free(socket_addrinfo p_addrinfo);

socket_native socket_native_allocate(socket_addrinfo p_addrinfo);
void socket_native_set_address_reusable(socket_native p_socket);
void socket_native_set_non_blocking(socket_native p_socket);
void socket_native_bind(socket_native p_socket, socket_addrinfo p_addrinfo);
void socket_native_listen(socket_native p_socket);
int8 socket_native_connect(socket_native p_socket, socket_addrinfo p_addrinfo);
socket_native socket_native_accept(socket_native p_socket);

int32 socket_native_send(socket_native p_socket, const Slice<int8>& p_memory);
int32 socket_native_recv(socket_native p_socket, const Slice<int8>& p_memory);

void socket_native_close(socket_native p_socket);
// If the socket returns an error that tells that the call should have blocked
int8 socket_native_is_error_is_blocking_type(const int32 p_error);
// If the socket returns an error that tells that there is still data to receive
int8 socket_native_is_error_is_msgsize_type(const int32 p_error);

// Socket - END