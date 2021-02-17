#pragma once

using thread_t
#ifdef _WIN32
= HANDLE
#elif __linux__
= pthread_t
#endif // _WIN32
;


struct Thread
{
    static thread_t get_current_thread();
    static void wait(const uimax p_time_in_ms);
};

#ifdef _WIN32

inline thread_t Thread::get_current_thread()
{
    return GetCurrentThread();
};

inline void Thread::wait(const uimax p_time_in_ms)
{
    WaitForSingleObject(get_current_thread(), (DWORD)p_time_in_ms);
};

#elif __linux__

inline void Thread::wait(const uimax p_time_in_ms)
{
    usleep(p_time_in_ms * 1000);
};


#endif