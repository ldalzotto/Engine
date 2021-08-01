#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#if __DEBUG

#define __MEMLEAK 1
#define __TOKEN 1

#elif __RELEASE

#define __MEMLEAK 0
#define __TOKEN 0

#endif

#ifndef __LIB
#define __LIB 0
#endif

// GLOBAL_STATIC is a shared variable across static lib
#if __LIB == 1
#define GLOBAL_STATIC extern
#define GLOBAL_STATIC_INIT(code)
#else
#define GLOBAL_STATIC
#define GLOBAL_STATIC_INIT(code) = code
#endif

#ifndef TYPES_H
#define TYPES_H

using int8 = char;
using uint8 = unsigned char;

using int16 = short;
using uint16 = unsigned short;

using int32 = int;
using uint32 = unsigned int;

using int64 = long long;
using uint64 = unsigned long long;

using float32 = float;
using float64 = double;
using float128 = long double;

using uimax = size_t;

const uint8 uint8_max = UCHAR_MAX;
const uint16 uint16_max = USHRT_MAX;

#endif

#ifndef MACROS_H
#define MACROS_H

#define COMA ,
#define SEMICOLON ;
#define STR(V1) #V1
#define CONCAT_2(V1, V2) V1##V2
#define CONCAT_3(V1, V2, V3) V1##V2##V3

#define LINE_RETURN \n
#define MULTILINE(...) #__VA_ARGS__
#define MULTILINE_(...) MULTILINE(__VA_ARGS__)

#define loop(Iteratorname, BeginNumber, EndNumber)                                                                                                                                                     \
    uimax Iteratorname = BeginNumber;                                                                                                                                                                  \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++
#define loop_int16(Iteratorname, BeginNumber, EndNumber)                                                                                                                                               \
    int16 Iteratorname = BeginNumber;                                                                                                                                                                  \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++

#define loop_int8(Iteratorname, BeginNumber, EndNumber)                                                                                                                                                \
    int8 Iteratorname = BeginNumber;                                                                                                                                                                   \
    Iteratorname < EndNumber;                                                                                                                                                                          \
    Iteratorname++

#define loop_reverse(Iteratorname, BeginNumber, EndNumber)                                                                                                                                             \
    uimax Iteratorname = EndNumber - 1;                                                                                                                                                                \
    Iteratorname != ((uimax)BeginNumber - 1);                                                                                                                                                          \
    --Iteratorname

#endif

#ifndef LIMITS_H
#define LIMITS_H
namespace Limits
{
// const int8* float_string_format = "%f.5";
const int8 tol_digit_number = 5;
const float64 tol_f = ((float64)1 / pow(10, tol_digit_number));
}; // namespace Limits

#define float_string_format % .5f
#define float_string_format_str "%.5f"

#define uimax_string_format % lld
#define uimax_string_format_str "%lld"

#define int32_string_format_str "%d"

#endif

#include "./_external/Syscall/syscall_interface.hpp"

#ifndef ASSERT_H
#define ASSERT_H

inline void assert_true(int8 p_condition)
{
    if (!p_condition)
    {
        abort();
    }
};

#endif

#ifndef MUTEX_H
#define MUTEX_H
template <class ElementType> struct Mutex
{
    using element_type = ElementType;
    element_type _data;
    mutex_native _mutex;

    inline static Mutex allocate()
    {
        Mutex l_mutex{};
        l_mutex._mutex = mutex_native_allocate();
        return l_mutex;
    };

    inline void free()
    {
        mutex_native_lock(this->_mutex);
        mutex_native_unlock(this->_mutex);
        mutex_native_free(this->_mutex);
    };

    template <class t_WithFunc> inline void acquire(const t_WithFunc& p_with)
    {
        mutex_native_lock(this->_mutex);
        p_with(_data);
        mutex_native_unlock(this->_mutex);
    };
};
#endif

#ifndef MEMORY_H
#define MEMORY_H

using token_t = uimax;

template <class _Allocator> struct TAllocator
{
    using allocator_pointer_type = _Allocator*;
    using token_value_type = typename _Allocator::token_value_type;

    allocator_pointer_type heap;

    token_value_type malloc(size_t p_size)
    {
        return (token_value_type)this->heap->malloc(p_size);
    };

    void free(token_value_type p_memory)
    {
        this->heap->free(p_memory);
    };

    int8 realloc(token_value_type p_memory, size_t p_new_size, token_value_type* out_memory)
    {
        return this->heap->realloc(p_memory, p_new_size, out_memory);
    };

    template <class ElementType> ElementType* get_memory(token_value_type p_memory)
    {
        return this->heap->template get_memory<ElementType>(p_memory);
    };
};

