#pragma once

#pragma once

typedef struct Slice_
{
    uimax Size;
    int8* Begin;
} Slice_;

inline void Slice__bound_inside_check(const Slice_* thiz, const Slice_* p_tested_slice, const uimax type_element)
{
#if __DEBUG
    if ((p_tested_slice->Begin + (p_tested_slice->Size * type_element)) > (thiz->Begin + (thiz->Size * type_element)))
    {
        abort();
    }
#endif
};

inline void Slice__bound_check(const Slice_* thiz, const uimax p_index)
{
#if __DEBUG
    if (p_index > thiz->Size)
    {
        abort();
    }
#endif
};

inline Slice_ Slice__build_default()
{
    return Slice_{0, NULL};
};

inline Slice_ Slice__build_begin_end(int8* p_memory, const uimax p_begin, const uimax p_end, const uimax type_element)
{
    return Slice_{p_end - p_begin, p_memory + (p_begin * type_element)};
};

inline Slice_ Slice__build_memory_elementnb(int8* p_memory, const uimax p_element_nb)
{
    return Slice_{p_element_nb, p_memory};
};

inline Slice_ Slice__build_memory_offset_elementnb(int8* p_memory, const uimax p_offset, const uimax p_element_nb, const uimax type_element)
{
    return Slice_{p_element_nb, p_memory + (p_offset * type_element)};
};

inline Slice_ Slice__build_asint8_begin_end(int8* p_memory, const uimax p_begin, const uimax p_end, const uimax type_element)
{
    return Slice_{type_element * (p_end - p_begin), (int8*)(p_memory + (p_begin * type_element))};
};

inline Slice_ Slice__int8_build_rawstr(const int8* p_str)
{
    return Slice__build_memory_elementnb((int8*)p_str, strlen(p_str));
};

inline Slice_ Slice__build_asint8(Slice_* thiz, const uimax type_element)
{
    return Slice_{type_element * thiz->Size, (int8*)thiz->Begin};
};

inline Slice_ Slice__build_asint8(const Slice_* thiz, const uimax type_element)
{
    return Slice_{type_element * thiz->Size, (int8*)thiz->Begin};
};

inline Slice_ Slice__build_asint8_memory_elementnb(const int8* p_memory, const uimax p_element_nb, const uimax type_element)
{
    return Slice_{type_element * p_element_nb, (int8*)p_memory};
};

inline Slice_ Slice__build_asint8_memory_singleelement(const int8* p_memory, const uimax type_element)
{
    return Slice_{type_element, (int8*)p_memory};
};

inline int8* Slice__get(Slice_* thiz, const uimax p_index, const uimax type_element)
{
#if __DEBUG
    Slice__bound_check(thiz, p_index);
#endif
    return thiz->Begin + (p_index * type_element);
};

inline const int8* Slice__get(const Slice_* thiz, const uimax p_index, const uimax type_element)
{
    return Slice__get((Slice_*)thiz, p_index, type_element);
};

inline void Slice__assert_null_terminated(const Slice_* thiz, const uimax type_element)
{
#if __DEBUG
    assert_true(type_element == sizeof(int8));
    assert_true(*Slice__get(thiz, thiz->Size - 1, type_element) == '\0');
#endif
};

inline void Slice__slide(Slice_* thiz, const uimax p_offset_index, const uimax type_element)
{
#if __DEBUG
    Slice__bound_check(thiz, p_offset_index);
#endif

    thiz->Begin = thiz->Begin + (p_offset_index * type_element);
    thiz->Size -= p_offset_index;
};

inline Slice_ Slice__slide_rv(Slice_* thiz, const uimax p_offset_index, const uimax type_element)
{
    Slice_ l_return = *thiz;
    Slice__slide(&l_return, p_offset_index, type_element);
    return l_return;
};

inline Slice_ Slice__slide_rv(const Slice_* thiz, const uimax p_offset_index, const uimax type_element)
{
    return Slice__slide_rv((Slice_*)thiz, p_offset_index, type_element);
};

inline Slice_ Slice__cast(const Slice_* p_slice, const uimax type_casted)
{
#if __DEBUG
    if ((p_slice->Size % type_casted) != 0)
    {
        abort();
    }
#endif

    return Slice_{(uimax)p_slice->Size / type_casted, p_slice->Begin};
};

inline int8 Slice__compare(Slice_* thiz, const Slice_* p_other, const uimax type_element)
{
    return memory_compare((int8*)thiz->Begin, (int8*)p_other->Begin, p_other->Size * type_element);
};

