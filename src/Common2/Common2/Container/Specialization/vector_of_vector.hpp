#pragma once

/*
    The header of every vector of the VectorOfVector. That indicates the associated memory chunk state.
*/
struct VectorOfVector_VectorHeader
{
    uimax Size;
    uimax Capacity;

    inline static VectorOfVector_VectorHeader build(const uimax p_size, const uimax p_capacity)
    {
        return VectorOfVector_VectorHeader{p_size, p_capacity};
    };

    inline static VectorOfVector_VectorHeader build_default()
    {
        return build(0, 0);
    };

    inline static Span<int8> allocate(const Slice<int8>& p_vector_slice, const uimax p_vector_size)
    {
        Span<int8> l_allocated_element = Span<int8>::allocate(sizeof(VectorOfVector_VectorHeader) + p_vector_slice.Size);
        VectorOfVector_VectorHeader l_vector_header = build(p_vector_size, p_vector_size);
        l_allocated_element.slice.copy_memory_at_index(0, Slice<VectorOfVector_VectorHeader>::build_asint8_memory_elementnb(&l_vector_header, 1));
        l_allocated_element.slice.copy_memory_at_index(sizeof(l_vector_header), p_vector_slice);
        return l_allocated_element;
    };

    template <class ElementType> inline static Span<int8> allocate_vectorelements(const Slice<ElementType>& p_vector_elements)
    {
        return allocate(p_vector_elements.build_asint8(), p_vector_elements.Size);
    };

    inline static uimax get_vector_offset()
    {
        return sizeof(VectorOfVector_VectorHeader);
    };

    inline static uimax get_vector_element_offset(const uimax p_element_index, const uimax p_element_size)
    {
        return get_vector_offset() + (p_element_index * p_element_size);
    };

    template <class ElementType> inline Slice<ElementType> get_vector_to_capacity() const
    {
        return Slice<ElementType>::build_memory_elementnb(cast(ElementType*, cast(int8*, this) + sizeof(VectorOfVector_VectorHeader)), this->Capacity);
    };
};