constexpr int32 MEM_LEAK_MAX_POINTER_COUNTER = 10000;
constexpr int8 MEM_LEAK_MAX_BACKTRACE = 64;

typedef int8* ptr_counter_t[MEM_LEAK_MAX_POINTER_COUNTER];
typedef void* backtrace_t[MEM_LEAK_MAX_BACKTRACE];

// #include "dbghelp.h"

struct Memleak
{
    // Not using mutex but a hash based nested table ?
    // -> NO, because some resource may be allocated from one thread and dealocated a different one
    Mutex<ptr_counter_t> ptr_counter;
    backtrace_t backtraces[MEM_LEAK_MAX_POINTER_COUNTER];

    static Memleak allocate()
    {
        Memleak l_memleak;
        l_memleak.ptr_counter = l_memleak.ptr_counter.allocate();
        return l_memleak;
    };

    void free()
    {
        this->ptr_counter.free();
    };

    int8* push_ptr_to_tracked(int8* p_ptr)
    {
        int8 l_backtrace_pushed = 0;
        int8* l_return = NULL;
        this->ptr_counter.acquire([&](ptr_counter_t p_ptr_counter) {
            for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
            {
                if (p_ptr_counter[i] == NULL)
                {
                    p_ptr_counter[i] = p_ptr;
                    backtrace_capture(this->backtraces[i], MEM_LEAK_MAX_BACKTRACE);
                    l_return = p_ptr;
                    l_backtrace_pushed = 1;
                    break;
                }
            }
        });

        if (l_backtrace_pushed)
        {
            return l_return;
        }

        abort();
    };

    void remove_ptr_to_tracked(int8* p_ptr)
    {
        int8 l_successful = 0;
        this->ptr_counter.acquire([&](ptr_counter_t p_ptr_counter) {
            for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
            {
                if (p_ptr_counter[i] == p_ptr)
                {
                    p_ptr_counter[i] = NULL;
                    l_successful = 1;
                    break;
                }
            }
        });

        if (l_successful)
        {
            return;
        }

        abort();
    };

    void check()
    {
#if __MEMLEAK
        this->ptr_counter.acquire([&](ptr_counter_t p_ptr_counter) {
            for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
            {
                if (p_ptr_counter[i] != NULL)
                {
                    /*
                    for (loop(j, 0, MEM_LEAK_MAX_BACKTRACE))
                    {
                        LPVOID l_func_pointer = backtraces[i][j];
                        if (l_func_pointer)
                        {
                            SYMBOL_INFO l_symbol;
                            assert_true(SymFromAddr(GetCurrentProcess(), (DWORD64)l_func_pointer, 0, &l_symbol));
                            printf(l_symbol.Name);
                            printf("\n");
                        }
                    }
                    */
                    abort();
                }
            }
        });

#endif
    };
};

struct GlobalHeap
{
    using token_value_type = int8*;

#if __MEMLEAK
    Memleak memleak;
#endif

    static GlobalHeap allocate()
    {
        GlobalHeap l_global_heap;
        l_global_heap.memleak = l_global_heap.memleak.allocate();
        return l_global_heap;
    };

    void free_heap()
    {
        this->memleak.free();
    };

    token_value_type malloc(size_t p_size)
    {
#if __MEMLEAK
        return this->memleak.push_ptr_to_tracked((int8*)::malloc(p_size));
#else
        return (int8*)::malloc(p_size);
#endif
    };
    void free(token_value_type p_memory)
    {
#if __MEMLEAK
        this->memleak.remove_ptr_to_tracked(p_memory);
#endif
        ::free(p_memory);
    };

    token_value_type calloc(uimax p_size)
    {
#if __MEMLEAK
        return (token_value_type)this->memleak.push_ptr_to_tracked((int8*)::calloc(1, p_size));
#else
        return (token_value_type)::calloc(1, p_size);
#endif
    };

    int8 realloc(token_value_type p_memory, uimax p_new_size, token_value_type* out_token)
    {
#if __MEMLEAK
        this->memleak.remove_ptr_to_tracked(p_memory);
#endif
        int8* l_new_memory = (int8*)::realloc(p_memory, p_new_size);
#if __MEMLEAK
        if (l_new_memory != NULL)
        {
            this->memleak.push_ptr_to_tracked(l_new_memory);
        }
#endif
        *out_token = l_new_memory;
        return l_new_memory != NULL;
    };

    template <class ElementType> ElementType* get_memory(token_value_type p_memory)
    {
        return (ElementType*)p_memory;
    };

