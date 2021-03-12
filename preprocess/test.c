

// clang-format off
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "sqlite3.h"
    // clang-format on

    typedef char int8;
typedef unsigned char uint8;

typedef short int16;
typedef unsigned short uint16;

typedef int int32;
typedef unsigned int uint32;

typedef long long int64;
typedef unsigned long long uint64;

typedef float float32;
typedef double float64;
typedef long double float128;

typedef size_t uimax;

const uint8 uint8_max = UCHAR_MAX;
const uint16 uint16_max = USHRT_MAX;

// clang-format off
#include <Windows.h>
#include <sysinfoapi.h>
    // clang-format on

    // clang-format off
#include "dbghelp.h"
    // clang-format on

    inline uimax
    dword_lowhigh_to_uimax(const DWORD p_low, const DWORD p_high)
{
    ULARGE_INTEGER ul;
    ul.LowPart = p_low;
    ul.HighPart = p_high;
    return ul.QuadPart;
};

inline uint64 FILETIME_to_mics(const FILETIME* p_filetime)
{
    return dword_lowhigh_to_uimax(p_filetime->dwLowDateTime, p_filetime->dwHighDateTime) / 10;
};

typedef struct Clock {
    uimax framecount;
    float32 deltatime;
} Clock;

inline static Clock Clock_build(const uimax p_framecount, const float32 p_deltatime)
{

    return (Clock) { .framecount = p_framecount, .deltatime = p_deltatime };
};

inline static Clock Clock_build_default()
{
    return Clock_build(0, 0.0f);
};

inline void Clock_newframe(Clock* thiz)
{
    thiz->framecount += 1;
}

inline void Clock_newupdate(Clock* thiz, float32 p_delta)
{
    thiz->deltatime = p_delta;
}

time_t clock_currenttime_mics();

inline time_t clock_currenttime_mics()
{
    FILETIME l_currentTime;
    GetSystemTimeAsFileTime(&l_currentTime);
    return FILETIME_to_mics(&l_currentTime);
};

typedef

    HANDLE

        thread_t;

static thread_t Thread_get_current_thread();
static void Thread_wait(const uimax p_time_in_ms);

inline thread_t Thread_get_current_thread()
{
    return GetCurrentThread();
};

inline void Thread_wait(const uimax p_time_in_ms)
{
    WaitForSingleObject(Thread_get_current_thread(), (DWORD)p_time_in_ms);
};

inline void assert_true(int8 p_condition)
{
    if (!p_condition) {
        abort();
    }
};

typedef uimax token_t;

inline token_t tokent_build_default()
{
    return -1;
};

inline void tokent_reset(token_t* p_token)
{
    *p_token = tokent_build_default();
};

inline int8 tokent_is_valid(const token_t* p_token)
{
    return *p_token != (token_t)-1;
};

typedef struct Token_ {
    token_t tok;
} Token_;

int8* ptr_counter[10000] = { 0 };

typedef LPVOID backtrace_t[64];

backtrace_t backtraces[10000] = { 0 };

inline void capture_backtrace(const uimax p_ptr_index)
{
    CaptureStackBackTrace(0, 64, backtraces[p_ptr_index], NULL);
};

inline int8* push_ptr_to_tracked(int8* p_ptr)
{
    for (uimax i = 0; i < 10000; i++) {
        if (ptr_counter[i] == NULL) {
            ptr_counter[i] = p_ptr;
            capture_backtrace(i);
            return p_ptr;
        }
    }

    abort();
};

inline void remove_ptr_to_tracked(int8* p_ptr)
{
    for (uimax i = 0; i < 10000; i++) {
        if (ptr_counter[i] == p_ptr) {
            ptr_counter[i] = NULL;
            return;
        }
    }

    abort();
};

