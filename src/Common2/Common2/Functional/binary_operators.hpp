#pragma once

template <class Flag, class Flag_t> inline constexpr Flag _bin_or(const Flag p_left, const Flag p_right)
{
    return (Flag)((Flag_t)p_left | (Flag_t)p_right);
};

template <class Flag, class Flag_t> inline constexpr Flag _bin_and(const Flag p_left, const Flag p_right)
{
    return (Flag)((Flag_t)p_left & (Flag_t)p_right);
};

#define declare_binary_operations(Type)                                                                                                                                                                \
    inline constexpr Type operator|(const Type p_left, const Type p_right)                                                                                                                             \
    {                                                                                                                                                                                                  \
        return _bin_or<Type, Type##_t>(p_left, p_right);                                                                                                                                               \
    };                                                                                                                                                                                                 \
    inline constexpr Type operator&(const Type p_left, const Type p_right)                                                                                                                             \
    {                                                                                                                                                                                                  \
        return _bin_and<Type, Type##_t>(p_left, p_right);                                                                                                                                              \
    }