    void memleak_check()
    {
        this->memleak.check();
    };
};

GlobalHeap _g_heap = GlobalHeap::allocate();

struct GlobalHeapNoAlloc
{
    using token_value_type = int8*;

    token_value_type malloc(uimax p_size)
    {
        return 0;
    };

    void free(token_value_type p_memory){};

    token_value_type calloc(uimax p_size)
    {
        return 0;
    };

    int8 realloc(token_value_type p_memory, uimax p_new_size, token_value_type* out_token)
    {
        *out_token = p_memory;
        return 1;
    };

    template <class ElementType> ElementType* get_memory(token_value_type p_memory)
    {
        return (ElementType*)p_memory;
    };
};

GlobalHeapNoAlloc _g_heap_noalloc;

inline int8* memory_cpy(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if __DEBUG
    if (p_size > p_dst_size)
    {
        abort();
    }
#endif

    return (int8*)::memcpy(p_dst, p_src, p_size);
};

inline void memory_zero(int8* p_dst, const uimax p_dst_size, const uimax p_size)
{
#if __DEBUG
    if (p_size > p_dst_size)
    {
        abort();
    }
#endif
    ::memset(p_dst, 0, p_size);
};

inline int8* memory_move(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if __DEBUG
    if (p_size > p_dst_size)
    {
        abort();
    }
#endif
    return (int8*)::memmove(p_dst, p_src, p_size);
};

inline int8 memory_compare(const int8* p_source, const int8* p_compared, const uimax p_size)
{
    return ::memcmp(p_source, p_compared, p_size) == 0;
};

uimax memory_resize_until_capacity_met_get_new_capacity(uimax p_size, uimax p_desired_capacity)
{
    uimax l_resized_capacity = p_size;

    if (l_resized_capacity >= p_desired_capacity)
    {
        return 0;
    }

    if (l_resized_capacity == 0)
    {
        l_resized_capacity = 1;
    }

    while (l_resized_capacity < p_desired_capacity)
    {
        l_resized_capacity *= 2;
    }

    return l_resized_capacity;
};

#endif

#ifndef SLICE_H
#define SLICE_H

