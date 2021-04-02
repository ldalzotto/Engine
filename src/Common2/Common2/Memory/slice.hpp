#pragma once

/*
    A Slice is an encapsulated C style array.
*/
template <class type_element> struct Slice
{
    uimax Size;
    type_element* Begin;
};

template <class type_element> inline void Slice_bound_inside_check(const Slice<type_element>* thiz, const Slice<type_element>* p_tested_slice)
{
#if __DEBUG
    if ((p_tested_slice->Begin + p_tested_slice->Size) > (thiz->Begin + thiz->Size))
    {
        abort();
    }
#endif
};

template <class type_element> inline void Slice_bound_check(const Slice<type_element>* thiz, const uimax p_index)
{
#if __DEBUG
    if (p_index > thiz->Size)
    {
        abort();
    }
#endif
};

template <class type_element> inline void Slice_assert_null_terminated(const Slice<type_element>* thiz)
{
#if __DEBUG
    assert_true(sizeof(type_element) == sizeof(int8));
    assert_true(*Slice_get(thiz, thiz->Size - 1) == '\0');
#endif
};

template <class type_element> inline Slice<type_element> Slice_build_default()
{
    return Slice<type_element>{0, NULL};
};

template <class type_element> inline Slice<type_element> Slice_build_begin_end(type_element* p_memory, const uimax p_begin, const uimax p_end)
{
    return Slice<type_element>{p_end - p_begin, p_memory + p_begin};
};

template <class type_element> inline Slice<type_element> Slice_build_memory_elementnb(type_element* p_memory, const uimax p_element_nb)
{
    return Slice<type_element>{p_element_nb, p_memory};
};

template <class type_element> inline Slice<type_element> Slice_build_memory_offset_elementnb(type_element* p_memory, const uimax p_offset, const uimax p_element_nb)
{
    return Slice<type_element>{p_element_nb, p_memory + p_offset};
};

template <class type_element> inline Slice<int8> Slice_build_asint8_begin_end(type_element* p_memory, const uimax p_begin, const uimax p_end)
{
    return Slice<int8>{sizeof(type_element) * (p_end - p_begin), (int8*)(p_memory + p_begin)};
};

inline Slice<int8> Slice_int8_build_rawstr(const int8* p_str)
{
    return Slice_build_memory_elementnb<int8>((int8*)p_str, strlen(p_str));
};

inline Slice<int8> Slice_int8_build_rawstr_with_null_termination(const int8* p_str)
{
    return Slice_build_memory_elementnb<int8>((int8*)p_str, strlen(p_str) + 1);
};

template <class type_element> inline Slice<int8> Slice_build_asint8(Slice<type_element>* thiz)
{
    return Slice<int8>{sizeof(type_element) * thiz->Size, (int8*)thiz->Begin};
};

template <class type_element> inline Slice<int8> Slice_build_asint8(const Slice<type_element>* thiz)
{
    return Slice<int8>{sizeof(type_element) * thiz->Size, (int8*)thiz->Begin};
};

template <class type_element> inline Slice<int8> Slice_build_asint8_memory_elementnb(const type_element* p_memory, const uimax p_element_nb)
{
    return Slice<int8>{sizeof(type_element) * p_element_nb, (int8*)p_memory};
};

template <class type_element> inline Slice<int8> Slice_build_asint8_memory_singleelement(const type_element* p_memory)
{
    return Slice<int8>{sizeof(type_element), (int8*)p_memory};
};

template <class type_element> inline static type_element* Slice_get(Slice<type_element>* thiz, const uimax p_index)
{
#if __DEBUG
    Slice_bound_check(thiz, p_index);
#endif
    return &thiz->Begin[p_index];
};

template <class type_element> inline const type_element* Slice_get(const Slice<type_element>* thiz, const uimax p_index)
{
    return Slice_get((Slice<type_element>*)thiz, p_index);
};

