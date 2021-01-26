#pragma once

using thread_t
#ifdef _WIN32
= HANDLE
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

#endif