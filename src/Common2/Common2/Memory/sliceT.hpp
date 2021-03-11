#pragma once


/*
    A Slice is an encapsulated C style array.
*/
template <class ElementType> struct Slice
{
    union
    {
        struct
        {
            uimax Size;
            ElementType* Begin;
        };
        Slice_ slice;
    };

    inline static Slice<ElementType> build_default()
    {
        return Slice<ElementType>{.slice = Slice__build_default()};
    };

    inline static Slice<ElementType> build(ElementType* p_memory, const uimax p_element_nb)
    {
        return Slice<ElementType>{.slice = Slice__build(p_element_nb, (int8*)p_memory)};
    };

    inline static Slice<ElementType> build_begin_end(ElementType* p_memory, const uimax p_begin, const uimax p_end)
    {
        return Slice<ElementType>{.slice = Slice__build_begin_end((int8*)p_memory, sizeof(ElementType), p_begin, p_end)};
    };

    inline static Slice<ElementType> build_memory_offset_elementnb(ElementType* p_memory, const uimax p_offset, const uimax p_element_nb)
    {
        return Slice<ElementType>{.slice = Slice__build_memory_offset_elementnb((int8*)p_memory, sizeof(ElementType), p_offset, p_element_nb)};
    };

    inline static Slice<int8> build_asint8_begin_end(ElementType* p_memory, const uimax p_begin, const uimax p_end)
    {
        return Slice<int8>{.slice = Slice__build_asint8_begin_end((int8*)p_memory, sizeof(ElementType), p_begin, p_end)};
    };

    inline Slice<int8> build_asint8() const
    {
        return Slice<int8>{.slice = Slice__build_asint8((Slice_*)&this->slice, sizeof(ElementType))};
    };

    inline static Slice<int8> build_asint8_memory_elementnb(const ElementType* p_memory, const uimax p_element_nb)
    {
        return Slice<int8>{.slice = Slice__build_asint8_memory_elementnb((int8*)p_memory, sizeof(ElementType), p_element_nb)};
    };

    inline static Slice<int8> build_asint8_memory_singleelement(const ElementType* p_memory)
    {
        return Slice<int8>{.slice = Slice__build_asint8_memory_singleelement((int8*)p_memory, sizeof(ElementType))};
    };

    inline ElementType& get(const uimax p_index)
    {
        return *(ElementType*)Slice__get((Slice_*)this, sizeof(ElementType), p_index);
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((Slice<ElementType>*)this)->get(p_index);
    };

    inline void slide(const uimax p_offset_index)
    {
        Slice__slide((Slice_*)&this->slice, sizeof(ElementType), p_offset_index);
    };

    inline Slice<ElementType> slide_rv(const uimax p_offset_index) const
    {
        return Slice<ElementType>{.slice = Slice__slide_rv((Slice_*)&this->slice, sizeof(ElementType), p_offset_index)};
    };

    inline int8 compare(const Slice<ElementType>& p_other) const
    {
        return Slice__memcompare(&this->slice, &p_other.slice, sizeof(ElementType));
    };

    inline int8 find(const Slice<ElementType>& p_other, uimax* out_index) const
    {
        return Slice__memfind(&this->slice, &p_other.slice, sizeof(ElementType), out_index);
    };

    inline void move_memory_down(const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice__move_memory_down(&this->slice, sizeof(ElementType), p_moved_block_size, p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice__move_memory_up(&this->slice, sizeof(ElementType), p_moved_block_size, p_break_index, p_move_delta);
    };

    inline void copy_memory(const uimax p_copy_index, const Slice<ElementType>& p_elements)
    {
        Slice__copy_memory(&this->slice, sizeof(ElementType), p_copy_index, &p_elements.slice);
    };

    inline void copy_memory_2(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        Slice__copy_memory_2(&this->slice, sizeof(ElementType), p_copy_index, &p_elements_1.slice, &p_elements_2.slice);
    };

    inline void copy_memory_3(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
    {
        Slice__copy_memory_3(&this->slice, sizeof(ElementType), p_copy_index, &p_elements_1.slice, &p_elements_2.slice, &p_elements_3.slice);
    };

    inline void zero()
    {
        Slice__zero(&this->slice, sizeof(ElementType));
    };

    inline void assert_null_terminated() const
    {
        Slice__assert_null_terminated(&this->slice, sizeof(ElementType));
    };
};

inline Slice<int8> slice_int8_build_rawstr(const int8* p_str)
{
    return Slice<int8>{.slice = Slice__build_rawstr(p_str)};
};

inline Slice<int8> slice_int8_build_rawstr_with_null_termination(const int8* p_str)
{
    return Slice<int8>{.slice = Slice__build_rawstr_with_null_termination(p_str)};
};

template <class CastedType> inline Slice<CastedType> slice_cast(const Slice<int8>& p_slice)
{
    return Slice<CastedType>{.slice = Slice__cast(&p_slice.slice, sizeof(CastedType))};
};

template <class CastedType> inline CastedType* slice_cast_singleelement(const Slice<int8>& p_slice)
{
    return (CastedType*)Slice__cast_singleelement(&p_slice.slice, sizeof(CastedType));
};

#if TOKEN_TYPE_SAFETY
#define sliceoftoken_cast(CastedType, SourceSlice)                                                                                                                                                     \
    Slice<TokenT(CastedType)>                                                                                                                                                                          \
    {                                                                                                                                                                                                  \
        (SourceSlice).Size, (TokenT(CastedType)*)(SourceSlice).Begin                                                                                                                                   \
    }
#else
#define sliceoftoken_cast(CastedType, SourceSlice) SourceSlice
#endif

template <class ElementType> inline int8* slice_memmove(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
    return Slice__memmove(&p_target.slice, &p_source.slice, sizeof(ElementType));
};

template <class ElementType> inline int8* slice_memcpy(const Slice<ElementType>& p_target, const Slice<ElementType>& p_source)
{
    return Slice__memcpy(&p_target.slice, &p_source.slice, sizeof(ElementType));
};

template <class ElementType> inline int8 slice_memcompare(const Slice<ElementType>& p_target, const Slice<ElementType>& p_compared)
{
    return Slice__memcompare(&p_target.slice, &p_compared.slice, sizeof(ElementType));
};

template <class ElementType, uint32 Size_t> struct SliceN
{
    ElementType Memory[Size_t];

    inline Slice<ElementType> to_slice()
    {
        return Slice<ElementType>{.slice = SliceN___to_slice((int8*)&this->Memory, Size_t)};
    };

    inline const Slice<ElementType> to_slice() const
    {
        return Slice<ElementType>{.slice = SliceN___to_slice((int8*)&this->Memory, Size_t)};
    };

    inline uimax Size()
    {
        return Size_t;
    };

    inline ElementType& get(const uimax p_index)
    {
        return *(ElementType*)SliceN___get((int8*)&this->Memory, sizeof(ElementType), Size_t, p_index);
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((SliceN<ElementType, Size_t>*)this)->get(p_index);
    }
};

template <class ElementType> struct SliceOffset
{
    union
    {
        struct
        {
            uimax Offset;
            ElementType* Memory;
        };
        SliceOffset_ slice_offset;
    };

    inline static SliceOffset<ElementType> build(ElementType* p_memory, const uimax p_offset)
    {
        return SliceOffset<ElementType>{.slice_offset = SliceOffset__build((int8*)p_memory, p_offset)};
    };

    inline static SliceOffset<ElementType> build_from_sliceindex(ElementType* p_memory, const SliceIndex& p_slice_index)
    {
        return SliceOffset<ElementType>{.slice_offset = SliceOffset__build_from_sliceindex((int8*)p_memory, &p_slice_index)};
    };
};