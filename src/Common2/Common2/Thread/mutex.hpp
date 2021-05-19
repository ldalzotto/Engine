#pragma once

#if _WIN32
using mutex_t = HANDLE;
#else
using mutex_t = pthread_mutex_t;
#endif

struct MutexNative
{
    inline static mutex_t allocate();
    inline static void lock(mutex_t& p_mutex);
    inline static void unlock(mutex_t& p_mutex);
    inline static void free(mutex_t& p_mutex);
};

#if _WIN32

inline mutex_t MutexNative::allocate()
{
    mutex_t l_mutex = CreateMutex(NULL, 0, NULL);
#if __DEBUG
    assert_true(l_mutex != NULL);
#endif
    return l_mutex;
};

inline void MutexNative::lock(mutex_t& p_mutex)
{
    DWORD l_wait = WaitForSingleObject(p_mutex, INFINITE);
#if __DEBUG
    assert_true(l_wait != WAIT_ABANDONED && l_wait != WAIT_TIMEOUT && l_wait != WAIT_FAILED);
#endif
};

inline void MutexNative::unlock(mutex_t& p_mutex)
{
    BOOL l_rm = ReleaseMutex(p_mutex);
#if __DEBUG
    assert_true(l_rm);
#endif
};

inline void MutexNative::free(mutex_t& p_mutex)
{
    BOOL l_ch = CloseHandle(p_mutex);
#if __DEBUG
    assert_true(l_ch);
#endif
};

#else

inline mutex_t MutexNative::allocate()
{
    mutex_t l_mutex;
    int l_return = pthread_mutex_init(&l_mutex, NULL);
#if __DEBUG
    assert_true(l_return == 0);
#endif
    return l_mutex;
};

inline void MutexNative::lock(mutex_t& p_mutex)
{
    timespec l_wait = miliseconds_to_timespec(-1);
    int l_return = pthread_mutex_timedlock(&p_mutex, &l_wait);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

inline void MutexNative::unlock(mutex_t& p_mutex)
{
    int l_return = pthread_mutex_unlock(&p_mutex);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

inline void MutexNative::free(mutex_t& p_mutex)
{
    int l_return = pthread_mutex_destroy(&p_mutex);
#if __DEBUG
    assert_true(l_return == 0);
#endif
};

#endif

template <class ElementType> struct Mutex
{
    ElementType _data;
    mutex_t _mutex;

    inline static Mutex<ElementType> allocate()
    {
        Mutex<ElementType> l_mutex{};
        l_mutex._mutex = MutexNative::allocate();
        return l_mutex;
    };

    inline void free()
    {
        MutexNative::lock(this->_mutex);
        MutexNative::free(this->_mutex);
    };

    template <class t_WithFunc> inline void acquire(const t_WithFunc& p_with)
    {
        MutexNative::lock(this->_mutex);
        p_with(_data);
        MutexNative::unlock(this->_mutex);
    };
};