/*
        A VectorOfVector is a chain of resizable Vector allocated on the same memory block.
        Every nested vectors can be altered with "vectorofvector_element_*" functions.
*/
// TODO -> having a way to have different ElementType for every nested vectors ? To facilitate the fact of grouping adjacent containers.
template <class ElementType> struct VectorOfVector
{
    VaryingVector varying_vector;

    inline static VectorOfVector<ElementType> allocate_default()
    {
        return VectorOfVector<ElementType>{VaryingVector::allocate_default()};
    };

    inline void free()
    {
        this->varying_vector.free();
    };

    inline uimax get_size()
    {
        return this->varying_vector.get_size();
    };

    inline void push_back_element_empty()
    {
        VectorOfVector_VectorHeader l_header = VectorOfVector_VectorHeader::build_default();
        Slice<int8> l_header_slice = Slice<VectorOfVector_VectorHeader>::build_asint8_memory_singleelement(&l_header);
        this->varying_vector.push_back(l_header_slice);
    };

    inline void push_back_element(const Slice<ElementType>& p_vector_elements)
    {
        VectorOfVector_VectorHeader l_header = VectorOfVector_VectorHeader::build(p_vector_elements.Size, p_vector_elements.Size);
        this->varying_vector.push_back_2(Slice<VectorOfVector_VectorHeader>::build_asint8_memory_singleelement(&l_header), p_vector_elements.build_asint8());
    };

    inline void insert_empty_at(const uimax p_index)
    {
        VectorOfVector_VectorHeader l_header = VectorOfVector_VectorHeader::build_default();
        this->varying_vector.insert_at(Slice<VectorOfVector_VectorHeader>::build_asint8_memory_singleelement(&l_header), p_index);
    };

    inline void erase_element_at(const uimax p_index)
    {
        this->varying_vector.erase_element_at(p_index);
    };

    inline void erase_element_at_always(const uimax p_index)
    {
        this->varying_vector.erase_element_at_always(p_index);
    };

    inline Slice<ElementType> get(const uimax p_index)
    {
        Slice<int8> l_element = this->varying_vector.get_element(p_index);
        VectorOfVector_VectorHeader* l_header = cast(VectorOfVector_VectorHeader*, l_element.Begin);
        return Slice<ElementType>::build_memory_elementnb(cast(ElementType*, l_element.slide_rv(VectorOfVector_VectorHeader::get_vector_offset()).Begin), l_header->Size);
    };

    inline void set(const uimax p_index, const Slice<ElementType>& p_element)
    {
        this->element_clear(p_index);
        this->element_push_back_array(p_index, p_element);
    };

    inline VectorOfVector_VectorHeader* get_vectorheader(const uimax p_index)
    {
        return cast(VectorOfVector_VectorHeader*, this->varying_vector.get_element(p_index).Begin);
    };

    inline void element_push_back_element(const uimax p_nested_vector_index, const ElementType& p_element)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

        if (l_vector_header->Size + 1 > l_vector_header->Capacity)
        {
            this->varying_vector.element_expand_with_value(p_nested_vector_index, Slice<ElementType>::build_asint8_memory_elementnb(&p_element, 1));

            // /!\ because we potentially reallocate the p_vector_of_vector, we nee to requery for the VectorOfVector_VectorHeader
            l_vector_header = this->get_vectorheader(p_nested_vector_index);
            l_vector_header->Capacity += 1;
        }
        else
        {
            this->element_write_element(p_nested_vector_index, l_vector_header->Size, p_element);
        }

        l_vector_header->Size += 1;
    };

    inline void element_push_back_array(const uimax p_nested_vector_index, const Slice<ElementType>& p_elements)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

        if (l_vector_header->Size == l_vector_header->Capacity)
        {
            this->varying_vector.element_expand_with_value(p_nested_vector_index, p_elements.build_asint8());

            l_vector_header = this->get_vectorheader(p_nested_vector_index);
            l_vector_header->Size += p_elements.Size;
            l_vector_header->Capacity += p_elements.Size;
        }
        else if (l_vector_header->Size + p_elements.Size > l_vector_header->Capacity)
        {
            uimax l_write_element_nb = l_vector_header->Capacity - l_vector_header->Size;
            uimax l_expand_element_nb = p_elements.Size - l_write_element_nb;

            this->element_write_array(p_nested_vector_index, l_vector_header->Size, Slice<ElementType>::build_memory_elementnb(p_elements.Begin, l_write_element_nb));

            this->varying_vector.element_expand_with_value(p_nested_vector_index, Slice<ElementType>::build_asint8_memory_elementnb(p_elements.Begin + l_write_element_nb, l_expand_element_nb));

            l_vector_header = this->get_vectorheader(p_nested_vector_index);
            l_vector_header->Size += p_elements.Size;
            l_vector_header->Capacity += l_expand_element_nb;
        }
        else
        {
            this->element_write_array(p_nested_vector_index, l_vector_header->Size, p_elements);

            l_vector_header->Size += p_elements.Size;
        }
    };

    inline void element_insert_element_at(const uimax p_nested_vector_index, const uimax p_index, const ElementType& p_element)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

#if __DEBUG
        assert_true(p_index != l_vector_header->Size); // use vectorofvector_element_push_back_element
        assert_true(p_index < l_vector_header->Size);
#endif

        if ((l_vector_header->Size + 1) > l_vector_header->Capacity)
        {
            this->element_movememory_down_and_resize(p_nested_vector_index, *l_vector_header, p_index, 1);
            l_vector_header = this->get_vectorheader(p_nested_vector_index);
        }
        else
        {
            this->element_movememory_down(p_nested_vector_index, *l_vector_header, p_index, 1);
        }

        l_vector_header->Size += 1;

        this->element_write_element(p_nested_vector_index, p_index, p_element);
    }

    inline void element_erase_element_at(const uimax p_nested_vector_index, const uimax p_index)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);

#if __DEBUG
        if (p_index == l_vector_header->Size)
        {
            abort();
        } // use vectorofvector_element_pop_back_element
        if (p_index > l_vector_header->Size)
        {
            abort();
        }
#endif
        this->element_erase_element_at_unchecked(p_nested_vector_index, p_index, l_vector_header);
    };

    inline void element_pop_back_element(const uimax p_nested_vector_index, const uimax p_index)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);
#if __DEBUG
        if (p_index != (l_vector_header->Size - 1))
        {
            abort(); // use element_erase_element_at
        }
