#pragma once

#if MEM_LEAK_DETECTION

#define MEM_LEAK_MAX_POINTER_COUNTER 10000
#define MEM_LEAK_MAX_BACKTRACE 64

int8* ptr_counter[MEM_LEAK_MAX_POINTER_COUNTER] = {};

typedef LPVOID backtrace_t[MEM_LEAK_MAX_BACKTRACE];

backtrace_t backtraces[MEM_LEAK_MAX_POINTER_COUNTER] = {};

#include "dbghelp.h"

inline void capture_backtrace(const uimax p_ptr_index)
{
    CaptureStackBackTrace(0, MEM_LEAK_MAX_BACKTRACE, backtraces[p_ptr_index], NULL);
};

inline int8* push_ptr_to_tracked(int8* p_ptr)
{
    for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
    {
        if (ptr_counter[i] == NULL)
        {
            ptr_counter[i] = p_ptr;
            capture_backtrace(i);
            return p_ptr;
        }
    }

    abort();
};

inline void remove_ptr_to_tracked(int8* p_ptr)
{
    for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
    {
        if (ptr_counter[i] == p_ptr)
        {
            ptr_counter[i] = NULL;
            return;
        }
    }

    abort();
};

#endif

inline void memleak_ckeck()
{
#if MEM_LEAK_DETECTION
    for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
    {
        if (ptr_counter[i] != NULL)
        {
            abort();
        }
    }
#endif
};

inline int8* heap_malloc(const uimax p_size)
{
#if MEM_LEAK_DETECTION
    return push_ptr_to_tracked((int8*)::malloc(p_size));
#else
    return (int8*)::malloc(p_size);
#endif
};

inline int8* heap_calloc(const uimax p_size)
{
#if MEM_LEAK_DETECTION
    return push_ptr_to_tracked((int8*)::calloc(1, p_size));
#else
    return (int8*)::calloc(1, p_size);
#endif
};

inline int8* heap_realloc(int8* p_memory, const uimax p_new_size)
{
#if MEM_LEAK_DETECTION
    remove_ptr_to_tracked(p_memory);
    return push_ptr_to_tracked((int8*)::realloc(p_memory, p_new_size));
#else
    return (int8*)::realloc(p_memory, p_new_size);
#endif
};

inline void heap_free(int8* p_memory)
{
#if MEM_LEAK_DETECTION
    remove_ptr_to_tracked(p_memory);
#endif
    ::free(p_memory);
};

inline int8* memory_cpy(int8* p_dst, const int8* p_src, const uimax p_size)
{
    return (int8*)::memcpy(p_dst, p_src, p_size);
};

inline static int8* memory_cpy_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
    if (p_size > p_dst_size)
    {
        abort();
    }
#endif

    return memory_cpy(p_dst, p_src, p_size);
};

inline void memory_zero(int8* p_dst, const uimax p_size)
{
    ::memset(p_dst, 0, p_size);
};

inline void memory_zero_safe(int8* p_dst, const uimax p_dst_size, const uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
    if (p_size > p_dst_size)
    {
        abort();
    }
#endif
    memory_zero(p_dst, p_size);
};

inline int8* memory_move(int8* p_dst, const int8* p_src, const uimax p_size)
{
    return (int8*)::memmove(p_dst, p_src, p_size);
};

inline int8* memory_move_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
    if (p_size > p_dst_size)
    {
        abort();
    }
#endif

    return memory_move(p_dst, p_src, p_size);
};

inline int8 memory_compare(const int8* p_source, const int8* p_compared, const uimax p_size)
{
    return ::memcmp(p_source, p_compared, p_size) == 0;
};

template <class ElementType> inline uimax memory_offset_bytes(const uimax p_size)
{
    return sizeof(ElementType) * p_size;
};

inline constexpr uimax strlen_constexpr(const char* start)
{
    const char* end = start;
    while (*end++ != 0)
        ;
    return end - start - 1;
};
