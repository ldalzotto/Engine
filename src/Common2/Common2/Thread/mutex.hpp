#pragma once

template <class ElementType> struct MutexNative
{
    ElementType _data;
    HANDLE _mutex;

    inline static MutexNative<ElementType> allocate()
    {
        MutexNative<ElementType> l_mutex{};
        l_mutex._mutex = CreateMutex(NULL, 0, NULL);
#if __DEBUG
        assert_true(l_mutex._mutex != NULL);
#endif
        return l_mutex;
    };

    inline void free()
    {
        DWORD l_wait = WaitForSingleObject(this->_mutex, INFINITE);
#if __DEBUG
        assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
        BOOL l_ch = CloseHandle(this->_mutex);
#if __DEBUG
        assert_true(l_ch);
#endif
    };

    template <class t_WithFunc> inline void acquire(const t_WithFunc& p_with)
    {
        DWORD l_wait = WaitForSingleObject(this->_mutex, INFINITE);
#if __DEBUG
        assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
        p_with(_data);
        BOOL l_rm = ReleaseMutex(this->_mutex);
#if __DEBUG
        assert_true(l_rm);
#endif
    };
};