#endif
        this->element_pop_back_element_unchecked(l_vector_header);
    };

    inline void element_erase_element_at_always(const uimax p_nested_vector_index, const uimax p_index)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);
#if __DEBUG
        if (p_index >= l_vector_header->Size)
        {
            abort();
        }
#endif
        if (p_index < l_vector_header->Size - 1)
        {
            this->element_erase_element_at_unchecked(p_nested_vector_index, p_index, l_vector_header);
        }
        else
        {
            this->element_pop_back_element_unchecked(l_vector_header);
        }
    };

    inline void element_clear(const uimax p_nested_vector_index)
    {
        this->get_vectorheader(p_nested_vector_index)->Size = 0;
    };

    /*
        Element_iVector is a wrapper around a VectorOfVector that acts like a regular Vector.
        It can be used in some templated algorithm that uses Vector.
    */
    struct Element_iVector
    {
        VectorOfVector<ElementType>* vectorOfVector;
        uimax Index;

        using _ElementValue = ElementType;
        using _SizeType = uimax;

        inline static Element_iVector build(VectorOfVector<ElementType>* p_vector_of_vector, const uimax p_index)
        {
            return Element_iVector{p_vector_of_vector, p_index};
        };

        inline uimax get_size() const
        {
            return this->vectorOfVector->get_vectorheader(this->Index)->Size;
        };

        inline ElementType& get(const uimax p_index)
        {
            return this->vectorOfVector->get(this->Index).get(p_index);
        };

        inline void erase_element_at(const uimax p_index)
        {
            this->vectorOfVector->element_erase_element_at(this->Index, p_index);
        };

        inline void erase_element_at_always(const uimax p_index)
        {
            this->vectorOfVector->element_erase_element_at_always(this->Index, p_index);
        };

        inline void push_back_element(const ElementType& p_index)
        {
            this->vectorOfVector->element_push_back_element(this->Index, p_index);
        };

        inline Slice<ElementType> to_slice() const
        {
            return this->vectorOfVector->get(this->Index);
        };

        inline iVector<Element_iVector> to_ivector()
        {
            return iVector<Element_iVector>{*this};
        };
    };

    inline Element_iVector element_as_iVector(const uimax p_nested_vector_index)
    {
        return Element_iVector::build(this, p_nested_vector_index);
    };

  private:
    inline void element_movememory_up(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<int8> l_source = p_nested_vector_header.get_vector_to_capacity<ElementType>().slide_rv(p_break_index).build_asint8();

        this->varying_vector.element_movememory(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index - p_move_delta, sizeof(ElementType)), l_source);
    };

    inline void element_movememory_down(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<int8> l_source = p_nested_vector_header.get_vector_to_capacity<ElementType>().slide_rv(p_break_index).build_asint8();

        this->varying_vector.element_movememory(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)), l_source);
    };

    inline void element_movememory_down_and_resize(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<int8> l_source = p_nested_vector_header.get_vector_to_capacity<ElementType>().slide_rv(p_break_index).build_asint8();

        this->varying_vector.element_expand(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)) + l_source.Size);

        this->varying_vector.element_movememory(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)), l_source);
    };

    inline void element_write_element(const uimax p_nested_vector_index, const uimax p_write_start_index, const ElementType& p_element)
    {
        this->varying_vector.element_writeto(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
                                             Slice<ElementType>::build_asint8_memory_singleelement(&p_element));
    };

    inline void element_write_array(const uimax p_nested_vector_index, const uimax p_write_start_index, const Slice<ElementType>& p_elements)
    {
        this->varying_vector.element_writeto(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
                                             Slice<ElementType>::build_asint8_memory_elementnb(p_elements.Begin, p_elements.Size));
    };

    inline void element_erase_element_at_unchecked(const uimax p_nested_vector_index, const uimax p_index, VectorOfVector_VectorHeader* p_vector_header)
    {
        this->element_movememory_up(p_nested_vector_index, *p_vector_header, p_index + 1, 1);
        p_vector_header->Size -= 1;
    };

    inline void element_pop_back_element_unchecked(VectorOfVector_VectorHeader* p_vector_header)
    {
        p_vector_header->Size -= 1;
    };
};