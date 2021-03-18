#pragma once

template <class ElementType> struct Mutex
{
    ElementType _data;
    int8 is_locked;

    inline static Mutex<ElementType> build_default()
    {
        return Mutex<ElementType>{};
    };

    inline static Mutex<ElementType> build(const ElementType& p_data)
    {
        return Mutex<ElementType>{p_data, 0};
    };

    template <class t_WithFunc> inline void acquire(const t_WithFunc& p_with)
    {
        while (this->is_locked)
        {
        }
        this->is_locked = 1;
        p_with(_data);
        this->is_locked = 0;
    };
};

struct mutex_
{
    int8 is_locked;

    template <class t_WithFunc> inline void acquire(const t_WithFunc& p_with)
    {
        while (this->is_locked)
        {
        }
        this->is_locked = 1;
        p_with();
        this->is_locked = 0;
    };
};