inline int8 Slice__compare(const Slice_* thiz, const Slice_* p_other, const uimax type_element)
{
    return Slice__compare((Slice_*)thiz, p_other, type_element);
};

inline int8 Slice__find(const Slice_* thiz, const Slice_* p_other, uimax* out_index, const uimax type_element)
{
#if __DEBUG
    if (p_other->Size > thiz->Size)
    {
        abort();
    }
#endif

    Slice_ l_target_slice = *thiz;
    if (Slice__compare(&l_target_slice, p_other, type_element))
    {
        *out_index = 0;
        return 1;
    };

    for (uimax i = 1; i < thiz->Size - p_other->Size + 1; i++)
    {
        Slice__slide(&l_target_slice, 1, type_element);
        if (Slice__compare(&l_target_slice, p_other, type_element))
        {
            *out_index = i;
            return 1;
        };
    };

    *out_index = -1;
    return 0;
};

inline int8* Slice__move_memory(Slice_* thiz, const Slice_* p_source, const uimax type_element)
{
#if __DEBUG
    return memory_move_safe((int8*)thiz->Begin, thiz->Size * type_element, (int8*)p_source->Begin, p_source->Size * type_element);
#else
    return memory_move((int8*)thiz->Begin, (int8*)p_source->Begin, p_source->Size * type_element);
#endif
};

inline void Slice__move_memory_down(Slice_* thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta, const uimax type_element)
{
    Slice_ l_target = Slice__build_memory_offset_elementnb(thiz->Begin, p_break_index + p_move_delta, p_moved_block_size, type_element);
#if __DEBUG
    Slice__bound_inside_check(thiz, &l_target, type_element);
#endif
    Slice_ l_source = Slice__build_begin_end(thiz->Begin, p_break_index, p_break_index + p_moved_block_size, type_element);
    Slice__move_memory(&l_target, &l_source, type_element);
};

inline void Slice__move_memory_up(Slice_* thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta, const uimax type_element)
{
    Slice_ l_target = Slice__build_memory_offset_elementnb(thiz->Begin, p_break_index - p_move_delta, p_moved_block_size, type_element);
#if __DEBUG
    Slice__bound_inside_check(thiz, &l_target, type_element);
#endif
    Slice_ l_source = Slice__build_begin_end(thiz->Begin, p_break_index, p_break_index + p_moved_block_size, type_element);
    Slice__move_memory(&l_target, &l_source, type_element);
};

inline int8* Slice__copy_memory(Slice_* thiz, const Slice_* p_elements, const uimax type_element)
{
#if __DEBUG
    return memory_cpy_safe((int8*)thiz->Begin, thiz->Size * type_element, (int8*)p_elements->Begin, p_elements->Size * type_element);
#else
    return memory_cpy((int8*)thiz->Begin, (int8*)p_elements->Begin, p_elements->Size * type_element);
#endif
};

inline void Slice__copy_memory_at_index(Slice_* thiz, const uimax p_copy_index, const Slice_* p_elements, const uimax type_element)
{
    Slice_ l_target = Slice__build_memory_elementnb(thiz->Begin + (p_copy_index * type_element), p_elements->Size);

#if __DEBUG
    Slice__bound_inside_check(thiz, &l_target, type_element);
#endif

    Slice__copy_memory(&l_target, p_elements, type_element);
};

inline void Slice__copy_memory_at_index_2(Slice_* thiz, const uimax p_copy_index, const Slice_* p_elements_1, const Slice_* p_elements_2, const uimax type_element)
{
    Slice__copy_memory_at_index(thiz, p_copy_index, p_elements_1, type_element);
    Slice__copy_memory_at_index(thiz, p_copy_index + p_elements_1->Size, p_elements_2, type_element);
};

inline void Slice__copy_memory_at_index_3(Slice_* thiz, const uimax p_copy_index, const Slice_* p_elements_1, const Slice_* p_elements_2, const Slice_* p_elements_3, const uimax type_element)
{
    Slice__copy_memory_at_index_2(thiz, p_copy_index, p_elements_1, p_elements_2, type_element);
    Slice__copy_memory_at_index(thiz, p_copy_index + p_elements_1->Size + p_elements_2->Size, p_elements_3, type_element);
};

inline void Slice__zero(Slice_* thiz, const uimax type_element)
{
#if __DEBUG
    memory_zero_safe((int8*)thiz->Begin, thiz->Size * type_element, thiz->Size * type_element);
#else
    memory_zero((int8*)thiz->Begin, thiz->Size * type_element);
#endif
};

