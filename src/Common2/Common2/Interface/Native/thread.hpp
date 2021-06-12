#pragma once

struct thread_native{
    int8* ptr;
};

thread_native thread_native_spawn_thread(void* p_function, void* p_parameter);
thread_native thread_native_get_current_thread();
int8 thread_native_wait_for_end(const thread_native p_thread, const uimax p_time_in_ms);
void thread_native_sleep(const thread_native p_thread, const uimax p_time_in_ms);
void thread_native_terminate(const thread_native p_thread);
void thread_native_kill(const thread_native p_thread);