inline void memleak_ckeck()
{

    for (uimax i = 0; i < 10000; i++) {
        if (ptr_counter[i] != NULL) {
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
};

inline int8* heap_malloc(const uimax p_size)
{

    return push_ptr_to_tracked((int8*)malloc(p_size));
};

inline int8* heap_calloc(const uimax p_size)
{

    return push_ptr_to_tracked((int8*)calloc(1, p_size));
};

inline int8* heap_realloc(int8* p_memory, const uimax p_new_size)
{

    remove_ptr_to_tracked(p_memory);
    return push_ptr_to_tracked((int8*)realloc(p_memory, p_new_size));
};

inline void heap_free(int8* p_memory)
{

    remove_ptr_to_tracked(p_memory);

    free(p_memory);
};

inline int8* memory_cpy(int8* p_dst, const int8* p_src, const uimax p_size)
{
    return (int8*)memcpy(p_dst, p_src, p_size);
};

inline static int8* memory_cpy_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{

    if (p_size > p_dst_size) {
        abort();
    }

    return memory_cpy(p_dst, p_src, p_size);
};

inline void memory_zero(int8* p_dst, const uimax p_size)
{
    memset(p_dst, 0, p_size);
};

inline void memory_zero_safe(int8* p_dst, const uimax p_dst_size, const uimax p_size)
{

    if (p_size > p_dst_size) {
        abort();
    }

    memory_zero(p_dst, p_size);
};

inline int8* memory_move(int8* p_dst, const int8* p_src, const uimax p_size)
{
    return (int8*)memmove(p_dst, p_src, p_size);
};

inline int8* memory_move_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{

    if (p_size > p_dst_size) {
        abort();
    }

    return memory_move(p_dst, p_src, p_size);
};

inline int8 memory_compare(const int8* p_source, const int8* p_compared, const uimax p_size)
{
    return memcmp(p_source, p_compared, p_size) == 0;
};

/*
    A Slice is an encapsulated C style array.
*/
typedef struct s_Slice_ {
    uimax Size;
    int8* Begin;
} Slice_;

inline Slice_ Slice__build(const uimax p_size, int8* p_begin)
{

    return (Slice_) { p_size, p_begin };
};

inline Slice_ Slice__build_default()
{
    return Slice__build(0, NULL);
};

inline Slice_ Slice__build_begin_end(int8* p_memory, const uimax t_memory, const uimax p_begin, const uimax p_end)
{
    return Slice__build(p_end - p_begin, p_memory + (p_begin * t_memory));
};

inline Slice_ Slice__build_memory_offset_elementnb(int8* p_memory, const uimax t_memory, const uimax p_offset, const uimax p_element_nb)
{
    return Slice__build(p_element_nb, p_memory + (p_offset * t_memory));
};

inline Slice_ Slice__build_asint8(Slice_* thiz, const uimax t_element_type)
{
    return Slice__build(t_element_type * thiz->Size, (int8*)thiz->Begin);
};

inline Slice_ Slice__build_asint8_begin_end(int8* p_memory, const uimax t_memory, const uimax p_begin, const uimax p_end)
{
    return Slice__build(t_memory * (p_end - p_begin), p_memory + (p_begin * t_memory));
};

inline Slice_ Slice__build_asint8_memory_elementnb(const int8* p_memory, const uimax t_memory, const uimax p_element_nb)
{
    return Slice__build(t_memory * p_element_nb, (int8*)p_memory);
};

inline Slice_ Slice__build_asint8_memory_singleelement(const int8* p_memory, const uimax t_memory)
{
    return Slice__build(t_memory, ((int8*)(p_memory)));
};

inline Slice_ Slice__build_rawstr(const int8* p_str)
{
    return Slice__build(strlen(p_str), (int8*)p_str);
};

inline Slice_ Slice__build_rawstr_with_null_termination(const int8* p_str)
{
    return Slice__build(strlen(p_str) + 1, (int8*)p_str);
};

inline void Slice__bound_inside_check(const Slice_* thiz, const uimax t_thiz_elementtype, const Slice_* p_tested_slice)
{

    if ((p_tested_slice->Begin + (p_tested_slice->Size * t_thiz_elementtype)) > (thiz->Begin + (thiz->Size * t_thiz_elementtype))) {
        abort();
    }
};

inline void Slice__bound_check(const Slice_* thiz, const uimax p_index)
{

    if (p_index > thiz->Size) {
        abort();
    }
};

inline int8* Slice__get(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{

    Slice__bound_check(thiz, p_index);

    return thiz->Begin + (p_index * t_thiz_elementtype);
};

inline void Slice__slide(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_offset_index)
{

    Slice__bound_check(thiz, p_offset_index);

    thiz->Begin = thiz->Begin + (p_offset_index * t_thiz_elementtype);
    thiz->Size -= p_offset_index;
};

inline Slice_ Slice__slide_rv(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_offset_index)
{
    Slice_ l_return = *thiz;
    Slice__slide(&l_return, t_thiz_elementtype, p_offset_index);
    return l_return;
};

inline int8* Slice__memcpy(const Slice_* p_target, const Slice_* p_source, const uimax t_elementtype)
{

    return memory_cpy_safe(p_target->Begin, p_target->Size * t_elementtype, p_source->Begin, p_source->Size * t_elementtype);
};

inline void Slice__zero(const Slice_* p_target, const uimax t_elementtype)
{

    memory_zero_safe(p_target->Begin, p_target->Size * t_elementtype, p_target->Size * t_elementtype);
};

inline int8* Slice__memmove(const Slice_* p_target, const Slice_* p_source, const uimax t_elementtype)
{

    return memory_move_safe(p_target->Begin, p_target->Size * t_elementtype, p_source->Begin, p_source->Size * t_elementtype);
};

inline int8 Slice__memcompare(const Slice_* p_target, const Slice_* p_compared, const uimax t_elementtype)
{
    return memory_compare(p_target->Begin, p_compared->Begin, p_compared->Size * t_elementtype);
};

inline int8 Slice__memfind(const Slice_* p_target, const Slice_* p_compared, const uimax t_elementtype, uimax* out_index)
{

    if (p_compared->Size > p_target->Size) {
        abort();
    }

    Slice_ l_target_slice = *p_target;
    if (Slice__memcompare(&l_target_slice, p_compared, t_elementtype)) {
        *out_index = 0;
        return 1;
    };

    for (uimax i = 1; i < p_target->Size - p_compared->Size + 1; i++) {
        Slice__slide(&l_target_slice, t_elementtype, 1);
        if (Slice__memcompare(&l_target_slice, p_compared, t_elementtype)) {
            *out_index = i;
            return 1;
        };
    };

    *out_index = -1;
    return 0;
};

inline void Slice__move_memory_down(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice_ l_target = Slice__build_memory_offset_elementnb(thiz->Begin, t_thiz_elementtype, p_break_index + p_move_delta, p_moved_block_size);

    Slice__bound_inside_check(thiz, t_thiz_elementtype, &l_target);

    Slice_ l_source = Slice__build_begin_end(thiz->Begin, t_thiz_elementtype, p_break_index, p_break_index + p_moved_block_size);
    Slice__memmove(&l_target, &l_source, t_thiz_elementtype);
};

inline void Slice__move_memory_up(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice_ l_target = Slice__build_memory_offset_elementnb(thiz->Begin, t_thiz_elementtype, p_break_index - p_move_delta, p_moved_block_size);

    Slice__bound_inside_check(thiz, t_thiz_elementtype, &l_target);

    Slice_ l_source = Slice__build_begin_end(thiz->Begin, t_thiz_elementtype, p_break_index, p_break_index + p_moved_block_size);
    Slice__memmove(&l_target, &l_source, t_thiz_elementtype);
};

inline void Slice__copy_memory(const Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_copy_index, const Slice_* p_elements)
{
    Slice_ l_target = Slice__build(p_elements->Size, thiz->Begin + (p_copy_index * t_thiz_elementtype));

    Slice__bound_inside_check(thiz, t_thiz_elementtype, &l_target);

    Slice__memcpy(&l_target, p_elements, t_thiz_elementtype);
};

inline void Slice__copy_memory_2(const Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_copy_index, const Slice_* p_elements_1, const Slice_* p_elements_2)
{
    Slice__copy_memory(thiz, t_thiz_elementtype, p_copy_index, p_elements_1);
    Slice__copy_memory(thiz, t_thiz_elementtype, p_copy_index + p_elements_1->Size, p_elements_2);
};

inline void Slice__copy_memory_3(const Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_copy_index, const Slice_* p_elements_1, const Slice_* p_elements_2, const Slice_* p_elements_3)
{
    Slice__copy_memory_2(thiz, t_thiz_elementtype, p_copy_index, p_elements_1, p_elements_2);
    Slice__copy_memory(thiz, t_thiz_elementtype, p_copy_index + p_elements_1->Size + p_elements_2->Size, p_elements_3);
};

inline Slice_ Slice__cast(const Slice_* p_slice, const uimax t_slice_casted_elementtype)
{

    if ((p_slice->Size % t_slice_casted_elementtype) != 0) {
        abort();
    }

    return Slice__build(p_slice->Size / t_slice_casted_elementtype, p_slice->Begin);
};

inline int8* Slice__cast_singleelement(const Slice_* p_slice, const uimax t_casted_elementtype)
{

    if (p_slice->Size < t_casted_elementtype) {
        abort();
    }

    return p_slice->Begin;
};

inline void Slice__assert_null_terminated(const Slice_* thiz, const uimax t_thiz_elementtype)
{

    assert_true(t_thiz_elementtype == sizeof(int8));
    assert_true(*Slice__get((Slice_*)thiz, t_thiz_elementtype, thiz->Size - 1) == '\0');
};
// TODO -> add methods
// inline Slice_ Slice__build_default() inline Slice_ Slice__build_begin_end(int8* p_memory, const uimax t_memory, const uimax p_begin, const uimax p_end) inline Slice_
// Slice__build_memory_offset_elementnb(int8* p_memory, const uimax t_memory, const uimax p_offset, const uimax p_element_nb)

inline Slice_ SliceN___to_slice(const int8* thiz, const uimax Size_t)
{
    return Slice__build(Size_t, (int8*)thiz);
};

inline int8* SliceN___get(const int8* thiz, const uimax t_thiz_elementtype, const uimax Size_t, const uimax p_index)
{

    assert_true(p_index < Size_t);

    return (int8*)thiz + (p_index * t_thiz_elementtype);
};
/*
    A SliceIndex is just a begin and end uimax
*/
typedef struct s_SliceIndex {
    uimax Begin;
    uimax Size;
} SliceIndex;

inline SliceIndex SliceIndex_build(const uimax p_begin, const uimax p_size)
{

    return (SliceIndex) { .Begin = p_begin, .Size = p_size };
};

inline SliceIndex SliceIndex_build_default()
{
    return SliceIndex_build(0, 0);
};

inline void SliceIndex_slice_two(const SliceIndex* thiz, const uimax p_break_point, SliceIndex* out_left, SliceIndex* out_right)
{
    uimax l_source_initial_size = thiz->Size;
    *out_left = SliceIndex_build(thiz->Begin, p_break_point - thiz->Begin);
    *out_right = SliceIndex_build(p_break_point, l_source_initial_size - out_left->Size);
};

typedef struct s_SliceOffset_ {
    uimax Offset;
    int8* Memory;
} SliceOffset_;

inline SliceOffset_ SliceOffset__build(int8* p_memory, const uimax p_offset)
{

    return (SliceOffset_) { p_offset, p_memory };
};

inline SliceOffset_ SliceOffset__build_from_sliceindex(int8* p_memory, const SliceIndex* p_slice_index)
{
    return SliceOffset__build(p_memory, p_slice_index->Begin);
};

/*
    A Span is a heap allocated chunk of memory.
    Span can allocate memory, be resized and freed.
*/
typedef struct s_Span_ {
    union {
        struct
        {
            uimax Capacity;
            int8* Memory;
        };
        Slice_ slice;
    };
} Span_;

inline Span_ Span__build(int8* p_memory, const uimax p_capacity)
{

    return (Span_) { p_capacity, p_memory };
};

inline Span_ Span__build_default()
{

    return (Span_) { .slice = Slice__build_default() };
};

inline Span_ Span__allocate(const uimax t_elementtype, const uimax p_capacity)
{
    return Span__build(heap_malloc(p_capacity * t_elementtype), p_capacity);
};

inline void Span__free(Span_* thiz)
{
    heap_free(thiz->Memory);
    *thiz = Span__build_default();
};

inline Span_ Span__allocate_slice(const uimax t_elementtype, const Slice_* p_elements)
{
    Span_ l_span = Span__allocate(t_elementtype, p_elements->Size);
    Slice__memcpy(&l_span.slice, p_elements, t_elementtype);
    return l_span;
};

inline Span_ Span__allocate_slice_2(const uimax t_elementtype, const Slice_* p_elements_1, const Slice_* p_elements_2)
{
    Span_ l_span = Span__allocate(t_elementtype, p_elements_1->Size + p_elements_2->Size);
    Slice__copy_memory_2(&l_span.slice, t_elementtype, 0, p_elements_1, p_elements_2);
    return l_span;
};

inline Span_ Span__allocate_slice_3(const uimax t_elementtype, const Slice_* p_elements_1, const Slice_* p_elements_2, const Slice_* p_elements_3)
{
    Span_ l_span = Span__allocate(t_elementtype, p_elements_1->Size + p_elements_2->Size + p_elements_3->Size);
    Slice__copy_memory_3(&l_span.slice, t_elementtype, 0, p_elements_1, p_elements_2, p_elements_3);
    return l_span;
};

inline Span_ Span__callocate(const uimax t_elementtype, const uimax p_capacity)
{
    return Span__build(heap_calloc(p_capacity * t_elementtype), p_capacity);
};

inline int8* Span__get(Span_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{

    assert_true(p_index < thiz->Capacity);

    return thiz->Memory + (p_index * t_thiz_elementtype);
};

inline int8 Span__resize(Span_* thiz, const uimax t_thiz_elementtype, const uimax p_new_capacity)
{
    if (p_new_capacity > thiz->Capacity) {
        int8* l_newMemory = heap_realloc(thiz->Memory, p_new_capacity * t_thiz_elementtype);
        if (l_newMemory != NULL) {
            *thiz = Span__build(l_newMemory, p_new_capacity);
            return 1;
        }
        return 0;
    }
    return 1;
};

inline Span_ Span__move(Span_* thiz)
{
    Span_ l_return = *thiz;
    *thiz = Span__build_default();
    return l_return;
};

inline void Span__resize_until_capacity_met(Span_* thiz, const uimax t_thiz_elementtype, const uimax p_desired_capacity)
{
    uimax l_resized_capacity = thiz->Capacity;

    if (l_resized_capacity >= p_desired_capacity) {
        return;
    }

    if (l_resized_capacity == 0) {
        l_resized_capacity = 1;
    }

    while (l_resized_capacity < p_desired_capacity) {
        l_resized_capacity *= 2;
    }

    Span__resize(thiz, t_thiz_elementtype, l_resized_capacity);
};

typedef uimax hash_t;

// http://www.cse.yorku.ca/~oz/hash.html
inline static uimax HashFunctionRawChecked(const int8* p_value, const uimax p_size)
{
    hash_t hash = 5381;

    for (uimax i = 0; i < p_size; i++) {
        int8 c = *(p_value + i);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    // -1 is usually the default value of hash_t, we we prevent in debuf mode any value that can clash
    assert_true(hash != (hash_t)-1);

    return hash;
}

inline hash_t HashSlice_(const Slice_* p_value, const uimax t_value_elementtype)
{
    return HashFunctionRawChecked(p_value->Begin, p_value->Size * t_value_elementtype);
};

inline hash_t HashSlice__0v(const Slice_ p_value, const uimax t_value_elementtype)
{
    return HashSlice_(&p_value, t_value_elementtype);
};

inline hash_t HashRaw(const int8* p_str)
{
    return HashFunctionRawChecked((int8*)p_str, strlen(p_str));
};

// TODO ->Implement Quick sort

/*
    A Vector is a Span with an imaginary boundary (Size).
    Vector memory is continuous, there is no "hole" between items.
    Vector is reallocated on need.
    Any memory access outside of this imaginary boundary will be in error.
    The Vector expose some safe way to insert/erase data (array or single element).
*/
typedef struct s_Vector_ {
    uimax Size;
    Span_ Memory;
} Vector_;

inline Vector_ Vector__build(const uimax p_size, Span_ p_span)
{

    return (Vector_) { .Size = p_size, .Memory = p_span };
};

inline Vector_ Vector__build_default()
{
    return Vector__build(0, Span__build_default());
};

inline Vector_ Vector__build_zero_size(int8* p_memory, const uimax p_initial_capacity)
{
    return Vector__build(0, Span__build(p_memory, p_initial_capacity));
};

inline Vector_ Vector__allocate(const uimax p_initial_capacity, const uimax t_elementtype)
{
    return Vector__build(0, Span__allocate(t_elementtype, p_initial_capacity));
};

inline Vector_ Vector__allocate_slice(const uimax t_elementtype, const Slice_* p_initial_elements)
{
    return Vector__build(0, Span__allocate_slice(t_elementtype, p_initial_elements));
};

inline void Vector__free(Vector_* thiz)
{
    Span__free(&thiz->Memory);
    *thiz = Vector__build_default();
};

inline int8 Vector__empty(Vector_* thiz)
{
    return thiz->Size == 0;
};

inline void Vector__clear(Vector_* thiz)
{
    thiz->Size = 0;
};

inline void Vector__bound_check(Vector_* thiz, const uimax p_index)
{

    if (p_index > thiz->Size) {
        abort();
    }
};

inline void Vector__bound_head_check(Vector_* thiz, const uimax p_index)
{

    if (p_index == thiz->Size) {
        abort();
    }
};

inline void Vector__bound_lastelement_check(Vector_* thiz, const uimax p_index)
{

    if (p_index == thiz->Size - 1) {
        abort();
    }
};

inline int8* Vector__get(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);
    Vector__bound_head_check(thiz, p_index);

    return thiz->Memory.Memory + (p_index * t_thiz_elementtype);
};

inline static void Vector__move_memory_down(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_down(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size - p_break_index, p_break_index, p_move_delta);
};

inline void Vector__move_memory_up(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_up(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size - p_break_index, p_break_index, p_move_delta);
};

inline static int8 Vector__insert_element_at_unchecked(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element, const uimax p_index)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + 1);
    Vector__move_memory_down(thiz, t_thiz_elementtype, p_index, 1);
    memory_cpy(thiz->Memory.Memory + (p_index * t_thiz_elementtype), p_element, t_thiz_elementtype);
    thiz->Size += 1;
    return 1;
};

inline static int8 Vector__insert_array_at_unchecked(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements, const uimax p_index)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_elements->Size);
    Vector__move_memory_down(thiz, t_thiz_elementtype, p_index, p_elements->Size);
    Slice__copy_memory(&thiz->Memory.slice, t_thiz_elementtype, p_index, p_elements);

    thiz->Size += p_elements->Size;

    return 1;
};

