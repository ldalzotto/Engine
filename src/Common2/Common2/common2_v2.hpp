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
#endif

#ifndef MEMORY_H
#define MEMORY_H

using token_t = uimax;

template <class _Heap, class _Heap_Token> struct iHeap
{
    _Heap* heap;
    _Heap_Token malloc(size_t p_size)
    {
        return (_Heap_Token)this->heap->malloc(p_size);
    };

    void free(_Heap_Token p_memory)
    {
        this->heap->free(p_memory);
    };

    int8 realloc(_Heap_Token p_memory, size_t p_new_size, _Heap_Token* out_memory)
    {
        return this->heap->realloc(p_memory, p_new_size, out_memory);
    };

    template <class ElementType> ElementType* get_memory(_Heap_Token p_memory)
    {
        return this->heap->template get_memory<ElementType>(p_memory);
    };
};

#define MEM_LEAK_MAX_POINTER_COUNTER 10000
#define MEM_LEAK_MAX_BACKTRACE 64

typedef int8* ptr_counter_t[MEM_LEAK_MAX_POINTER_COUNTER];
typedef void* backtrace_t[MEM_LEAK_MAX_BACKTRACE];

// #include "dbghelp.h"

using GlobalHeapToken = int8*;

struct GlobalHeap
{

    // Not using mutex but a hash based nested table ?
    // -> NO, because some resource may be allocated from one thread and dealocated a different one
    Mutex<ptr_counter_t> ptr_counter = Mutex<ptr_counter_t>::allocate();
    backtrace_t backtraces[MEM_LEAK_MAX_POINTER_COUNTER] = {};

    GlobalHeapToken malloc(size_t p_size)
    {
#if __MEMLEAK
        return push_ptr_to_tracked((int8*)::malloc(p_size));
#else
        return (int8*)::malloc(p_size);
#endif
    };
    void free(GlobalHeapToken p_memory)
    {
#if __MEMLEAK
        remove_ptr_to_tracked(p_memory);
#endif
        ::free(p_memory);
    };

    GlobalHeapToken calloc(uimax p_size)
    {
#if __MEMLEAK
        return (GlobalHeapToken)push_ptr_to_tracked((int8*)::calloc(1, p_size));
#else
        return (GlobalHeapToken)::calloc(1, p_size);
#endif
    };

    int8 realloc(GlobalHeapToken p_memory, uimax p_new_size, GlobalHeapToken* out_token)
    {
#if __MEMLEAK
        remove_ptr_to_tracked(p_memory);
#endif
        int8* l_new_memory = (int8*)::realloc(p_memory, p_new_size);
#if __MEMLEAK
        if (l_new_memory != NULL)
        {
            push_ptr_to_tracked(l_new_memory);
        }
#endif
        *out_token = l_new_memory;
        return l_new_memory != NULL;
    };

    template <class ElementType> ElementType* get_memory(GlobalHeapToken p_memory)
    {
        return (ElementType*)p_memory;
    };

    int8* push_ptr_to_tracked(int8* p_ptr)
    {
        int8* l_return = NULL;
        ptr_counter.acquire([&](ptr_counter_t p_ptr_counter) {
            for (uimax i = 0; i < MEM_LEAK_MAX_POINTER_COUNTER; i++)
            {
                if (p_ptr_counter[i] == NULL)
                {
                    p_ptr_counter[i] = p_ptr;
                    backtrace_capture(backtraces[i], MEM_LEAK_MAX_BACKTRACE);
                    l_return = p_ptr;
                    break;
                }
            }
        });

        if (l_return)
        {
            return l_return;
        }

        abort();
    };

