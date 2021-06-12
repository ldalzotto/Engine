#pragma once

mutex_native mutex_native_allocate(){
    mutex_native l_mutex;
    l_mutex.ptr = CreateMutex(NULL, 0, NULL);
#if __DEBUG
    assert_true(l_mutex.ptr != NULL);
#endif
    return l_mutex;
};
void mutex_native_lock(const mutex_native p_mutex){
    DWORD l_wait = WaitForSingleObject(p_mutex.ptr, INFINITE);
#if __DEBUG
    assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
};

void mutex_native_unlock(const mutex_native p_mutex){
    BOOL l_rm = ReleaseMutex(p_mutex.ptr);
#if __DEBUG
    assert_true(l_rm);
#endif
};

void mutex_native_free(mutex_native& p_mutex){
    BOOL l_ch = CloseHandle(p_mutex.ptr);
#if __DEBUG
    assert_true(l_ch);
#endif
    p_mutex.ptr = 0;
};