/*
    A Slice is an encapsulated C style array.
*/
template <class ElementType> struct Slice
{
    uimax size;
    ElementType* memory;

    static Slice build(ElementType* p_memory, uimax p_size)
    {
        Slice<ElementType> l_slice;
        l_slice.memory = p_memory;
        l_slice.size = p_size;
        return l_slice;
    };

    static Slice build_default()
    {
        Slice<ElementType> l_slice;
        l_slice.memory = 0;
        l_slice.size = 0;
        return l_slice;
    };

    static Slice build_memory_elementnb(ElementType* p_memory, uimax p_element_nb)
    {
        return build(p_memory, p_element_nb);
    };

    static Slice build_memory_offset_elementnb(ElementType* p_memory, uimax p_offset, uimax p_element_nb)
    {
        return build(&(p_memory[p_offset]), p_element_nb);
    };

    Slice<int8> build_asint8()
    {
        return Slice<int8>::build(this->memory, sizeof(ElementType) * this->size);
    };

    Slice<int8> build_asint8_memory_elementnb(uimax p_element_nb)
    {
        return build(this->memory, sizeof(ElementType) * p_element_nb);
    };

    static Slice build_begin_end(ElementType* p_memory, uimax p_begin, uimax p_end)
    {
        return build(&(p_memory[p_begin]), p_end - p_begin);
    };

    static Slice<int8> build_rawstr(const int8* p_str)
    {
        return Slice<int8>::build_memory_elementnb((int8*)p_str, strlen(p_str));
    };

    void bound_assert(uimax p_index)
    {
#if __DEBUG
        if (p_index > this->size)
        {
            abort();
        }
#endif
    };

    void bound_inside_assert(Slice<ElementType>* p_tested_slice)
    {
#if __DEBUG
        if (this->size == 0)
        {
            abort();
        }
        if (p_tested_slice->size != 0)
        {
            if ((&p_tested_slice->memory[p_tested_slice->size]) > &(this->memory[this->size]))
            {
                abort();
            }
        }
#endif
    };

    ElementType* get(uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < this->size);
#endif

        return &this->memory[p_index];
    };

    template <class CastedElementType> Slice<CastedElementType> cast()
    {

#if __DEBUG
        if (sizeof(ElementType) <= sizeof(CastedElementType))
        {
            if ((this->size % sizeof(CastedElementType)) != 0)
            {
                abort();
            }
        }
        else
        {
            if ((sizeof(CastedElementType) % this->size) != 0)
            {
                abort();
            }
        }
#endif

        return Slice<CastedElementType>::build((CastedElementType*)this->memory, (uimax)((this->size * sizeof(ElementType)) / sizeof(CastedElementType)));
    };

    void zero()
    {
        memory_zero((int8*)this->memory, this->size * sizeof(ElementType), this->size * sizeof(ElementType));
    };

    void slide(uimax p_offset_index)
    {
#if __DEBUG
        assert_true(p_offset_index <= this->size);
#endif

        this->memory = &(this->memory[p_offset_index]);
        this->size -= p_offset_index;
    };

    Slice slide_rv(uimax p_offset_index)
    {
        Slice l_return = *this;
        l_return.slide(p_offset_index);
        return l_return;
    };

    int8 compare(Slice<ElementType>* p_other)
    {
        return memory_compare((int8*)this->memory, (int8*)p_other->memory, p_other->size * sizeof(ElementType));
    };

    int8 compare_0v(Slice<ElementType> p_other)
    {
        return this->compare(&p_other);
    };

    void move_memory(Slice<ElementType>* p_source)
    {
        memory_move((int8*)this->memory, this->size * sizeof(ElementType), (int8*)p_source->memory, p_source->size * sizeof(ElementType));
    };

    void move_memory_down(uimax p_moved_block_size, uimax p_break_index, uimax p_move_delta)
    {
        Slice<ElementType> l_target = build_memory_offset_elementnb(this->memory, p_break_index + p_move_delta, p_moved_block_size);
#if __DEBUG
        this->bound_inside_assert(&l_target);
#endif
        Slice<ElementType> l_source = build_begin_end(this->memory, p_break_index, p_break_index + p_moved_block_size);
        l_target.move_memory(&l_source);
    };

    void move_memory_up(uimax p_moved_block_size, uimax p_break_index, uimax p_move_delta)
    {
        Slice<ElementType> l_target = build_memory_offset_elementnb(this->memory, p_break_index - p_move_delta, p_moved_block_size);
#if __DEBUG
        this->bound_inside_assert(&l_target);
#endif
        Slice<ElementType> l_source = build_begin_end(this->memory, p_break_index, p_break_index + p_moved_block_size);
        l_target.move_memory(&l_source);
    };

    void copy_memory(Slice<ElementType>* p_elements)
    {
        memory_cpy((int8*)this->memory, this->size * sizeof(ElementType), (int8*)p_elements->memory, p_elements->size * sizeof(ElementType));
    };

    void copy_memory_2(Slice<ElementType>* p_elements_1, Slice<ElementType>* p_elements_2)
    {
        this->copy_memory(p_elements_1);
        Slice l_slided_slice = this->slide_rv(p_elements_1->size);
        l_slided_slice.copy_memory(p_elements_2);
    };

    void copy_memory_3(Slice<ElementType>* p_elements_1, Slice<ElementType>* p_elements_2, Slice<ElementType>* p_elements_3)
    {
        this->copy_memory_2(p_elements_1, p_elements_2);
        Slice l_slided_slice = this->slide_rv(p_elements_1->size + p_elements_2->size);
        l_slided_slice.copy_memory(p_elements_3);
    };

    void copy_memory_at_index(uimax p_copy_index, Slice<ElementType>* p_elements)
    {
        Slice l_target = build_memory_elementnb(&(this->memory[p_copy_index]), p_elements->size);

#if __DEBUG
        this->bound_inside_assert(&l_target);
#endif

        l_target.copy_memory(p_elements);
    };

    void copy_memory_at_index_2(uimax p_copy_index, Slice<ElementType>* p_elements_1, Slice<ElementType>* p_elements_2)
    {
        this->copy_memory_at_index(p_copy_index, p_elements_1);
        this->copy_memory_at_index(p_copy_index + p_elements_1->size, p_elements_2);
    };

    void copy_memory_at_index_2_1v_2v(uimax p_copy_index, Slice<ElementType> p_elements_1, Slice<ElementType> p_elements_2)
    {
        return this->copy_memory_at_index_2(p_copy_index, &p_elements_1, &p_elements_2);
    };

    void copy_memory_at_index_3(uimax p_copy_index, Slice<ElementType>* p_elements_1, Slice<ElementType>* p_elements_2, Slice<ElementType>* p_elements_3)
    {
        this->copy_memory_at_index_2(p_copy_index, p_elements_1, p_elements_2);
        this->copy_memory_at_index(p_copy_index + p_elements_1->size + p_elements_2->size, p_elements_3);
    };
};