    void remove_ptr_to_tracked(int8* p_ptr)
    {
        int8 l_successful = 0;
        ptr_counter.acquire([&](ptr_counter_t p_ptr_counter) {
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

    void memleak_ckeck()
    {
#if __MEMLEAK
        ptr_counter.acquire([&](ptr_counter_t p_ptr_counter) {
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

#define TGlobalHeap GlobalHeap, GlobalHeapToken

GlobalHeap _g_heap;
iHeap<TGlobalHeap> gheap = iHeap<TGlobalHeap>{&_g_heap};

struct GlobalHeapNoAlloc
{
    GlobalHeapToken malloc(uimax p_size)
    {
        abort();
    };

    void free(GlobalHeapToken p_memory)
    {
        abort();
    };

    GlobalHeapToken calloc(uimax p_size)
    {
        abort();
    };

    int8 realloc(GlobalHeapToken p_memory, uimax p_new_size, GlobalHeapToken* out_token)
    {
        abort();
    };

    template <class ElementType> ElementType* get_memory(GlobalHeapToken p_memory)
    {
        return (ElementType*)p_memory;
    };
};

#define TGlobalHeapNoAlloc GlobalHeapNoAlloc, GlobalHeapToken

GlobalHeapNoAlloc _g_heap_noalloc;
iHeap<TGlobalHeapNoAlloc> gheapnoalloc = iHeap<TGlobalHeapNoAlloc>{&_g_heap_noalloc};

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
        if ((this->size % sizeof(CastedElementType)) != 0)
        {
            abort();
        }
#endif

        return build(this->memory, (uimax)(this->size / sizeof(CastedElementType)), sizeof(CastedElementType));
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

#undef iHeap_s
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

#endif

#ifndef SPAN_H
#define SPAN_H

template <class ElementType, class _Heap, class _Heap_Token> struct iSpan
{
#define iHeap_s iHeap<_Heap, _Heap_Token>
    uimax size;
    _Heap_Token memory;

    static iSpan build(_Heap_Token p_memory, uimax p_size)
    {
        iSpan l_iSpan;
        l_iSpan.memory = p_memory;
        l_iSpan.size = p_size;
        return l_iSpan;
    };

    static iSpan allocate(uimax p_capacity, iHeap_s p_heap)
    {
        return build(p_heap.malloc(p_capacity * sizeof(ElementType)), p_capacity);
    };

    static iSpan allocate_slice(Slice<ElementType>* p_elements, iHeap_s p_heap)
    {
        iSpan l_iSpan = allocate(p_elements->size, p_heap);
        l_iSpan.to_slice(p_heap).copy_memory(p_elements);
        return l_iSpan;
    };

    static iSpan allocate_slice_0v(Slice<ElementType> p_elements, iHeap_s p_heap)
    {
        return allocate_slice(&p_elements, p_heap);
    };

    void free(iHeap_s p_heap)
    {
        p_heap.free(this->memory);
        this->memory = 0;
        this->size = 0;
    };

    Slice<ElementType> to_slice(iHeap_s p_heap)
    {
        return Slice<ElementType>::build(get_memory(p_heap), this->size);
    };

    ElementType* get(uimax p_index, iHeap_s p_heap)
    {
        return this->to_slice(p_heap).get(p_index);
    };

    int8 resize(uimax p_new_capacity, iHeap_s p_heap)
    {
        if (p_new_capacity > this->size)
        {
            _Heap_Token l_newMemory;
            if (p_heap.realloc(this->memory, p_new_capacity * sizeof(ElementType), &l_newMemory))
            {
                *this = build(l_newMemory, p_new_capacity);
                return 1;
            }
            return 0;
        }
        return 1;
    };

    ElementType* get_memory(iHeap_s p_heap)
    {
        return p_heap.template get_memory<ElementType>(this->memory);
    };
#undef iHeap_s
};

#endif

#ifndef VECTOR_H
#define VECTOR_H

template <class ElementType, class _Heap, class _Heap_Token> struct iVector
{
#define iHeap_s iHeap<_Heap, _Heap_Token>
    iSpan<ElementType, _Heap, _Heap_Token> memory;
    uimax size;

    static iVector allocate(uimax p_initial_capacity, iHeap_s p_heap)
    {
        iVector l_return;
        l_return.size = 0;
        l_return.memory = iSpan<ElementType, _Heap, _Heap_Token>::allocate(p_initial_capacity, p_heap);
        return l_return;
    };

    static iVector build_zero_size(_Heap_Token p_memory, uimax p_initial_capacity)
    {
        iVector l_return;
        l_return.size = 0;
        l_return.memory = iSpan<ElementType, _Heap, _Heap_Token>::build(p_memory, p_initial_capacity);
        return l_return;
    };

    static iVector build_memory(_Heap_Token p_memory, uimax p_memory_size, uimax p_initial_size)
    {
        iVector l_return;
        l_return.size = p_initial_size;
        l_return.memory = iSpan<ElementType, _Heap, _Heap_Token>::build(p_memory, p_memory_size);
        return l_return;
    };

    void free(iHeap_s p_heap)
    {
        this->memory.free(p_heap);
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

    Slice<ElementType> to_slice(iHeap_s p_heap)
    {
        return Slice<ElementType>::build_memory_elementnb(get_memory(p_heap), this->size);
    };

    ElementType* get_memory(iHeap_s p_heap)
    {
        return this->memory.get_memory(p_heap);
    };

    ElementType* get(uimax p_index, iHeap_s p_heap)
    {
        return this->to_slice(p_heap).get(p_index);
    };

#if 0
    void realloc(uimax p_new_capacity, iHeap_s p_heap)
    {
        this->memory.resize(l_new_capacity, p_heap);
    };
#endif

    void push_back_element_empty(iHeap_s p_heap)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + 1);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }
        this->size += 1;
    };

    void push_back_element(ElementType* p_element, iHeap_s p_heap)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + 1);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }
        *this->memory.get(this->size, p_heap) = *p_element;
        this->size += 1;
    };

    void push_back_element_0v(ElementType p_element, iHeap_s p_heap)
    {
        this->push_back_element(&p_element, p_heap);
    };

    void insert_element_at(ElementType* p_element, uimax p_index, iHeap_s p_heap)
    {
#if __DEBUG
        assert_true(p_index < this->size);
        assert_true(p_index != (this->size - 1)); // cannot insert at head. Use iVector_insert_element_at_always instead.
#endif
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + 1);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }

        Slice<ElementType> l_slice = this->memory.to_slice(p_heap);
        l_slice.move_memory_down(this->size - p_index, p_index, 1);
        Slice<ElementType> l_target = Slice<ElementType>::build(p_element, 1);
        l_slice.copy_memory_at_index(p_index, &l_target);

        this->size += 1;
    };