inline int8 Vector__insert_array_at(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);
    Vector__bound_head_check(thiz, p_index); // cannot insert at head. Use vector_insert_array_at_always instead.

    return Vector__insert_array_at_unchecked(thiz, t_thiz_elementtype, p_elements, p_index);
};

inline int8 Vector__insert_element_at(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);
    Vector__bound_head_check(thiz, p_index); // cannot insert at head. Use vector_insert_element_at_always instead.

    return Vector__insert_element_at_unchecked(thiz, t_thiz_elementtype, p_element, p_index);
};

inline int8 Vector__push_back_array(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_elements->Size);
    Slice__copy_memory(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size, p_elements);
    thiz->Size += p_elements->Size;

    return 1;
};

inline int8 Vector__push_back_array_2(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements_0, const Slice_* p_elements_1)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_elements_0->Size + p_elements_1->Size);
    Slice__copy_memory_2(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size, p_elements_0, p_elements_1);
    thiz->Size += (p_elements_0->Size + p_elements_1->Size);

    return 1;
};

inline int8 Vector__push_back_array_empty(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_array_size)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_array_size);
    thiz->Size += p_array_size;
    return 1;
};

inline int8 Vector__push_back_element_empty(Vector_* thiz, const uimax t_thiz_elementtype)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + 1);
    thiz->Size += 1;
    return 1;
};