template <class ElementType, uimax Size_t> struct SliceN
{
    ElementType Memory[Size_t];

    constexpr uimax size()
    {
        return Size_t;
    };

    ElementType* get(uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < Size_t);
#endif
        return &(this->Memory[p_index]);
    };
};

template <class ElementType, uimax Size_t> inline Slice<ElementType> slice_from_slicen(SliceN<ElementType, Size_t>* p_slice_n)
{
    return Slice<ElementType>{Size_t, (ElementType*)p_slice_n->Memory};
};

/*
    A SliceIndex is just a begin and end uimax
*/
struct SliceIndex
{
    uimax begin;
    uimax size;

    static SliceIndex build(uimax p_begin, uimax p_size)
    {
        SliceIndex l_slice_index;
        l_slice_index.begin = p_begin;
        l_slice_index.size = p_size;
        return l_slice_index;
    };

    static SliceIndex build_default()
    {
        return build(0, 0);
    };

    void slice_two(uimax p_break_point, SliceIndex* out_left, SliceIndex* out_right)
    {
        uimax l_source_initial_size = this->size;
        *out_left = SliceIndex::build(this->begin, p_break_point - this->begin);
        *out_right = SliceIndex::build(p_break_point, l_source_initial_size - out_left->size);
    };
};

#endif

#ifndef SPAN_H
#define SPAN_H

template <class ElementType, class _Allocator> struct iSpan
{
    using allocator_value_type = TAllocator<_Allocator>;
    using allocator_token_value_type = typename allocator_value_type::token_value_type;

    allocator_value_type allocator;
    uimax size;
    allocator_token_value_type memory;

    void allocate(uimax p_capacity, _Allocator* p_allocator)
    {
        this->allocator = {p_allocator};
        this->size = p_capacity;
        this->memory = this->allocator.malloc(p_capacity * sizeof(ElementType));
    };

    void allocate_slice(Slice<ElementType>* p_elements, _Allocator* p_allocator)
    {
        this->allocate(p_elements->size, p_allocator);
        this->to_slice().copy_memory(p_elements);
    };

    void allocate_slice_0v(Slice<ElementType> p_elements, _Allocator* p_allocator)
    {
        this->allocate_slice(&p_elements, p_allocator);
    };

    void build_memory(allocator_token_value_type p_memory, uimax p_size, _Allocator* p_allocator)
    {
        this->allocator = {p_allocator};
        this->size = p_size;
        this->memory = p_memory;
    };

    void free()
    {
        this->allocator.free(this->memory);
        this->memory = 0;
        this->size = 0;
    };

    Slice<ElementType> to_slice()
    {
        return Slice<ElementType>::build(this->get_memory(), this->size);
    };

    ElementType* get(uimax p_index)
    {
        return this->to_slice().get(p_index);
    };

    int8 resize(uimax p_new_capacity)
    {
        if (p_new_capacity > this->size)
        {
            allocator_token_value_type l_newMemory;
            if (this->allocator.realloc(this->memory, p_new_capacity * sizeof(ElementType), &l_newMemory))
            {
                this->memory = l_newMemory;
                this->size = p_new_capacity;
                return 1;
            }
            return 0;
        }
        return 1;
    };

    ElementType* get_memory()
    {
        return this->allocator.template get_memory<ElementType>(this->memory);
    };
};

#endif

#ifndef VECTOR_H
#define VECTOR_H

template <class ElementType, class _Allocator> struct iVector
{
    using allocator_token_value_type = typename _Allocator::token_value_type;

    uimax size;
    iSpan<ElementType, _Allocator> memory;

    void allocate(uimax p_initial_capacity, _Allocator* p_allocator)
    {
        this->size = 0;
        this->memory.allocate(p_initial_capacity, p_allocator);
    };

    void allocate_slice(Slice<ElementType>* p_elements, _Allocator* p_allocator)
    {
        this->size = p_elements->size;
        this->memory.allocate_slice(p_elements, p_allocator);
    };

    void allocate_slice_0v(Slice<ElementType> p_elements, _Allocator* p_allocator)
    {
        this->allocate_slice(&p_elements, p_allocator);
    };

    void build_memory(allocator_token_value_type p_memory, uimax p_size, _Allocator* p_allocator)
    {
        this->size = 0;
        this->memory.build_memory(p_memory, p_size, p_allocator);
    };

    void free()
    {
        this->memory.free();
        this->size = 0;
    };

    void clear()
    {
        this->size = 0;
    };

    void _bound_check(uimax p_index)
    {
#if __DEBUG
        if (p_index > this->size)
        {
            abort();
        }
#endif
    };

    void _bound_head_check(uimax p_index)
    {
#if __DEBUG
        if (p_index == this->size)
        {
            abort();
        }
#endif
    };

    void _bound_lastelement_check(uimax p_index)
    {
#if __DEBUG
        if (p_index == this->size - 1)
        {
            abort();
        }
#endif
    };

    int8 empty()
    {
        return this->size == 0;
    };

    Slice<ElementType> to_slice()
    {
        return Slice<ElementType>::build_memory_elementnb(get_memory(), this->size);
    };

    ElementType* get_memory()
    {
        return this->memory.get_memory();
    };

    ElementType* get(uimax p_index)
    {
        return this->to_slice().get(p_index);
    };

    void push_back_element_empty()
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + 1);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }
        this->size += 1;
    };

    void push_back_element(ElementType* p_element)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + 1);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }
        *this->memory.get(this->size) = *p_element;
        this->size += 1;
    };

    void push_back_element_0v(ElementType p_element)
    {
        this->push_back_element(&p_element);
    };

    void insert_element_at(ElementType* p_element, uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < this->size);
        assert_true(p_index != (this->size - 1)); // cannot insert at head. Use iVector_insert_element_at_always instead.
