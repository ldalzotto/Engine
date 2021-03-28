#pragma once

template <class type_element> struct MutexNative
{
    type_element _data;
    HANDLE _mutex;
};

#define MutexNative_get_data(thiz) (thiz)->_data
#define MutexNative_get_mutex(thiz) (thiz)->_mutex

template <class type_element>
inline MutexNative<type_element> MutexNative_allocate()
{
    MutexNative<type_element> l_mutex{};
    l_mutex._mutex = CreateMutex(NULL, 0, NULL);
#if __DEBUG
    assert_true(l_mutex._mutex != NULL);
#endif
    return l_mutex;
};

template <class type_element>
inline void MutexNative_free(MutexNative<type_element>* thiz)
{
    DWORD l_wait = WaitForSingleObject(MutexNative_get_mutex(thiz), INFINITE);
#if __DEBUG
    assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
    BOOL l_ch = CloseHandle(MutexNative_get_mutex(thiz));
#if __DEBUG
    assert_true(l_ch);
#endif
};

template <class type_element, class func_with> inline void MutexNative_acquire(MutexNative<type_element>* thiz, CB(func_with) p_with)
{
    DWORD l_wait = WaitForSingleObject(MutexNative_get_mutex(thiz), INFINITE);
#if __DEBUG
    assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
    p_with(thiz->_data);
    BOOL l_rm = ReleaseMutex(MutexNative_get_mutex(thiz));
#if __DEBUG
    assert_true(l_rm);
#endif
};