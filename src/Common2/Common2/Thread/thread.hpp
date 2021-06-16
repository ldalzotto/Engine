#pragma once

struct Thread
{

    template <class ThreadCtx> struct s_main
    {
#if _WIN32
        inline static int8 __stdcall main(void* lpParam)
        {
            ThreadCtx* thiz = (ThreadCtx*)lpParam;
            return thiz->operator()();
        };
#else
        inline static void* main(void* lpParam)
        {
            ThreadCtx* thiz = (ThreadCtx*)lpParam;
            int8 l_return = thiz->operator()();
            thread_native_on_main_function_finished(l_return);
        };
#endif
    };


    template <class ThreadCtx> inline static thread_native spawn_thread(ThreadCtx& p_thread_ctx)
    {
        thread_native l_thread = thread_native_spawn_thread((void*)&s_main<ThreadCtx>::main, (void*)&p_thread_ctx);
#if __MEMLEAK
        push_ptr_to_tracked(l_thread.ptr);
#endif
        return l_thread;
    };

    inline static thread_native get_current_thread()
    {
        return thread_native_get_current_thread();
    };

    inline static int8 wait_for_end(const thread_native p_thread, const uimax p_time_in_ms)
    {
        return thread_native_wait_for_end(p_thread, p_time_in_ms);
    };

    inline static int8 sleep(const thread_native p_thread, const uimax p_time_in_ms)
    {
        thread_native_sleep(p_thread, p_time_in_ms);
    };

    inline static void wait_for_end_and_terminate(const thread_native p_thread, const uimax p_max_time_in_ms)
    {
        int8 l_exit_code = Thread::wait_for_end(p_thread, p_max_time_in_ms);
        assert_true(l_exit_code == 0);
#if __MEMLEAK
        remove_ptr_to_tracked(p_thread.ptr);
#endif
        thread_native_terminate(p_thread);
    };

    inline static void kill(const thread_native p_thread)
    {
#if __MEMLEAK
        remove_ptr_to_tracked(p_thread.ptr);
#endif
        thread_native_kill(p_thread);
    };
};