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

    template <class ThreadCtx> inline static thread_t spawn_thread(ThreadCtx& p_thread_ctx)
    {
        struct s_main
        {
            inline static DWORD WINAPI main(LPVOID lpParam)
            {
                ThreadCtx* thiz = (ThreadCtx*)lpParam;
                return thiz->operator()();
            };
        };

        DWORD l_id;
        thread_t l_thread = (thread_t)CreateThread(NULL, 0, s_main::main, (LPVOID)&p_thread_ctx, 0, &l_id);

#if __DEBUG
        assert_true(l_thread != NULL);
#endif

#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_thread);
#endif

        return l_thread;
    };

    inline static thread_t get_current_thread()
    {
        return GetCurrentThread();
    };

    inline static void wait(const thread_t p_thread, const uimax p_time_in_ms)
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

    inline static void wait_for_end_and_terminate(const thread_t p_thread, const uimax p_max_time_in_ms)
    {
        Thread::wait(p_thread, p_max_time_in_ms);

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

    inline static void kill(const thread_t p_thread)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)p_thread);
#endif
        TerminateThread(p_thread, 0);
    };
};