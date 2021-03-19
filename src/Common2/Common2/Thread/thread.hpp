#pragma once

using thread_t
#ifdef _WIN32
    = HANDLE
#elif __linux__
    = pthread_t
#endif // _WIN32
    ;

typedef int8 (*Thread_MainFunction)(const Slice<int8*>& p_args);

struct Thread
{
    struct MainInput
    {
        Thread_MainFunction function;
        Slice<int8*> args;
    };

    static thread_t spawn_thread(const MainInput& p_thread_main);
    static thread_t get_current_thread();
    static void wait(const thread_t p_thread, const uimax p_time_in_ms);
    static void wait_for_end_and_terminate(const thread_t p_thread, const uimax p_max_time_in_ms);
    static void kill(const thread_t p_thread);
};

#ifdef _WIN32

DWORD WINAPI Thread_native_main_function(LPVOID lpParam)
{
    Thread::MainInput* l_input = (Thread::MainInput*)lpParam;
    return l_input->function(l_input->args);
};

inline thread_t Thread::spawn_thread(const MainInput& p_thread_main)
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

inline thread_t Thread::get_current_thread()
{
    return GetCurrentThread();
};

inline void Thread::wait(const thread_t p_thread, const uimax p_time_in_ms)
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

inline void Thread::wait_for_end_and_terminate(const thread_t p_thread, const uimax p_max_time_in_ms)
{
    Thread::wait(p_thread, p_max_time_in_ms);
    DWORD l_exit_code;

#if __MEMLEAK
    remove_ptr_to_tracked((int8*)p_thread);
#endif

#if __DEBUG
    assert_true(
#endif
        GetExitCodeThread(p_thread, &l_exit_code)
#if __DEBUG
    )
#endif
        ;
    assert_true(l_exit_code != STILL_ACTIVE);
    TerminateThread(p_thread, 0);
};

inline void Thread::kill(const thread_t p_thread)
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