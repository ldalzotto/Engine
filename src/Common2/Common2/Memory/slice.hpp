#pragma once

/*
    A Slice is an encapsulated C style array.
*/
template <class ElementType> struct Slice
{
    uimax Size;
    ElementType* Begin;

    inline static Slice<ElementType> build_default()
    {
        return Slice<ElementType>{0, NULL};
    };

    inline static Slice<ElementType> build_begin_end(ElementType* p_memory, const uimax p_begin, const uimax p_end)
    {
        return Slice<ElementType>{p_end - p_begin, p_memory + p_begin};
    };

    inline static Slice<ElementType> build_memory_elementnb(ElementType* p_memory, const uimax p_element_nb)
    {
        return Slice<ElementType>{p_element_nb, p_memory};
    };

    inline static Slice<ElementType> build_memory_offset_elementnb(ElementType* p_memory, const uimax p_offset, const uimax p_element_nb)
    {
        return Slice<ElementType>{p_element_nb, p_memory + p_offset};
    };

    inline static Slice<int8> build_asint8(ElementType* p_memory, const uimax p_begin, const uimax p_end)
    {
        return Slice<int8>{sizeof(ElementType) * (p_end - p_begin), cast(int8*, (p_memory + p_begin))};
    };

    inline Slice<int8> build_asint8() const
    {
        return Slice<int8>{sizeof(ElementType) * this->Size, cast(int8*, this->Begin)};
    };

    inline static Slice<int8> build_asint8_memory_elementnb(const ElementType* p_memory, const uimax p_element_nb)
    {
        return Slice<int8>{sizeof(ElementType) * p_element_nb, cast(int8*, p_memory)};
    };

    inline static Slice<int8> build_asint8_memory_singleelement(const ElementType* p_memory)
    {
        return Slice<int8>{sizeof(ElementType), cast(int8*, p_memory)};
    };

    inline ElementType& get(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif
        return this->Begin[p_index];
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((Slice<ElementType>*)this)->get(p_index);
    };

    inline void slide(const uimax p_offset_index)
    {
#if __DEBUG
        this->bound_check(p_offset_index);
#endif

        this->Begin = this->Begin + p_offset_index;
        this->Size -= p_offset_index;
    };

    inline Slice<ElementType> slide_rv(const uimax p_offset_index) const
    {
        Slice<ElementType> l_return = *this;
        l_return.slide(p_offset_index);
        return l_return;
    };

    inline int8 compare(const Slice<ElementType>& p_other) const
    {
        return memory_compare(cast(int8*, this->Begin), cast(int8*, p_other.Begin), p_other.Size * sizeof(ElementType));
    };

    inline int8 find(const Slice<ElementType>& p_other, uimax* out_index) const
    {
#if __DEBUG
        if (p_other.Size > this->Size)
        {
            abort();
        }
#endif

        Slice<ElementType> l_target_slice = *this;
        if (l_target_slice.compare(p_other))
        {
            *out_index = 0;
            return 1;
        };

        for (uimax i = 1; i < this->Size - p_other.Size + 1; i++)
        {
            l_target_slice.slide(1);
            if (l_target_slice.compare(p_other))
            {
                *out_index = i;
                return 1;
            };
        };

        *out_index = -1;
        return 0;
    };

    inline int8* move_memory(const Slice<ElementType>& p_source)
    {
#if __DEBUG
        return memory_move_safe(cast(int8*, this->Begin), this->Size * sizeof(ElementType), cast(int8*, p_source.Begin), p_source.Size * sizeof(ElementType));
#else
        return memory_move((int8*)this->Begin, (int8*)p_source.Begin, p_source.Size * sizeof(ElementType));
#endif
    };

    inline void move_memory_down(const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<ElementType> l_target = Slice<ElementType>::build_memory_offset_elementnb(this->Begin, p_break_index + p_move_delta, p_moved_block_size);
#if __DEBUG
        this->bound_inside_check(l_target);
#endif
        Slice<ElementType> l_source = Slice<ElementType>::build_begin_end(this->Begin, p_break_index, p_break_index + p_moved_block_size);
        l_target.move_memory(l_source);
    };

    inline void move_memory_up(const uimax p_moved_block_size, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<ElementType> l_target = Slice<ElementType>::build_memory_offset_elementnb(this->Begin, p_break_index - p_move_delta, p_moved_block_size);
#if __DEBUG
        this->bound_inside_check(l_target);
#endif
        Slice<ElementType> l_source = Slice<ElementType>::build_begin_end(this->Begin, p_break_index, p_break_index + p_moved_block_size);
        l_target.move_memory(l_source);
    };

    inline int8* copy_memory(const Slice<ElementType>& p_elements)
    {
#if __DEBUG
        return memory_cpy_safe(cast(int8*, this->Begin), this->Size * sizeof(ElementType), cast(int8*, p_elements.Begin), p_elements.Size * sizeof(ElementType));
#else
        return memory_cpy((int8*)this->Begin, (int8*)p_elements.Begin, p_elements.Size * sizeof(ElementType));
#endif
    };

    inline void copy_memory_at_index(const uimax p_copy_index, const Slice<ElementType>& p_elements)
    {
        Slice<ElementType> l_target = Slice<ElementType>::build_memory_elementnb(this->Begin + p_copy_index, p_elements.Size);

#if __DEBUG
        this->bound_inside_check(l_target);
#endif

        l_target.copy_memory(p_elements);
    };

    inline void copy_memory_at_index_2(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        this->copy_memory_at_index(p_copy_index, p_elements_1);
        this->copy_memory_at_index(p_copy_index + p_elements_1.Size, p_elements_2);
    };

    inline void copy_memory_at_index_3(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
    {
        this->copy_memory_at_index_2(p_copy_index, p_elements_1, p_elements_2);
        this->copy_memory_at_index(p_copy_index + p_elements_1.Size + p_elements_2.Size, p_elements_3);
    };

    inline void zero()
    {
#if __DEBUG
        memory_zero_safe((int8*)this->Begin, this->Size * sizeof(ElementType), this->Size * sizeof(ElementType));
#else
        memory_zero((int8*)this->Begin, this->Size * sizeof(ElementType));
#endif
    };

    inline void bound_inside_check(const Slice<ElementType>& p_tested_slice)
    {
#if __DEBUG
        if ((p_tested_slice.Begin + p_tested_slice.Size) > (this->Begin + this->Size))
        {
            abort();
        }
#endif
    };

    inline void bound_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index > this->Size)
        {
            abort();
        }
#endif
    };

    inline void assert_null_terminated() const
    {
#if __DEBUG
        assert_true(sizeof(ElementType) == sizeof(int8));
        assert_true(this->get(this->Size - 1) == '\0');
#endif
    };
};

