#pragma once

template <class ElementType> struct Mutex
{
    ElementType _data;
    mutex_native _mutex;

    inline static Mutex<ElementType> allocate()
    {
        Mutex<ElementType> l_mutex{};
        l_mutex._mutex = mutex_native_allocate();
        return l_mutex;
    };

    inline void free()
    {
        mutex_native_lock(this->_mutex);
        mutex_native_free(this->_mutex);
    };

    template <class t_WithFunc> inline void acquire(const t_WithFunc& p_with)
    {
        mutex_native_lock(this->_mutex);
        p_with(_data);
        mutex_native_unlock(this->_mutex);
    };
};