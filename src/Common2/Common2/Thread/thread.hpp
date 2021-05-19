#pragma once

using thread_t
#ifdef _WIN32
    = HANDLE;
#elif __linux__
    = pthread_t;
#endif // _WIN32

struct ThreadNative
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

    template <class ThreadCtx> inline static thread_t spawn_thread(ThreadCtx& p_thread_ctx);
    inline static thread_t get_current_thread();
    inline static int8 wait(const thread_t p_thread, const uimax p_time_in_ms);
    inline static void terminate(const thread_t p_thread);
    inline static void kill(const thread_t p_thread);
};

#if _WIN32
template <class ThreadCtx> inline thread_t ThreadNative::spawn_thread(ThreadCtx& p_thread_ctx)
{
    DWORD l_id;
    thread_t l_thread = (thread_t)CreateThread(NULL, 0, s_main<ThreadCtx>::main, (LPVOID)&p_thread_ctx, 0, &l_id);

#if __DEBUG
    assert_true(l_thread != NULL);
#endif
    return l_thread;
};

inline thread_t ThreadNative::get_current_thread()
{
    return GetCurrentThread();
};

inline int8 ThreadNative::wait(const thread_t p_thread, const uimax p_time_in_ms)
{
#if __DEBUG
    assert_true(
#endif
        WaitForSingleObject(p_thread, (DWORD)p_time_in_ms)
#if __DEBUG
        != WAIT_FAILED)
#endif
        ;

    DWORD l_exit_code;
    assert_true(GetExitCodeThread(p_thread, &l_exit_code));
    return (int8)l_exit_code;
};

inline void ThreadNative::terminate(const thread_t p_thread)
{
    TerminateThread(p_thread, 0);
};

inline void ThreadNative::kill(const thread_t p_thread)
{
    ThreadNative::terminate(p_thread);
};

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

inline int8 ThreadNative::wait(const thread_t p_thread, const uimax p_time_in_ms)
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

    template <class ThreadCtx> inline static thread_t spawn_thread(ThreadCtx& p_thread_ctx)
    {
        thread_t l_thread = ThreadNative::spawn_thread(p_thread_ctx);
#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_thread);
#endif

        return l_thread;
    };

    inline static thread_t get_current_thread()
    {
        return ThreadNative::get_current_thread();
    };

    inline static int8 wait(const thread_t p_thread, const uimax p_time_in_ms)
    {
        return ThreadNative::wait(p_thread, p_time_in_ms);
    };

    inline static void wait_for_end_and_terminate(const thread_t p_thread, const uimax p_max_time_in_ms)
    {
        int8 l_exit_code = Thread::wait(p_thread, p_max_time_in_ms);
        assert_true(l_exit_code == 0);
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)p_thread);
#endif
        ThreadNative::terminate(p_thread);
    };

    inline static void kill(const thread_t p_thread)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)p_thread);
#endif
        ThreadNative::kill(p_thread);
    };
};