inline int8* Slice__cast_singleelement(const Slice_* p_slice, const uimax type_casted)
{
#if __DEBUG
    if (p_slice->Size < type_casted)
    {
        abort();
    }
#endif
    return p_slice->Begin;
};

/*
    A Slice is an encapsulated C style array.
*/
template <class type_element> struct Slice
{
    union
    {
        struct
        {
            uimax Size;
            type_element* Begin;
        };
        Slice_ slice;
    };
};

template <class type_element> inline void Slice_assert_null_terminated(const Slice<type_element>* thiz)
{
    Slice__assert_null_terminated(&thiz->slice, sizeof(type_element));
};

template <class type_element> inline Slice<type_element> Slice_build_default()
{
    return Slice<type_element>{.slice = Slice__build_default()};
};

template <class type_element> inline Slice<type_element> Slice_build_begin_end(type_element* p_memory, const uimax p_begin, const uimax p_end)
{
    return Slice<type_element>{.slice = Slice__build_begin_end((int8*)p_memory, p_begin, p_end, sizeof(type_element))};
};

template <class type_element> inline Slice<type_element> Slice_build_memory_elementnb(type_element* p_memory, const uimax p_element_nb)
{
    return Slice<type_element>{.slice = Slice__build_memory_elementnb((int8*)p_memory, p_element_nb)};
};

template <class type_element> inline Slice<type_element> Slice_build_memory_offset_elementnb(type_element* p_memory, const uimax p_offset, const uimax p_element_nb)
{
    return Slice<type_element>{.slice = Slice__build_memory_offset_elementnb(p_memory, p_offset, p_element_nb, sizeof(type_element))};
};

template <class type_element> inline Slice<int8> Slice_build_asint8_begin_end(type_element* p_memory, const uimax p_begin, const uimax p_end)
{
    return Slice<int8>{.slice = Slice__build_asint8_begin_end(p_memory, p_begin, p_end, sizeof(type_element))};
};

inline Slice<int8> Slice_int8_build_rawstr(const int8* p_str)
{
    return Slice<int8>{.slice = Slice__int8_build_rawstr(p_str)};
};

template <class type_element> inline Slice<int8> Slice_build_asint8(Slice<type_element>* thiz)
{
    return Slice<int8>{.slice = Slice__build_asint8(&thiz->slice, sizeof(type_element))};
};

template <class type_element> inline Slice<int8> Slice_build_asint8(const Slice<type_element>* thiz)
{
    return Slice<int8>{.slice = Slice__build_asint8(&thiz->slice, sizeof(type_element))};
};

template <class type_element> inline Slice<int8> Slice_build_asint8_memory_elementnb(const type_element* p_memory, const uimax p_element_nb)
{
    return Slice<int8>{.slice = Slice__build_asint8_memory_elementnb((int8*)p_memory, p_element_nb, sizeof(type_element))};
};

template <class type_element> inline Slice<int8> Slice_build_asint8_memory_singleelement(const type_element* p_memory)
{
    return Slice<int8>{.slice = Slice__build_asint8_memory_singleelement((int8*)p_memory, sizeof(type_element))};
};

template <class type_element> inline static type_element* Slice_get(Slice<type_element>* thiz, const uimax p_index)
{
    return (type_element*)Slice__get(&thiz->slice, p_index, sizeof(type_element));
};

template <class type_element> inline const type_element* Slice_get(const Slice<type_element>* thiz, const uimax p_index)
{
    return Slice_get((Slice<type_element>*)thiz, p_index);
};

template <class type_element> inline static void Slice_slide(Slice<type_element>* thiz, const uimax p_offset_index)
{
    Slice__slide(&thiz->slice, p_offset_index, sizeof(type_element));
};

template <class type_element> inline static Slice<type_element> Slice_slide_rv(Slice<type_element>* thiz, const uimax p_offset_index)
{
    return Slice<type_element>{.slice = Slice__slide_rv(&thiz->slice, p_offset_index, sizeof(type_element))};
};

template <class type_element> inline static Slice<type_element> Slice_slide_rv(const Slice<type_element>* thiz, const uimax p_offset_index)
{
    return Slice<type_element>{.slice = Slice__slide_rv(&thiz->slice, p_offset_index, sizeof(type_element))};
};

template <class type_casted> inline Slice<type_casted> Slice_cast(const Slice<int8>* p_slice)
{
    return Slice<type_casted>{.slice = Slice__cast(&p_slice->slice, sizeof(type_casted))};
};

