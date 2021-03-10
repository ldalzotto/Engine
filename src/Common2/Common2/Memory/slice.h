#pragma once

typedef struct s_Slice_
{
    uimax Size;
    int8* Begin;
} Slice_;

inline Slice_ Slice__build(const uimax p_size, int8* p_begin)
{
#if __cplusplus
    return Slice_{p_size, p_begin};
#else
    return (Slice_){p_size, p_begin};
#endif
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
    return Slice__build(t_memory, cast(int8*, p_memory));
};

inline void Slice__bound_inside_check(const Slice_* thiz, const uimax t_thiz_elementtype, const Slice_* p_tested_slice)
{
#if CONTAINER_BOUND_TEST
    if ((p_tested_slice->Begin + (p_tested_slice->Size * t_thiz_elementtype)) > (thiz->Begin + (thiz->Size * t_thiz_elementtype)))
    {
        abort();
    }
#endif
};

inline void Slice__bound_check(const Slice_* thiz, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    if (p_index > thiz->Size)
    {
        abort();
    }
#endif
};

inline int8* Slice__get(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Slice__bound_check(thiz, p_index);
#endif
    return thiz->Begin + (p_index * t_thiz_elementtype);
};

inline void Slice__slide(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_offset_index)
{
#if CONTAINER_BOUND_TEST
    Slice__bound_check(thiz, p_offset_index);
#endif

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
#if STANDARD_ALLOCATION_BOUND_TEST
    return memory_cpy_safe(p_target->Begin, p_target->Size * t_elementtype, p_source->Begin, p_source->Size * t_elementtype);
#else
    return memory_cpy((int8*)p_target.Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

inline void Slice__zero(const Slice_* p_target, const uimax t_elementtype)
{
#if STANDARD_ALLOCATION_BOUND_TEST
    memory_zero_safe(p_target->Begin, p_target->Size * t_elementtype, p_target->Size * t_elementtype);
#else
    memory_zero((int8*)p_target.Begin, p_target.Size * sizeof(ElementType));
#endif
};

inline int8* Slice__memmove(const Slice_* p_target, const Slice_* p_source, const uimax t_elementtype)
{
#if STANDARD_ALLOCATION_BOUND_TEST
    return memory_move_safe(p_target->Begin, p_target->Size * t_elementtype, p_source->Begin, p_source->Size * t_elementtype);
#else
    return memory_move((int8*)p_target.Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
};

inline int8 Slice__memcompare(const Slice_* p_target, const Slice_* p_compared, const uimax t_elementtype)
{
    return memory_compare(p_target->Begin, p_compared->Begin, p_compared->Size * t_elementtype);
};

inline int8 Slice__memfind(const Slice_* p_target, const Slice_* p_compared, const uimax t_elementtype, uimax* out_index)
{
#if CONTAINER_BOUND_TEST
    if (p_compared->Size > p_target->Size)
    {
        abort();
    }
#endif

    Slice_ l_target_slice = *p_target;
    if (Slice__memcompare(&l_target_slice, p_compared, t_elementtype))
    {
        *out_index = 0;
        return 1;
    };

    for (uimax i = 1; i < p_target->Size - p_compared->Size + 1; i++)
    {
        Slice__slide(&l_target_slice, t_elementtype, 1);
        if (Slice__memcompare(&l_target_slice, p_compared, t_elementtype))
        {
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
#if CONTAINER_BOUND_TEST
    Slice__bound_inside_check(thiz, t_thiz_elementtype, &l_target);
#endif
    Slice_ l_source = Slice__build_begin_end(thiz->Begin, t_thiz_elementtype, p_break_index, p_break_index + p_moved_block_size);
    Slice__memmove(&l_target, &l_source, t_thiz_elementtype);
};

inline void Slice__move_memory_up(Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice_ l_target = Slice__build_memory_offset_elementnb(thiz->Begin, t_thiz_elementtype, p_break_index - p_move_delta, p_moved_block_size);
#if CONTAINER_BOUND_TEST
    Slice__bound_inside_check(thiz, t_thiz_elementtype, &l_target);
#endif
    Slice_ l_source = Slice__build_begin_end(thiz->Begin, t_thiz_elementtype, p_break_index, p_break_index + p_moved_block_size);
    Slice__memmove(&l_target, &l_source, t_thiz_elementtype);
};

inline void Slice__copy_memory(const Slice_* thiz, const uimax t_thiz_elementtype, const uimax p_copy_index, const Slice_* p_elements)
{
    Slice_ l_target = Slice__build(p_elements->Size, thiz->Begin + (p_copy_index * t_thiz_elementtype));

#if CONTAINER_BOUND_TEST
    Slice__bound_inside_check(thiz, t_thiz_elementtype, &l_target);
#endif

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

inline void Slice__assert_null_terminated(const Slice_* thiz, const uimax t_thiz_elementtype)
{
#if CONTAINER_BOUND_TEST
    assert_true(t_thiz_elementtype == sizeof(int8));
    assert_true(*Slice__get((Slice_*)thiz, t_thiz_elementtype, thiz->Size - 1) == '\0');
#endif
};

#define SliceC(ElementType) Slice_##ElementType
#define SliceC_build(ElementType) Slice_##ElementType##_build

#define Slice_declare(ElementType)                                                                                                                                                                     \
    typedef struct s_Slice_##ElementType                                                                                                                                                               \
    {                                                                                                                                                                                                  \
        union                                                                                                                                                                                          \
        {                                                                                                                                                                                              \
            struct                                                                                                                                                                                     \
            {                                                                                                                                                                                          \
                uimax Size;                                                                                                                                                                            \
                ElementType* Begin;                                                                                                                                                                    \
            };                                                                                                                                                                                         \
            Slice_ slice;                                                                                                                                                                              \
        };                                                                                                                                                                                             \
    } SliceC(ElementType)

    // TODO -> add methods

#define Slice_declare_functions(ElementType)                                                                                                                                                           \
    inline SliceC(ElementType) SliceC_build(ElementType)(const uimax p_size, ElementType* p_begin)                                                                                         \
    {                                                                                                                                                                                                  \
        return (SliceC(ElementType)){.slice = Slice__build(p_size, (int8*)p_begin)};                                                                                                                   \
    };

// inline Slice_ Slice__build_default() inline Slice_ Slice__build_begin_end(int8* p_memory, const uimax t_memory, const uimax p_begin, const uimax p_end) inline Slice_
// Slice__build_memory_offset_elementnb(int8* p_memory, const uimax t_memory, const uimax p_offset, const uimax p_element_nb)