#endif
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + 1);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }

        Slice<ElementType> l_slice = this->memory.to_slice();
        l_slice.move_memory_down(this->size - p_index, p_index, 1);
        Slice<ElementType> l_target = Slice<ElementType>::build(p_element, 1);
        l_slice.copy_memory_at_index(p_index, &l_target);

        this->size += 1;
    };

    void insert_element_at_0v(ElementType p_element, uimax p_index)
    {
        this->insert_element_at(&p_element, p_index);
    };

    void push_back_array(Slice<ElementType>* p_elements_0)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }

        this->memory.to_slice().copy_memory_at_index(this->size, p_elements_0);
        this->size += p_elements_0->size;
    };

    void push_back_array_empty(uimax p_elements_0_size)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0_size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }

        this->size += p_elements_0_size;
    };

    void push_back_array_2(Slice<ElementType>* p_elements_0, Slice<ElementType>* p_elements_1)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size + p_elements_1->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }

        this->memory.to_slice().copy_memory_at_index_2(this->size, p_elements_0, p_elements_1);
        this->size += (p_elements_0->size + p_elements_1->size);
    };

    void push_back_array_3(Slice<ElementType>* p_elements_0, Slice<ElementType>* p_elements_1, Slice<ElementType>* p_elements_2)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size + p_elements_1->size + p_elements_2->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }

        this->memory.to_slice().copy_memory_at_index_3(this->size, p_elements_0, p_elements_1, p_elements_2);
        this->size += (p_elements_0->size + p_elements_1->size + p_elements_2->size);
    };

    void insert_array_at(Slice<ElementType>* p_elements_0, uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < this->size);
        assert_true(p_index != (this->size - 1)); // cannot insert at head. Use insert_array_at_always instead.
#endif
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity);
        }

        Slice<ElementType> l_slice = this->memory.to_slice();
        l_slice.move_memory_down(this->size - p_index, p_index, p_elements_0->size);
        l_slice.copy_memory_at_index(p_index, p_elements_0);
        this->size += p_elements_0->size;
    };

    void insert_array_at_always(Slice<ElementType>* p_elements, uimax p_index)
    {
        if (p_index == this->size)
        {
            this->push_back_array(p_elements);
        }
        else
        {
            this->insert_array_at(p_elements, p_index);
        }
    };

    void insert_element_at_always(ElementType* p_element, uimax p_index)
    {
        if (p_index == this->size)
        {
            this->push_back_element(p_element);
        }
        else
        {
            this->insert_element_at(p_element, p_index);
        }
    };

    void erase_element_at(uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < this->size);
        assert_true(p_index != this->size - 1); // use pop_back
#endif

        this->memory.to_slice().move_memory_up(this->size - (p_index + 1), p_index + 1, 1);
        this->size -= 1;
    };

    void erase_array_at(uimax p_index, uimax p_elements_0_size)
    {
#if __DEBUG
        assert_true((p_index + p_elements_0_size) < this->size);
        assert_true(p_index != this->size - 1); // use pop_back
#endif

        this->memory.to_slice().move_memory_up(this->size - (p_index + p_elements_0_size), p_index + p_elements_0_size, p_elements_0_size);
        this->size -= p_elements_0_size;
    };

    void pop_back_element()
    {
#if __DEBUG
        assert_true(this->size > 0);
#endif
        this->size -= 1;
    };

    void pop_back_array(uimax p_elements_0_size)
    {
#if __DEBUG
        assert_true(this->size >= p_elements_0_size);
#endif
        this->size -= p_elements_0_size;
    };

    void erase_element_at_always(uimax p_index)
    {
        if (p_index == this->size - 1)
        {
            return this->pop_back_element();
        }
        else
        {
            return this->erase_element_at(p_index);
        }
    };
};