template <class type_element> inline static int8 Slice_compare(Slice<type_element>* thiz, const Slice<type_element>* p_other)
{
    return Slice__compare(&thiz->slice, &p_other->slice, sizeof(type_element));
};

template <class type_element> inline static int8 Slice_compare(const Slice<type_element>* thiz, const Slice<type_element>* p_other)
{
    return Slice__compare(&thiz->slice, &p_other->slice, sizeof(type_element));
};

template <class type_element> inline static int8 Slice_find(const Slice<type_element>* thiz, const Slice<type_element>* p_other, uimax* out_index)
{
    return Slice__find(&thiz->slice, &p_other->slice, out_index, sizeof(type_element));
};

template <class type_element> inline static int8* Slice_move_memory(Slice<type_element>* thiz, const Slice<type_element>* p_source)
{
    return Slice__move_memory(&thiz->slice, &p_source->slice, sizeof(type_element));
};

template <class type_element> inline static void Slice_move_memory_down(Slice<type_element>* thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_down(&thiz->slice, p_moved_block_size, p_break_index, p_move_delta, sizeof(type_element));
};

template <class type_element> inline static void Slice_move_memory_up(Slice<type_element>* thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_up(&thiz->slice, p_moved_block_size, p_break_index, p_move_delta, sizeof(type_element));
};

template <class type_element> inline static int8* Slice_copy_memory(Slice<type_element>* thiz, const Slice<type_element>* p_elements)
{
    return Slice__copy_memory(&thiz->slice, &p_elements->slice, sizeof(type_element));
};

template <class type_element> inline static void Slice_copy_memory_at_index(Slice<type_element>* thiz, const uimax p_copy_index, const Slice<type_element>* p_elements)
{
    Slice__copy_memory_at_index(&thiz->slice, p_copy_index, &p_elements->slice, sizeof(type_element));
};

template <class type_element>
inline static void Slice_copy_memory_at_index_2(Slice<type_element>* thiz, const uimax p_copy_index, const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2)
{
    Slice__copy_memory_at_index_2(&thiz->slice, p_copy_index, &p_elements_1->slice, &p_elements_2->slice, sizeof(type_element));
};

template <class type_element>
inline static void Slice_copy_memory_at_index_3(Slice<type_element>* thiz, const uimax p_copy_index, const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2,
                                                const Slice<type_element>* p_elements_3)
{
    Slice__copy_memory_at_index_3(&thiz->slice, p_copy_index, &p_elements_1->slice, &p_elements_2->slice, &p_elements_3->slice, sizeof(type_element));
};

template <class type_element> inline static void Slice_zero(Slice<type_element>* thiz)
{
    Slice__zero(&thiz->slice, sizeof(type_element));
};

template <class type_casted> inline type_casted* Slice_cast_singleelement(const Slice<int8>* p_slice)
{
    return (type_casted*)Slice__cast_singleelement(&p_slice->slice, sizeof(type_casted));
};

#define Slice_declare_sized(type_element, type_size, name_arr, name_slice, ...)                                                                                                                        \
    type_element name_arr[type_size] = {__VA_ARGS__};                                                                                                                                                  \
    Slice<type_element> name_slice = {type_size, name_arr};

/*
    A SliceIndex is just a begin and end uimax
*/
struct SliceIndex
{
    uimax Begin;
    uimax Size;
};

inline static SliceIndex SliceIndex_build(const uimax p_begin, const uimax p_size)
{
    return SliceIndex{p_begin, p_size};
};

inline static SliceIndex SliceIndex_build_default()
{
    return SliceIndex_build(0, 0);
};

inline static void SliceIndex_slice_two(const SliceIndex* thiz, const uimax p_break_point, SliceIndex* out_left, SliceIndex* out_right)
{
    uimax l_source_initial_size = thiz->Size;
    *out_left = SliceIndex_build(thiz->Begin, p_break_point - thiz->Begin);
    *out_right = SliceIndex_build(p_break_point, l_source_initial_size - out_left->Size);
};

template <class type_element> struct SliceOffset
{
    uimax Offset;
    type_element* Memory;
};

template <class type_element> inline static SliceOffset<type_element> SliceOffset_build(type_element* p_memory, const uimax p_offset)
{
    return SliceOffset<type_element>{p_offset, p_memory};
};

template <class type_element> inline static SliceOffset<type_element> SliceOffset_build_from_sliceindex(type_element* p_memory, const SliceIndex* p_slice_index)
{
    return SliceOffset<type_element>{p_slice_index->Begin, p_memory};
};