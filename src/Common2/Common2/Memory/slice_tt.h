
#ifndef SLICE_TT_H

#define Slice(type_element) CAT_2(Slice_, type_element)

#define Slice_c_build_default(type_element) CAT_2_(Slice(type_element), _build_default)()
#define Slice_c_build_begin_end(type_element, p_memory, p_begin, p_end) CAT_2_(Slice(type_element), _build_begin_end)(p_memory, p_begin, p_end)
#define Slice_c_build_memory_elementnb(type_element, p_memory, p_element_nb) CAT_2_(Slice(type_element), _build_memory_elementnb)(p_memory, p_element_nb)
#define Slice_c_build_memory_offset_elementnb(type_element, p_memory, p_offset, p_element_nb) CAT_2_(Slice(type_element), _build_memory_offset_elementnb)(p_memory, p_offset, p_element_nb)
#define Slice_c_build_asint8_begin_end(type_element, p_memory, p_begin, p_end) CAT_2_(Slice(type_element), _build_asint8_begin_end)(p_memory, p_begin, p_end)
#define Slice_c_build_asint8(type_element, thiz) CAT_2_(Slice(type_element), _build_asint8)(thiz)
#define Slice_c_build_asint8_memory_elementnb(type_element, p_memory, p_elementnb) CAT_2_(Slice(type_element), _build_asint8_memory_elementnb)(p_memory, p_elementnb)
#define Slice_c_build_asint8_memory_singleelement(type_element, p_memory) CAT_2_(Slice(type_element), _build_asint8_memory_singleelement)(p_memory)
#define Slice_c_get(type_element, thiz, p_index) CAT_2_(Slice(type_element), _get)(thiz, p_index)
#define Slice_c_slide(type_element, thiz, p_offset_index) CAT_2_(Slice(type_element), _slide)(thiz, p_offset_index)
#define Slice_c_slide_rv(type_element, thiz, p_offset_index) CAT_2_(Slice(type_element), _slide_rv)(thiz, p_offset_index)
#define Slice_c_compare(type_element, thiz, p_other) CAT_2_(Slice(type_element), _compare)(thiz, p_other)
#define Slice_c_find(type_element, thiz, p_other, out_index) CAT_2_(Slice(type_element), _find)(thiz, p_other, out_index)
#define Slice_c_move_memory(type_element, thiz, p_source) CAT_2_(Slice(type_element), _move_memory)(thiz, p_source)
#define Slice_c_move_memory_down(type_element, thiz, p_moved_block_size, p_break_index, p_move_delta)                                                                                                  \
    CAT_2_(Slice(type_element), _move_memory_down)(thiz, p_moved_block_size, p_break_index, p_move_delta)
#define Slice_c_move_memory_up(type_element, thiz, p_moved_block_size, p_break_index, p_move_delta) CAT_2_(Slice(type_element), _move_memory_up)(thiz, p_moved_block_size, p_break_index, p_move_delta)
#define Slice_c_copy_memory(type_element, thiz, p_elements) CAT_2_(Slice(type_element), _copy_memory)(thiz, p_elements)
#define Slice_c_copy_memory_at_index(type_element, thiz, p_copy_index, p_elements) CAT_2_(Slice(type_element), _copy_memory_at_index)(thiz, p_copy_index, p_elements)
#define Slice_c_copy_memory_at_index_2(type_element, thiz, p_copy_index, p_elements_1, p_elements_2)                                                                                                   \
    CAT_2_(Slice(type_element), _copy_memory_at_index_2)(thiz, p_copy_index, p_elements_1, p_elements_2)
#define Slice_c_copy_memory_at_index_3(type_element, thiz, p_copy_index, p_elements_1, p_elements_2, p_elements_3)                                                                                     \
    CAT_2_(Slice(type_element), _copy_memory_at_index_3)(thiz, p_copy_index, p_elements_1, p_elements_2, p_elements_3)
#define Slice_c_zero(type_element, thiz) CAT_2_(Slice(type_element), _zero)(thiz)
#define Slice_c_cast_singleelement(type_element, p_slice) CAT_2_(Slice(type_element), _cast_singleelement)(p_slice)

#define Slice_c_get_Size(thiz) (thiz)->Size
#define Slice_c_get_Begin(thiz) (thiz)->Begin
#define Slice_c_get_slice(thiz) (thiz)->slice

#define Slice_c_cast(type_casted_element, p_slice)                                                                                                                                                     \
    Slice(Slice_c_cast)                                                                                                                                                                                \
    {                                                                                                                                                                                                  \
        .slice = Slice__cast(&Slice_c_get_slice(p_slice), sizeof(type_casted_element))                                                                                                                 \
    }

#define Slice_c_declare_sized(type_element, type_size, name_arr, name_slice, ...)                                                                                                                      \
    type_element name_arr[type_size] = {__VA_ARGS__};                                                                                                                                                  \
    Slice(type_element) name_slice = {type_size, name_arr};

#define SLICE_TT_H
#endif

typedef struct Slice(mp_type_element)
{
    union
    {
        uimax Size;
        mp_type_element* Begin;
    };
    Slice_ slice;
}
Slice(mp_type_element);

inline Slice(mp_type_element) Slice_c_build_default(mp_type_element)
{
    return Slice(mp_type_element){.slice = Slice__build_default()};
};

inline Slice(mp_type_element) Slice_c_build_begin_end(mp_type_element, mp_type_element* p_memory, const uimax p_begin, const uimax p_end)
{
    return Slice(mp_type_element){.slice = Slice__build_begin_end((int8*)p_memory, p_begin, p_end, sizeof(mp_type_element))};
};