#endif

#ifndef HEAP_VECTOR_H
#define HEAP_VECTOR_H

/*
    A iVaryingVector is a Vector which elements can have different sizes.
    Elements are accessed via the chunks lookup table.
    Memory is continuous.
    Memory operations on nested elements are done by calling _element_XXX functions.
*/
template <class _Allocator> struct iVaryingVector
{
    iVector<int8, _Allocator> memory;
    iVector<SliceIndex, _Allocator> chunks;

    void build(iVector<int8, _Allocator>* p_memory, iVector<SliceIndex, _Allocator>* p_chunks)
    {
        this->chunks = *p_chunks;
        this->memory = *p_memory;
    };

    void allocate(uimax p_memory_array_initial_capacity, uimax p_chunk_array_initial_capacity, _Allocator* p_allocator)
    {
        this->memory.allocate(p_memory_array_initial_capacity, p_allocator);
        this->chunks.allocate(p_chunk_array_initial_capacity, p_allocator);
    };

    void allocate_default(_Allocator* p_allocator)
    {
        this->allocate(0, 0, p_allocator);
    };

    void free()
    {
        this->memory.free();
        this->chunks.free();
    };

    uimax get_size()
    {
        return this->chunks.size;
    };

    Slice<int8> get(uimax p_index)
    {
        SliceIndex* l_chunk = this->chunks.get(p_index);
        return Slice<int8>::build_memory_offset_elementnb(this->memory.get_memory(), l_chunk->begin, l_chunk->size);
    };

    template <class ElementType> Slice<ElementType> get_casted(uimax p_index)
    {
        return this->get(p_index).template cast<ElementType>();
    };

    void push_back_element(Slice<int8>* p_bytes)
    {
        SliceIndex l_chunk = SliceIndex::build(this->memory.size, p_bytes->size);
        this->memory.push_back_array(p_bytes);
        this->chunks.push_back_element(&l_chunk);
    };

    void push_back_element_0v(Slice<int8> p_bytes)
    {
        return this->push_back_element(&p_bytes);
    };

    void push_back_element_empty(uimax p_size)
    {
        SliceIndex l_chunk = SliceIndex::build(this->memory.size, p_size);
        this->memory.push_back_array_empty(p_size);
        this->chunks.push_back_element(&l_chunk);
    };

    void pop_back()
    {
        this->memory.pop_back_array(this->chunks.get(this->chunks.size - 1)->size);
        this->chunks.pop_back_element();
    };

    void erase_element_at(uimax p_index)
    {
        SliceIndex* l_chunk = this->chunks.get(p_index);
        this->memory.erase_array_at(l_chunk->begin, l_chunk->size);

        for (loop(i, p_index, this->chunks.size))
        {
            this->chunks.get(i)->begin -= l_chunk->size;
        };
        this->chunks.erase_element_at(p_index);
    };

    void erase_element_at_always(uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < this->get_size());
#endif

        if (p_index == this->get_size() - 1)
        {
            this->pop_back();
        }
        else
        {
            this->erase_element_at(p_index);
        }
    };

    void erase_array_at(uimax p_index, uimax p_element_nb)
    {
        SliceIndex l_removed_chunk = SliceIndex::build_default();

        for (loop(i, p_index, p_index + p_element_nb))
        {
            l_removed_chunk.size += this->chunks.get(i)->size;
        };

        SliceIndex* l_first_chunk = this->chunks.get(p_index);
        l_removed_chunk.begin = l_first_chunk->begin;

        this->memory.erase_array_at(l_removed_chunk.begin, l_removed_chunk.size);

        for (loop(i, p_index + p_element_nb, this->chunks.size))
        {
            this->chunks.get(i)->begin -= l_removed_chunk.size;
        };

        this->chunks.erase_array_at(p_index, p_element_nb);
    };

    /*
        Expand the size of the chunk by pushing the p_pushed_element Slice value.
    */
    void element_expand_with_value(uimax p_index, Slice<int8>* p_pushed_element)
    {
        SliceIndex* l_updated_chunk = this->chunks.get(p_index);

#if __DEBUG
        assert_true(p_pushed_element->size != 0);
#endif

        uimax l_size_delta = p_pushed_element->size;

        this->memory.insert_array_at_always(p_pushed_element, l_updated_chunk->begin + l_updated_chunk->size);
        l_updated_chunk->size += l_size_delta;

        for (loop(i, p_index + 1, this->chunks.size))
        {
            this->chunks.get(i)->begin += l_size_delta;
        }
    };

    void element_expand_with_value_2v(uimax p_index, Slice<int8> p_pushed_element)
    {
        this->element_expand_with_value(p_index, &p_pushed_element);
    };

    /*
        Expand the size of the VaryingVector chunk by pushing zeroed values of p_delta size.
    */
    void element_expand_delta(uimax p_index, uimax p_delta)
    {
        // !!\ We push an empty slice of the size of delta at the end of the p_index chunk
        this->element_expand_with_value_2v(p_index, Slice<int8>::build((int8*)this, p_delta));
    };

    void element_expand(uimax p_index, uimax p_new_size)
    {
#if __DEBUG
        assert_true(p_new_size > 0);
#endif

        SliceIndex* l_updated_chunk = this->chunks.get(p_index);

#if __DEBUG
        assert_true(p_new_size > l_updated_chunk->size);
#endif

        this->element_expand_delta(p_index, p_new_size - l_updated_chunk->size);
    };

    void element_shrink_delta(uimax p_index, uimax p_size_delta)
    {
        SliceIndex* l_updated_chunk = this->chunks.get(p_index);

#if __DEBUG
        assert_true(p_size_delta != 0);
        assert_true(p_size_delta <= l_updated_chunk->size);
#endif

        this->memory.erase_array_at(l_updated_chunk->begin + l_updated_chunk->size - p_size_delta, p_size_delta);
        l_updated_chunk->size -= p_size_delta;

        for (loop(i, p_index + 1, this->chunks.size))
        {
            this->chunks.get(i)->begin -= p_size_delta;
        }
    };
};

