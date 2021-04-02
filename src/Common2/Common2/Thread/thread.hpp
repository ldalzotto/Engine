#pragma once

using thread_t
#ifdef _WIN32
    = HANDLE
#elif __linux__
    = pthread_t
#endif // _WIN32
    ;

typedef int8 (*Thread_MainFunction)(const Slice<int8*>* p_args);

struct ThreadInput
{
    Thread_MainFunction function;
    Slice<int8*> args;
};

thread_t Thread_spawn_thread(ThreadInput* p_thread_main);
thread_t Thread_get_current_thread();
void Thread_wait(const thread_t p_thread, const uimax p_time_in_ms);
void Thread_wait_for_end_and_terminate(const thread_t p_thread, const uimax p_max_time_in_ms);
void Thread_kill(const thread_t p_thread);

#ifdef _WIN32

DWORD WINAPI Thread_native_main_function(LPVOID lpParam)
{
    ThreadInput* l_input = (ThreadInput*)lpParam;
    return l_input->function(&l_input->args);
};

inline thread_t Thread_spawn_thread(ThreadInput& p_thread_main)
{
    DWORD l_id;
    thread_t l_thread = (thread_t)CreateThread(NULL, 0, Thread_native_main_function, (LPVOID)&p_thread_main, 0, &l_id);

#if __DEBUG
    assert_true(l_thread != NULL);
#endif

#if __MEMLEAK
    push_ptr_to_tracked((int8*)l_thread);
#endif

    return l_thread;
};

inline thread_t Thread_get_current_thread()
{
    return GetCurrentThread();
};

inline void Thread_wait(const thread_t p_thread, const uimax p_time_in_ms)
{
#if __DEBUG
    assert_true(
#endif
        WaitForSingleObject(p_thread, (DWORD)p_time_in_ms)
#if __DEBUG
        != WAIT_FAILED)
#endif
        ;
};

inline void Thread_wait_for_end_and_terminate(const thread_t p_thread, const uimax p_max_time_in_ms)
{
    Thread_wait(p_thread, p_max_time_in_ms);

#if __MEMLEAK
    remove_ptr_to_tracked((int8*)p_thread);
#endif

#if __DEBUG
    DWORD l_exit_code;
    assert_true(GetExitCodeThread(p_thread, &l_exit_code));
    TerminateThread(p_thread, 0);
    assert_true(l_exit_code == 0);
#endif

    TerminateThread(p_thread, 0);
};

inline void Thread_kill(const thread_t p_thread)
{
#if __MEMLEAK
    remove_ptr_to_tracked((int8*)p_thread);
#endif
    TerminateThread(p_thread, 0);
};

#elif __linux__

inline void Thread::wait(const uimax p_time_in_ms)
{
    usleep(p_time_in_ms * 1000);
};

#endif