inline int8 Vector__push_back_element(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + 1);
    memory_cpy(thiz->Memory.Memory + (thiz->Size * t_thiz_elementtype), p_element, t_thiz_elementtype);
    thiz->Size += 1;

    return 1;
};

inline int8 Vector__insert_array_at_always(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);

    if (p_index == thiz->Size) {
        return Vector__push_back_array(thiz, t_thiz_elementtype, p_elements);
    } else {
        return Vector__insert_array_at_unchecked(thiz, t_thiz_elementtype, p_elements, p_index);
    }
};

inline int8 Vector__insert_element_at_always(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);

    if (p_index == thiz->Size) {
        return Vector__push_back_element(thiz, t_thiz_elementtype, p_element);
    } else {
        return Vector__insert_element_at_unchecked(thiz, t_thiz_elementtype, p_element, p_index);
    }
};

inline int8 Vector__erase_array_at(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index, const uimax p_element_nb)
{

    Vector__bound_check(thiz, p_index);
    Vector__bound_check(thiz, p_index + p_element_nb);
    Vector__bound_lastelement_check(thiz, p_index); // use vector_pop_back_array

    Vector__move_memory_up(thiz, t_thiz_elementtype, p_index + p_element_nb, p_element_nb);
    thiz->Size -= p_element_nb;

    return 1;
};