inline Slice<int8> slice_int8_build_rawstr(const int8* p_str)
{
    return Slice<int8>::build_memory_elementnb((int8*)p_str, strlen(p_str));
};

inline Slice<int8> slice_int8_build_rawstr_with_null_termination(const int8* p_str)
{
    return Slice<int8>::build_memory_elementnb((int8*)p_str, strlen(p_str) + 1);
};

template <class CastedType> inline Slice<CastedType> slice_cast(const Slice<int8>& p_slice)
{
#if __DEBUG
    if ((p_slice.Size % sizeof(CastedType)) != 0)
    {
        abort();
    }
#endif

    return Slice<CastedType>{cast(uimax, p_slice.Size / sizeof(CastedType)), cast(CastedType*, p_slice.Begin)};
};

template <class CastedType> inline CastedType* slice_cast_singleelement(const Slice<int8>& p_slice)
{
#if __DEBUG
    if (p_slice.Size < sizeof(CastedType))
    {
        abort();
    }
#endif
    return cast(CastedType*, p_slice.Begin);
};

template <class CastedType> inline Slice<CastedType> slice_cast_fixedelementnb(const Slice<int8>& p_slice, const uimax p_element_nb)
{
#if __DEBUG
    if (p_slice.Size < (sizeof(CastedType) * p_element_nb))
    {
        abort();
    }
#endif

    return slice_build_memory_elementnb(cast(CastedType*, p_slice.Begin), p_element_nb);
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

template <class ElementType, uint32 Size_t> struct SliceN
{
    ElementType Memory[Size_t];

    inline uimax Size()
    {
        return Size_t;
    };

    inline ElementType& get(const uimax p_index)
    {
#if __DEBUG
        assert_true(p_index < Size_t);
#endif
        return this->Memory[p_index];
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((SliceN<ElementType, Size_t>*)this)->get(p_index);
    }
};

template <class ElementType, uint32 Size_t> inline Slice<ElementType> slice_from_slicen(const SliceN<ElementType, Size_t>* p_slice_n)
{
    return Slice<ElementType>{Size_t, (ElementType*)p_slice_n->Memory};
};

/*
    A SliceIndex is just a begin and end uimax
*/
struct SliceIndex
{
    uimax Begin;
    uimax Size;

    inline static SliceIndex build(const uimax p_begin, const uimax p_size)
    {
        return SliceIndex{p_begin, p_size};
    };

    inline static SliceIndex build_default()
    {
        return build(0, 0);
    };

    inline void slice_two(const uimax p_break_point, SliceIndex* out_left, SliceIndex* out_right) const
    {
        uimax l_source_initial_size = this->Size;
        *out_left = SliceIndex::build(this->Begin, p_break_point - this->Begin);
        *out_right = SliceIndex::build(p_break_point, l_source_initial_size - out_left->Size);
    };
};

template <class ElementType> struct SliceOffset
{
    uimax Offset;
    ElementType* Memory;

    inline static SliceOffset<ElementType> build(ElementType* p_memory, const uimax p_offset)
    {
        return SliceOffset<ElementType>{p_offset, p_memory};
    };

    inline static SliceOffset<ElementType> build_from_sliceindex(ElementType* p_memory, const SliceIndex& p_slice_index)
    {
        return SliceOffset<ElementType>{p_slice_index.Begin, p_memory};
    };
};