    void insert_element_at_0v(ElementType p_element, uimax p_index, iHeap_s p_heap)
    {
        this->insert_element_at(&p_element, p_index, p_heap);
    };

    void push_back_array(Slice<ElementType>* p_elements_0, iHeap_s p_heap)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }

        this->memory.to_slice(p_heap).copy_memory_at_index(this->size, p_elements_0);
        this->size += p_elements_0->size;
    };

    void push_back_array_empty(uimax p_elements_0_size, iHeap_s p_heap)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0_size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }

        this->size += p_elements_0_size;
    };

    void push_back_array_2(Slice<ElementType>* p_elements_0, Slice<ElementType>* p_elements_1, iHeap_s p_heap)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size + p_elements_1->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }

        this->memory.to_slice(p_heap).copy_memory_at_index_2(this->size, p_elements_0, p_elements_1);
        this->size += (p_elements_0->size + p_elements_1->size);
    };

    void push_back_array_3(Slice<ElementType>* p_elements_0, Slice<ElementType>* p_elements_1, Slice<ElementType>* p_elements_2, iHeap_s p_heap)
    {
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size + p_elements_1->size + p_elements_2->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }

        this->memory.to_slice(p_heap).copy_memory_at_index_3(this->size, p_elements_0, p_elements_1, p_elements_2);
        this->size += (p_elements_0->size + p_elements_1->size + p_elements_2->size);
    };

    void insert_array_at(Slice<ElementType>* p_elements_0, uimax p_index, iHeap_s p_heap)
    {
#if __DEBUG
        assert_true(p_index < this->size);
        assert_true(p_index != (this->size - 1)); // cannot insert at head. Use insert_array_at_always instead.
#endif
        uimax l_new_capacity = memory_resize_until_capacity_met_get_new_capacity(this->memory.size, this->size + p_elements_0->size);
        if (l_new_capacity != 0)
        {
            this->memory.resize(l_new_capacity, p_heap);
        }

        Slice<ElementType> l_slice = this->memory.to_slice(p_heap);
        l_slice.move_memory_down(this->size - p_index, p_index, p_elements_0->size);
        l_slice.copy_memory_at_index(p_index, p_elements_0);
        this->size += p_elements_0->size;
    };

    void erase_element_at(uimax p_index, iHeap_s p_heap)
    {
#if __DEBUG
        assert_true(p_index < this->size);
        assert_true(p_index != this->size - 1); // use pop_back
#endif

        this->memory.to_slice(p_heap).move_memory_up(this->size - (p_index + 1), p_index + 1, 1);
        this->size -= 1;
    };

    void erase_array_at(uimax p_index, uimax p_elements_0_size, iHeap_s p_heap)
    {
#if __DEBUG
        assert_true((p_index + p_elements_0_size) < this->size);
        assert_true(p_index != this->size - 1); // use pop_back
#endif

        this->memory.to_slice(p_heap).move_memory_up(this->size - (p_index + p_elements_0_size), p_index + p_elements_0_size, p_elements_0_size);
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

    void erase_element_at_always(uimax p_index, iHeap_s p_heap)
    {
        if (p_index == this->size - 1)
        {
            return this->pop_back_element();
        }
        else
        {
            return this->erase_element_at(p_index, p_heap);
        }
    };

#undef iHeap_s
};

#endif

// TODO -> re enable ?
#if 0
#ifndef FUNCIONAL_H
#define FUNCIONAL_H

template <class ElementType, class Accessor, class _Heap, class _Heap_Token> struct iAccessor
{
    Accessor accessor;

    uimax get_size()
    {
        return this->container.get_size();
    };

    ElementType* get(uimax p_index, iHeap<_Heap, _Heap_Token> p_heap)
    {
        this->accessor->get(p_index, p_heap);
    };
};

template <class ElementType, class Remover, class _Heap, class _Heap_Token> struct iRemover
{
    Remover remover;
};

#endif

#endif

template <class ElementType> using Span = iSpan<ElementType, TGlobalHeap>;
template <class ElementType> using Vector = iVector<ElementType, TGlobalHeap>;

template <class ElementType> using VectorSlice = iVector<ElementType, TGlobalHeapNoAlloc>;