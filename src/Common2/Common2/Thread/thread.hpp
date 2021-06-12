#pragma once

#if _WIN32


#else
template <class ThreadCtx> inline thread_t ThreadNative::spawn_thread(ThreadCtx& p_thread_ctx)
{
    thread_t l_thread = 0;
    pthread_create(&l_thread, NULL, s_main<ThreadCtx>::main, (void*)&p_thread_ctx);

#if __DEBUG
    assert_true(l_thread != 0);
#endif
    return l_thread;
};

inline thread_t ThreadNative::get_current_thread()
{
    return pthread_self();
};

inline int8 ThreadNative::wait_for_end(const thread_t p_thread, const uimax p_time_in_ms)
{
    timespec l_time = miliseconds_to_timespec(p_time_in_ms);
    int8* l_return;
    if (pthread_timedjoin_np(p_thread, (void**)&l_return, &l_time) == 0)
    {
        int8 l_return_local = *l_return;
        heap_free((int8*)l_return);
        return l_return_local;
    }
    return 1;
};

inline void ThreadNative::sleep(const thread_t p_thread, const uimax p_time_in_ms)
{
    usleep(p_time_in_ms * 1000);
};

inline void ThreadNative::terminate(const thread_t p_thread){
    // /!\ For linux, termination is ensured by the join operation when waiting for end
};

inline void ThreadNative::kill(const thread_t p_thread)
{
    int l_return = pthread_cancel(p_thread);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

#endif

struct Thread
{

    template <class ThreadCtx> struct s_main
    {
#if _WIN32
        inline static DWORD WINAPI main(LPVOID lpParam)
        {
            ThreadCtx* thiz = (ThreadCtx*)lpParam;
            return thiz->operator()();
        };
#else
        inline static void* main(void* lpParam)
        {
            ThreadCtx* thiz = (ThreadCtx*)lpParam;
            int8 l_return = thiz->operator()();
            Span<int8> l_return_heap = Span<int8>::allocate_slice(Slice<int8>::build_memory_elementnb(&l_return, 1));
            pthread_exit(l_return_heap.Memory);
        };
#endif
    };


    template <class ThreadCtx> inline static thread_native spawn_thread(ThreadCtx& p_thread_ctx)
    {
        thread_native l_thread = thread_native_spawn_thread(&s_main<ThreadCtx>::main, &p_thread_ctx);
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