template <class type_element> inline static void Slice_slide(Slice<type_element>* thiz, const uimax p_offset_index)
{
#if __DEBUG
    Slice_bound_check(thiz, p_offset_index);
#endif

    thiz->Begin = thiz->Begin + p_offset_index;
    thiz->Size -= p_offset_index;
};

template <class type_element> inline static Slice<type_element> Slice_slide_rv(Slice<type_element>* thiz, const uimax p_offset_index)
{
    Slice<type_element> l_return = *thiz;
    Slice_slide(&l_return, p_offset_index);
    return l_return;
};

template <class type_element> inline static Slice<type_element> Slice_slide_rv(const Slice<type_element>* thiz, const uimax p_offset_index)
{
    return Slice_slide_rv((Slice<type_element>*)thiz, p_offset_index);
};

template <class type_casted> inline Slice<type_casted> Slice_cast(const Slice<int8>* p_slice)
{
#if __DEBUG
    if ((p_slice->Size % sizeof(type_casted)) != 0)
    {
        abort();
    }
#endif

    return Slice<type_casted>{(uimax)p_slice->Size / sizeof(type_casted), (type_casted*)p_slice->Begin};
};

template <class type_element> inline static int8 Slice_compare(Slice<type_element>* thiz, const Slice<type_element>* p_other)
{
    return memory_compare((int8*)thiz->Begin, (int8*)p_other->Begin, p_other->Size * sizeof(type_element));
};

template <class type_element> inline static int8 Slice_compare(const Slice<type_element>* thiz, const Slice<type_element>* p_other)
{
    return Slice_compare((Slice<type_element>*)thiz, p_other);
};

template <class type_element> inline static int8 Slice_find(const Slice<type_element>* thiz, const Slice<type_element>* p_other, uimax* out_index)
{
#if __DEBUG
    if (p_other->Size > thiz->Size)
    {
        abort();
    }
#endif

    Slice<type_element> l_target_slice = *thiz;
    if (Slice_compare(&l_target_slice, p_other))
    {
        *out_index = 0;
        return 1;
    };

    for (uimax i = 1; i < thiz->Size - p_other->Size + 1; i++)
    {
        Slice_slide(&l_target_slice, 1);
        if (Slice_compare(&l_target_slice, p_other))
        {
            *out_index = i;
            return 1;
        };
    };

    *out_index = -1;
    return 0;
};

template <class type_element> inline static int8* Slice_move_memory(Slice<type_element>* thiz, const Slice<type_element>* p_source)
{
#if __DEBUG
    return memory_move_safe((int8*)thiz->Begin, thiz->Size * sizeof(type_element), (int8*)p_source->Begin, p_source->Size * sizeof(type_element));
#else
    return memory_move((int8*)thiz->Begin, (int8*)p_source->Begin, p_source->Size * sizeof(type_element));
#endif
};

template <class type_element> inline static void Slice_move_memory_down(Slice<type_element>* thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice<type_element> l_target = Slice_build_memory_offset_elementnb<type_element>(thiz->Begin, p_break_index + p_move_delta, p_moved_block_size);
#if __DEBUG
    Slice_bound_inside_check(thiz, &l_target);
#endif
    Slice<type_element> l_source = Slice_build_begin_end<type_element>(thiz->Begin, p_break_index, p_break_index + p_moved_block_size);
    Slice_move_memory(&l_target, &l_source);
};

template <class type_element> inline static void Slice_move_memory_up(Slice<type_element>* thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice<type_element> l_target = Slice_build_memory_offset_elementnb<type_element>(thiz->Begin, p_break_index - p_move_delta, p_moved_block_size);
#if __DEBUG
    Slice_bound_inside_check(thiz, &l_target);
#endif
    Slice<type_element> l_source = Slice_build_begin_end<type_element>(thiz->Begin, p_break_index, p_break_index + p_moved_block_size);
    Slice_move_memory(&l_target, &l_source);
};