template <class _Allocator> struct iHeapVector
{
    using token_value_type = uimax;

#if __MEMLEAK
    Memleak* memleak;
#endif

    iVaryingVector<_Allocator> varying_vector;

    void allocate_default(_Allocator* p_allocator)
    {
        this->varying_vector.allocate_default(p_allocator);

#if __MEMLEAK
        this->memleak = (Memleak*)_g_heap.malloc(sizeof(Memleak));
        *this->memleak = (*this->memleak).allocate();
#endif
    };

    void free()
    {
        this->varying_vector.free();

#if __MEMLEAK
        this->memleak->check();
        this->memleak->free();
        _g_heap.free((int8*)this->memleak);
        this->memleak = 0;
#endif
    };

    token_value_type malloc(uimax p_size)
    {
        this->varying_vector.push_back_element_empty(p_size);
        token_value_type l_memory = this->varying_vector.get_size() - 1;

#if __MEMLEAK
        this->memleak->push_ptr_to_tracked((int8*)l_memory);
#endif

        return l_memory;
    };

    int8 realloc(token_value_type p_memory, uimax p_size, token_value_type* out_token)
    {
        this->varying_vector.element_expand(p_memory, p_size);
        *out_token = p_memory;
        return 1;
    };

    void free(token_value_type p_memory)
    {
        this->varying_vector.erase_element_at_always(p_memory);

#if __MEMLEAK
        this->memleak->remove_ptr_to_tracked((int8*)p_memory);
#endif
    };

    template <class ElementType> ElementType* get_memory(token_value_type p_memory)
    {
        // return this->varying_vector.get(p_memory, this->heap).template cast<ElementType>().memory;

        // Cast is unsafe here because the varying_vector slice size doesn't always a number of ElementType.
        // This is because the varying_vector memory has it's own growth.
        return (ElementType*)this->varying_vector.get(p_memory).memory;
    };
};

#endif

template <class ElementType> using Span = iSpan<ElementType, GlobalHeap>;
template <class ElementType> using Vector = iVector<ElementType, GlobalHeap>;
template <class ElementType, class _Heap> using iVectorNested = iVector<ElementType, iHeapVector<_Heap>>;
template <class ElementType, class _Heap> using iVectorNested1 = iVectorNested<iVectorNested<ElementType, _Heap>, _Heap>;

using VaryingVector = iVaryingVector<GlobalHeap>;
template <class ElementType> using VectorSlice = iVector<ElementType, GlobalHeapNoAlloc>;
using HeapVector = iHeapVector<GlobalHeap>;
template <class ElementType> using VectorNested = iVectorNested<ElementType, GlobalHeap>;
template <class ElementType> using VectorNested1 = iVectorNested1<ElementType, GlobalHeap>;
