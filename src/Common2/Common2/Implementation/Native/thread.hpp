#pragma once

thread_native thread_native_spawn_thread(void* p_function, void* p_parameter)
{
    thread_native l_thread;
    DWORD l_id;
    l_thread.ptr = (int8*)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)p_function, (LPVOID)p_parameter, 0, &l_id);

#if __DEBUG
    assert_true(l_thread.ptr != NULL);
#endif
    return l_thread;
};

thread_native thread_native_get_current_thread()
{
    thread_native l_thread;
    l_thread.ptr = (int8*)GetCurrentThread();
    return l_thread;
};

int8 thread_native_wait_for_end(const thread_native p_thread, const uimax p_time_in_ms)
{
#if __DEBUG
    assert_true(
#endif
        WaitForSingleObject(p_thread.ptr, (DWORD)p_time_in_ms)
#if __DEBUG
        != WAIT_FAILED)
#endif
        ;

    DWORD l_exit_code;
    assert_true(GetExitCodeThread(p_thread.ptr, &l_exit_code));
    return (int8)l_exit_code;
};

void thread_native_sleep(const thread_native p_thread, const uimax p_time_in_ms){
    // TODO
};

void thread_native_terminate(const thread_native p_thread)
{
    TerminateThread(p_thread.ptr, 0);
};
void thread_native_kill(const thread_native p_thread)
{
    thread_native_terminate(p_thread);
};