inline Slice(mp_type_element) Slice_c_build_memory_elementnb(mp_type_element, mp_type_element* p_memory, const uimax p_element_nb)
{
    return Slice(mp_type_element){.slice = Slice__build_memory_elementnb((int8*)p_memory, p_element_nb)};
};

inline Slice(mp_type_element) Slice_c_build_memory_offset_elementnb(mp_type_element, mp_type_element* p_memory, const uimax p_offset, const uimax p_element_nb)
{
    return Slice(mp_type_element){.slice = Slice__build_memory_offset_elementnb((int8*)p_memory, p_offset, p_element_nb, sizeof(mp_type_element))};
};

inline Slice(mp_type_element) Slice_c_build_asint8_begin_end(mp_type_element, mp_type_element* p_memory, const uimax p_begin, const uimax p_end)
{
    return Slice(mp_type_element){.slice = Slice__build_asint8_begin_end((int8*)p_memory, p_begin, p_end, sizeof(mp_type_element))};
};

inline Slice(int8) Slice_c_build_asint8(mp_type_element, const Slice(mp_type_element) * thiz)
{
    return Slice(int8){.slice = Slice__build_asint8((Slice_*)&Slice_c_get_slice(thiz), sizeof(mp_type_element))};
};

inline Slice(int8) Slice_c_build_asint8_memory_elementnb(mp_type_element, const mp_type_element* p_memory, const uimax p_element_nb)
{
    return Slice(int8){.slice = Slice__build_asint8_memory_elementnb((int8*)p_memory, p_element_nb, sizeof(mp_type_element))};
};

inline Slice(int8) Slice_c_build_asint8_memory_singleelement(mp_type_element, const mp_type_element* p_memory)
{
    return Slice(int8){.slice = Slice__build_asint8_memory_singleelement((int8*)p_memory, sizeof(mp_type_element))};
};

inline mp_type_element* Slice_c_get(mp_type_element, const Slice(mp_type_element) * thiz, const uimax p_index)
{
    return (mp_type_element*)Slice__get((Slice_*)&Slice_c_get_slice(thiz), p_index, sizeof(mp_type_element));
};

inline void Slice_c_slide(mp_type_element, Slice(mp_type_element) * thiz, const uimax p_offset_index)
{
    Slice__slide(&thiz->slice, p_offset_index, sizeof(mp_type_element));
};

inline Slice(mp_type_element) Slice_c_slide_rv(mp_type_element, const Slice(mp_type_element) * thiz, const uimax p_offset_index)
{
    return Slice(mp_type_element){.slice = Slice__slide_rv((Slice_*)&Slice_c_get_slice(thiz), p_offset_index, sizeof(mp_type_element))};
};

inline int8 Slice_c_compare(mp_type_element, const Slice(mp_type_element) * thiz, const Slice(mp_type_element) * p_other)
{
    return Slice__compare((Slice_*)&thiz->slice, &p_other->slice, sizeof(mp_type_element));
};

inline int8 Slice_c_find(mp_type_element, const Slice(mp_type_element) * thiz, const Slice(mp_type_element) * p_other, uimax* out_index)
{
    return Slice__find(&thiz->slice, &p_other->slice, out_index, sizeof(mp_type_element));
};

inline int8* Slice_c_move_memory(mp_type_element, Slice(mp_type_element) * thiz, const Slice(mp_type_element) * p_source)
{
    return Slice__move_memory(&thiz->slice, &p_source->slice, sizeof(mp_type_element));
};

inline void Slice_c_move_memory_down(mp_type_element, Slice(mp_type_element) * thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_down(&thiz->slice, p_moved_block_size, p_break_index, p_move_delta, sizeof(mp_type_element));
};

inline void Slice_c_move_memory_up(mp_type_element, Slice(mp_type_element) * thiz, const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_up(&thiz->slice, p_moved_block_size, p_break_index, p_move_delta, sizeof(mp_type_element));
};

inline int8* Slice_c_copy_memory(mp_type_element, Slice(mp_type_element) * thiz, const Slice(mp_type_element) * p_elements)
{
    return Slice__copy_memory(&thiz->slice, &p_elements->slice, sizeof(mp_type_element));
};

inline void Slice_c_copy_memory_at_index(mp_type_element, Slice(mp_type_element) * thiz, const uimax p_copy_index, const Slice(mp_type_element) * p_elements)
{
    Slice__copy_memory_at_index(&thiz->slice, p_copy_index, &p_elements->slice, sizeof(mp_type_element));
};

inline void Slice_c_copy_memory_at_index_2(mp_type_element, Slice(mp_type_element) * thiz, const uimax p_copy_index, const Slice(mp_type_element) * p_elements_1,
                                           const Slice(mp_type_element) * p_elements_2)
{
    Slice__copy_memory_at_index_2(&thiz->slice, p_copy_index, &p_elements_1->slice, &p_elements_2->slice, sizeof(mp_type_element));
};

inline void Slice_c_copy_memory_at_index_3(mp_type_element, Slice(mp_type_element) * thiz, const uimax p_copy_index, const Slice(mp_type_element) * p_elements_1,
                                           const Slice(mp_type_element) * p_elements_2, const Slice(mp_type_element) * p_elements_3)
{
    Slice__copy_memory_at_index_3(&thiz->slice, p_copy_index, &p_elements_1->slice, &p_elements_2->slice, &p_elements_3->slice, sizeof(mp_type_element));
};

inline void Slice_c_zero(mp_type_element, Slice(mp_type_element) * thiz)
{
    Slice__zero(&thiz->slice, sizeof(mp_type_element));
};

inline mp_type_element* Slice_c_cast_singleelement(mp_type_element, const Slice<int8>* p_slice)
{
    return (mp_type_element*)Slice__cast_singleelement(&p_slice->slice, sizeof(mp_type_element));
};

#undef mp_type_element
