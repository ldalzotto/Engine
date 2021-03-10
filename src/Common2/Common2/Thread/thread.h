#pragma once

typedef
#ifdef _WIN32
    HANDLE
#elif
    pthread_t
#endif
        thread_t;

static thread_t Thread_get_current_thread();
static void Thread_wait(const uimax p_time_in_ms);

#ifdef _WIN32

inline thread_t Thread_get_current_thread()
{
    return GetCurrentThread();
};

inline void Thread_wait(const uimax p_time_in_ms)
{
    WaitForSingleObject(Thread_get_current_thread(), (DWORD)p_time_in_ms);
};

#elif __linux__

inline void Thread::wait(const uimax p_time_in_ms)
{
    usleep(p_time_in_ms * 1000);
};

#endif