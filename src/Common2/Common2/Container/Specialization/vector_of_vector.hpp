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

    template <class ElementType> inline Slice<ElementType> get_vector_to_size() const
    {
        return Slice<ElementType>::build_memory_elementnb((ElementType*)((int8*)this + sizeof(VectorOfVector_VectorHeader)), this->Size);
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
        VectorOfVector_VectorHeader* l_header = (VectorOfVector_VectorHeader*)l_element.Begin;
        return Slice<ElementType>::build_memory_elementnb((ElementType*)l_element.slide_rv(VectorOfVector_VectorHeader::get_vector_offset()).Begin, l_header->Size);
    };

    // TODO -> remove
    inline void set(const uimax p_index, const Slice<ElementType>& p_element)
    {
        this->element_clear(p_index);
        // TODO -> remove for iVector v2
        this->element_push_back_array(p_index, p_element);
    };

    inline VectorOfVector_VectorHeader* get_vectorheader(const uimax p_index)
    {
        return (VectorOfVector_VectorHeader*)this->varying_vector.get_element(p_index).Begin;
    };

    // TODO -> write test
    inline void element_resize(const uimax p_nested_vector_index, const uimax p_new_size)
    {
        VectorOfVector_VectorHeader* l_vector_header = this->get_vectorheader(p_nested_vector_index);
        if (p_new_size > l_vector_header->Capacity)
        {
            // const uimax p_new_size_bytes = sizeof(VectorOfVector_VectorHeader) + (p_new_size * sizeof(ElementType));
            const uimax p_size_delta = (p_new_size - l_vector_header->Capacity) * sizeof(ElementType);
            this->varying_vector.element_expand_delta(p_nested_vector_index, p_size_delta);

            // /!\ because we potentially reallocate the p_vector_of_vector, we nee to requery for the VectorOfVector_VectorHeader
            l_vector_header = this->get_vectorheader(p_nested_vector_index);
            l_vector_header->Capacity = p_new_size;
        }

        if (l_vector_header->Size > p_new_size)
        {
            l_vector_header->Size = p_new_size;
        }
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

    // TODO -> remove for iVector v2
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

    // TODO -> remove for iVector v2
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

    // TODO -> remove for iVector v2
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

    // TODO -> remove for iVector v2
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

        using t_ElementValue = ElementType;
        using t_SizeType = uimax;

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

    /*
            Element_iVector is a wrapper around a VectorOfVector that acts like a regular Vector.
            It can be used in some templated algorithm that uses Vector.
    */
    struct Element_iVector_v2
    {
        VectorOfVector<ElementType>* vectorOfVector;
        uimax Index;

        using t_Element = ElementType&;
        using t_ElementValue = ElementType;

        inline static Element_iVector_v2 build(VectorOfVector<ElementType>* p_vector_of_vector, const uimax p_index)
        {
            Element_iVector_v2 l_ivector;
            l_ivector.vectorOfVector = p_vector_of_vector;
            l_ivector.Index = p_index;
            return l_ivector;
        };

        inline uimax get_size() const
        {
            return this->vectorOfVector->get_vectorheader(this->Index)->Size;
        };

        inline void set_size(const uimax p_size)
        {
            this->vectorOfVector->get_vectorheader(this->Index)->Size = p_size;
        };

        inline uimax get_capacity() const
        {
            return this->vectorOfVector->get_vectorheader(this->Index)->Capacity;
        };

        inline void set(const uimax p_index, const ElementType& p_element)
        {
            this->vectorOfVector->get(this->Index).get(p_index) = p_element;
        };

        inline void set_unchecked(const uimax p_index, const ElementType& p_element)
        {
            this->vectorOfVector->get(this->Index).get(p_index) = p_element;
        };

        inline ElementType& get_unchecked(const uimax p_index)
        {
            return this->vectorOfVector->get(this->Index).get(p_index);
        };

        inline Slice<ElementType> to_slice() const
        {
            return this->vectorOfVector->get(this->Index);
        };

        inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
        {
            this->vectorOfVector->element_movememory_down(this->Index, *this->vectorOfVector->get_vectorheader(this->Index), p_break_index, p_move_delta);
        };

        inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
        {
            this->vectorOfVector->element_movememory_up(this->Index, *this->vectorOfVector->get_vectorheader(this->Index), p_break_index, p_move_delta);
        };

        inline void copy_memory_at_index(const uimax p_copy_index, const Slice<ElementType>& p_elements)
        {
            this->vectorOfVector->element_write_array(this->Index, p_copy_index, p_elements);
        };

        inline void copy_memory_at_index_2(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
        {
            this->vectorOfVector->element_write_array_2(this->Index, p_copy_index, p_elements_1, p_elements_2);
        };

        inline void copy_memory_at_index_3(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
        {
            this->vectorOfVector->element_write_array_3(this->Index, p_copy_index, p_elements_1, p_elements_2, p_elements_3);
        };

        inline void resize_until_capacity_met(const uimax p_desired_capacity)
        {
            this->vectorOfVector->element_resize(this->Index, p_desired_capacity);
        };
    };

    inline Element_iVector element_as_iVector(const uimax p_nested_vector_index)
    {
        return Element_iVector::build(this, p_nested_vector_index);
    };

    inline Element_iVector_v2 element_as_iVector_v2(const uimax p_nested_vector_index)
    {
        return Element_iVector_v2::build(this, p_nested_vector_index);
    };

    // TODO -> is there a way to simulate this with insert empty instead ? Yes, with a VectorSlice
    inline void element_movememory_up(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<int8> l_source = p_nested_vector_header.get_vector_to_size<ElementType>().slide_rv(p_break_index).build_asint8();

        this->varying_vector.element_movememory(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index - p_move_delta, sizeof(ElementType)), l_source);
    };

    // TODO -> is there a way to simulate this with insert empty instead ? Yes, with a VectorSlice
    inline void element_movememory_down(const uimax p_nested_vector_index, const VectorOfVector_VectorHeader& p_nested_vector_header, const uimax p_break_index, const uimax p_move_delta)
    {
        Slice<int8> l_source = p_nested_vector_header.get_vector_to_size<ElementType>().slide_rv(p_break_index).build_asint8();

        this->varying_vector.element_movememory(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)), l_source);
    };

  private:
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

    inline void element_write_array_2(const uimax p_nested_vector_index, const uimax p_write_start_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        this->varying_vector.element_writeto_2(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
                                               Slice<ElementType>::build_asint8_memory_elementnb(p_elements_1.Begin, p_elements_1.Size),
                                               Slice<ElementType>::build_asint8_memory_elementnb(p_elements_2.Begin, p_elements_2.Size));
    };

    inline void element_write_array_3(const uimax p_nested_vector_index, const uimax p_write_start_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2,
                                      const Slice<ElementType>& p_elements_3)
    {
        this->varying_vector.element_writeto_2(p_nested_vector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
                                               Slice<ElementType>::build_asint8_memory_elementnb(p_elements_1.Begin, p_elements_1.Size),
                                               Slice<ElementType>::build_asint8_memory_elementnb(p_elements_2.Begin, p_elements_2.Size),
                                               Slice<ElementType>::build_asint8_memory_elementnb(p_elements_3.Begin, p_elements_3.Size));
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