template <class type_element> inline static int8* Slice_copy_memory(Slice<type_element>* thiz, const Slice<type_element>* p_elements)
{
#if __DEBUG
    return memory_cpy_safe((int8*)thiz->Begin, thiz->Size * sizeof(type_element), (int8*)p_elements->Begin, p_elements->Size * sizeof(type_element));
#else
    return memory_cpy((int8*)thiz->Begin, (int8*)p_elements->Begin, p_elements->Size * sizeof(type_element));
#endif
};

template <class type_element> inline static void Slice_copy_memory_at_index(Slice<type_element>* thiz, const uimax p_copy_index, const Slice<type_element>* p_elements)
{
    Slice<type_element> l_target = Slice_build_memory_elementnb<type_element>(thiz->Begin + p_copy_index, p_elements->Size);

#if __DEBUG
    Slice_bound_inside_check(thiz, &l_target);
#endif

    Slice_copy_memory(&l_target, p_elements);
};

template <class type_element>
inline static void Slice_copy_memory_at_index_2(Slice<type_element>* thiz, const uimax p_copy_index, const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2)
{
    Slice_copy_memory_at_index(thiz, p_copy_index, p_elements_1);
    Slice_copy_memory_at_index(thiz, p_copy_index + p_elements_1->Size, p_elements_2);
};

template <class type_element>
inline static void Slice_copy_memory_at_index_3(Slice<type_element>* thiz, const uimax p_copy_index, const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2,
                                                const Slice<type_element>* p_elements_3)
{
    Slice_copy_memory_at_index_2(thiz, p_copy_index, p_elements_1, p_elements_2);
    Slice_copy_memory_at_index(thiz, p_copy_index + p_elements_1->Size + p_elements_2->Size, p_elements_3);
};

template <class type_element> inline static void Slice_zero(Slice<type_element>* thiz)
{
#if __DEBUG
    memory_zero_safe((int8*)thiz->Begin, thiz->Size * sizeof(type_element), thiz->Size * sizeof(type_element));
#else
    memory_zero((int8*)thiz->Begin, thiz->Size * sizeof(type_element));
#endif
};

template <class type_casted> inline type_casted* Slice_cast_singleelement(const Slice<int8>* p_slice)
{
#if __DEBUG
    if (p_slice->Size < sizeof(type_casted))
    {
        abort();
    }
#endif
    return (type_casted*)p_slice->Begin;
};

template <class type_casted> inline Slice<type_casted> Slice_cast_fixedelementnb(const Slice<int8>* p_slice, const uimax p_element_nb)
{
#if __DEBUG
    if (p_slice->Size < (sizeof(type_casted) * p_element_nb))
    {
        abort();
    }
#endif

    return slice_build_memory_elementnb((type_casted*)p_slice->Begin, p_element_nb);
};

#if __TOKEN
#define sliceoftoken_cast(CastedType, SourceSlice)                                                                                                                                                     \
    Slice<Token(CastedType)>                                                                                                                                                                           \
    {                                                                                                                                                                                                  \
        (SourceSlice).Size, (Token(CastedType)*)(SourceSlice).Begin                                                                                                                                    \
    }
#else
#define sliceoftoken_cast(CastedType, SourceSlice) SourceSlice
#endif

#define Slice_declare_sized(type_element, type_size, name_arr, name_slice, ...) type_element name_arr[type_size] = {__VA_ARGS__}; Slice<type_element> name_slice = {type_size, name_arr};

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

template <class type_element>
inline static SliceOffset<type_element> SliceOffset_build(type_element* p_memory, const uimax p_offset)
{
    return SliceOffset<type_element>{p_offset, p_memory};
};

template <class type_element>
inline static SliceOffset<type_element> SliceOffset_build_from_sliceindex(type_element* p_memory, const SliceIndex* p_slice_index)
{
    return SliceOffset<type_element>{p_slice_index->Begin, p_memory};
};