inline int8 Vector__erase_element_at(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);
    Vector__bound_lastelement_check(thiz, p_index); // use vector_pop_back

    Vector__move_memory_up(thiz, t_thiz_elementtype, p_index + 1, 1);
    thiz->Size -= 1;

    return 1;
};

inline int8 Vector__pop_back_array(Vector_* thiz, const uimax p_element_nb)
{
    thiz->Size -= p_element_nb;
    return 1;
};

inline int8 Vector__pop_back(Vector_* thiz)
{
    thiz->Size -= 1;
    return 1;
};

inline int8 Vector__erase_element_at_always(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{

    Vector__bound_check(thiz, p_index);

    if (p_index == thiz->Size - 1) {
        return Vector__pop_back(thiz);
    } else {
        return Vector__erase_element_at(thiz, t_thiz_elementtype, p_index);
    }
};

typedef struct s_Slice_uimax {
    union {
        struct {
            uimax Size;
            uimax* Begin;
        };
        Slice_ slice;
    };
} Slice_uimax;
typedef struct s_Span_uimax {
    union {
        struct {
            union {
                struct {
                    uimax Capacity;
                    uimax* Memory;
                };
                Slice_uimax slice;
            };
        };
        Span_ span;
    };
} Span_uimax;
inline Span_uimax Span_uimax_build(uimax* p_memory, const uimax p_capacity) { return (Span_uimax) { .span = Span__build((int8*)p_memory, p_capacity) }; };
inline void Span_uimax_free(Span_uimax* thiz) { Span__free(&thiz->span); };
inline int8 Span_uimax_resize(Span_uimax* thiz, const uimax p_new_capacity) { return Span__resize(&thiz->span, sizeof(uimax), p_new_capacity); };
inline uimax* Span_uimax_get(Span_uimax* thiz, const uimax p_index) { return (uimax*)Span__get(&thiz->span, sizeof(uimax), p_index); };
\;

inline void assert_span_unitialized(Span_* p_span)
{
    assert_true(p_span->Capacity == 0);
    assert_true(p_span->Memory == NULL);
};

inline void slice_span_test()
{
    Span_uimax l_span_sizet = Span_uimax_build(NULL, 0);
    // When resizing the span, new memory is allocated
    {
        uimax l_new_capacity = 10;
        Span_uimax_resize(&l_span_sizet, 10);
        assert_true(l_span_sizet.Capacity == l_new_capacity);
        assert_true(l_span_sizet.Memory != NULL);
    }

    // When freeing the span, it's structure is resetted
    {
        Span_uimax_free(&l_span_sizet);
        assert_span_unitialized(&l_span_sizet.span);
    }

    /*
        // Move memory
        {
            l_span_sizet.resize(10);

            l_span_sizet.get(0) = 3;
            l_span_sizet.get(1) = 5;
            l_span_sizet.get(3) = 10;

            l_span_sizet.slice.move_memory_down(1, 3, 1);
            assert_true(l_span_sizet.get(4) == 10);

            l_span_sizet.slice.move_memory_up(1, 4, 2);
            assert_true(l_span_sizet.get(2) == 10);

            assert_true(l_span_sizet.get(0) == 3);
            assert_true(l_span_sizet.get(1) == 5);

            l_span_sizet.slice.move_memory_up(2, 2, 2);

            assert_true(l_span_sizet.get(0) == 10);
            assert_true(l_span_sizet.get(1) == 10);
        }

        l_span_sizet.free();

        {
            l_span_sizet = Span<uimax>::allocate(10);

            SliceN<uimax, 4> l_slice_1 = {0, 1, 2, 3};
            SliceN<uimax, 4> l_slice_2 = {5, 6, 7, 8};
            l_span_sizet.slice.copy_memory_2(1, l_slice_1.to_slice(), l_slice_2.to_slice());

            assert_true(l_span_sizet.slice.slide_rv(1).compare(l_slice_1.to_slice()));
            assert_true(l_span_sizet.slice.slide_rv(5).compare(l_slice_2.to_slice()));

            l_span_sizet.free();
        }
        {
            SliceN<uimax, 4> l_slice = {15, 26, 78, 10};
            l_span_sizet = Span<uimax>::allocate_slice(l_slice.to_slice());
            assert_true(l_span_sizet.Capacity == 4);
            assert_true(l_span_sizet.slice.compare(l_slice.to_slice()));

            l_span_sizet.free();
        }
         */
};

/*
inline void vector_test()
{
    Vector<uimax> l_vector_sizet = Vector<uimax>::build_zero_size((uimax*)NULL, 0);

    // vector_push_back_array
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build(l_elements, 5);

        l_vector_sizet.push_back_array(l_elements_slice);
        assert_true(l_vector_sizet.Size == l_old_size + 5);
        for (loop(i, l_old_size, l_vector_sizet.Size))
        {
            assert_true(l_vector_sizet.get(i) == l_elements[i - l_old_size]);
        }
    }

    // push_back_array_empty
    {
        uimax l_old_size = l_vector_sizet.Size;
        l_vector_sizet.push_back_array_empty(5);
        assert_true(l_vector_sizet.Size == (l_old_size + (uimax)5));
    }

    // vector_push_back_element
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_element = 25;
        l_vector_sizet.push_back_element(l_element);
        assert_true(l_vector_sizet.Size == l_old_size + 1);
        assert_true(l_vector_sizet.get(l_vector_sizet.Size - 1) == l_element);
    }

    // vector_insert_array_at
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build(l_elements, 5);
        l_vector_sizet.insert_array_at(l_elements_slice, 0);
        assert_true(l_vector_sizet.Size == l_old_size + 5);
        for (loop_int16(i, 0, 5))
        {
            assert_true((l_vector_sizet.get(i)) == (uimax)i);
        }

        l_vector_sizet.insert_array_at(l_elements_slice, 3);
        assert_true(l_vector_sizet.Size == l_old_size + 10);
        for (loop_int16(i, 0, 3))
        {
            assert_true((l_vector_sizet.get(i)) == l_elements[i]);
        }
        // Middle insertion
        for (loop_int16(i, 3, 8))
        {
            assert_true((l_vector_sizet.get(i)) == l_elements[i - cast(uimax, 3)]);
        }
        for (loop_int16(i, 8, 10))
        {
            assert_true((l_vector_sizet.get(i)) == l_elements[i - cast(uimax, 5)]);
        }
    }

    // vector_insert_element_at
    {
        uimax l_element = 20;
        uimax l_old_size = l_vector_sizet.Size;

        l_vector_sizet.insert_element_at(l_element, 7);
        assert_true(l_vector_sizet.get(7) == l_element);
        assert_true(l_vector_sizet.Size == l_old_size + 1);

        l_vector_sizet.insert_element_at(cast(uimax, 20), 9);
    }

    // vector_erase_element_at
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_erase_index = 1;
        uimax l_element_after = l_vector_sizet.get(l_erase_index + 1);
        l_vector_sizet.erase_element_at(1);
        assert_true(l_vector_sizet.Size == l_old_size - 1);
        assert_true(l_vector_sizet.get(1) == l_element_after);
    }

    // vector_erase_array_at
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_erase_begin_index = 3;
        const uimax l_erase_nb = 6;
        const uimax l_old_element_check_nb = 3;

        uimax l_old_values[l_old_element_check_nb];
        for (loop(i, l_erase_begin_index + l_erase_nb, (l_erase_begin_index + l_erase_nb) + l_old_element_check_nb))
        {
            l_old_values[i - (l_erase_begin_index + l_erase_nb)] = l_vector_sizet.get(i);
        }

        l_vector_sizet.erase_array_at(l_erase_begin_index, l_erase_nb);

        assert_true(l_vector_sizet.Size == l_old_size - l_erase_nb);
        for (loop(i, 0, l_old_element_check_nb))
        {
            assert_true(l_vector_sizet.get(l_erase_begin_index + i) == l_old_values[i]);
        }
    }

    // vector_pop_back
    {
        uimax l_old_size = l_vector_sizet.Size;
        l_vector_sizet.pop_back();
        assert_true(l_vector_sizet.Size == l_old_size - 1);
    }

    // vector_pop_back_array
    {
        uimax l_old_size = l_vector_sizet.Size;
        l_vector_sizet.pop_back_array(3);
        assert_true(l_vector_sizet.Size == l_old_size - 3);
    }

    // When freeing the vcetor, it's structure is resetted
    {
        l_vector_sizet.free();
        assert_true(l_vector_sizet.Size == 0);
        assert_span_unitialized(&l_vector_sizet.Memory);
    }
};


*/

inline void Slice_uimax_sort_decroissant(Slice_uimax* p_slice)
{
    for (uimax i = 0; i < (&p_slice->slice)->Size; i++) {
        uimax* l_left = (uimax*)Slice__get((&p_slice->slice), sizeof(uimax), i);
        for (uimax j = i + 1; j < (&p_slice->slice)->Size; j++) {
            uimax* l_right = (uimax*)Slice__get((&p_slice->slice), sizeof(uimax), j);
            int8 l_comparison;
            {
                l_comparison = *l_left <= *l_right;
            };
            if (l_comparison) {
                uimax l_left_tmp = *l_left;
                *l_left = *l_right;
                *l_right = l_left_tmp;
            }
        }
    };
};

int main()
{
    slice_span_test();

    memleak_ckeck();
}
