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
        Every nested vectors can be altered by using the Element_iVector_v2 structure.
*/
// TODO -> making the VectorOfVector usable with the iVector
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
        return this->get_as_ivector(p_index).to_slice();
    };

    inline void set(const uimax p_index, const Slice<ElementType>& p_element)
    {
        Element_iVector_v2 l_element = this->get_as_ivector(p_index);
        l_element.clear();
        l_element.push_back_array(p_element);
    };

    inline VectorOfVector_VectorHeader* get_vectorheader(const uimax p_index)
    {
        return (VectorOfVector_VectorHeader*)this->varying_vector.get_element(p_index).Begin;
    };

    /*
            Element_iVector is a wrapper around a VectorOfVector that acts like a regular Vector.
            It can be used in some templated algorithm that uses Vector.
    */
    struct Element_iVector_v2
    {
        VectorOfVector<ElementType>* vectorOfVector;
        uimax nestedvector_index;

        using t_Element = ElementType&;
        using t_ElementValue = ElementType;

        inline static Element_iVector_v2 build(VectorOfVector<ElementType>* p_vector_of_vector, const uimax p_index)
        {
            Element_iVector_v2 l_ivector;
            l_ivector.vectorOfVector = p_vector_of_vector;
            l_ivector.nestedvector_index = p_index;
            return l_ivector;
        };

        inline uimax get_size() const
        {
            return this->vectorOfVector->get_vectorheader(this->nestedvector_index)->Size;
        };

        inline void set_size(const uimax p_size)
        {
            this->vectorOfVector->get_vectorheader(this->nestedvector_index)->Size = p_size;
        };

        inline uimax get_capacity() const
        {
            return this->vectorOfVector->get_vectorheader(this->nestedvector_index)->Capacity;
        };

        iVector_functions_forward_declare(Element_iVector_v2);

        inline void set(const uimax p_index, const ElementType& p_element)
        {
            this->vectorOfVector->get(this->nestedvector_index).get(p_index) = p_element;
        };

        inline void set_unchecked(const uimax p_index, const ElementType& p_element)
        {
            this->vectorOfVector->get(this->nestedvector_index).get(p_index) = p_element;
        };

        inline ElementType& get_unchecked(const uimax p_index)
        {
            return this->vectorOfVector->get(this->nestedvector_index).get(p_index);
        };

        inline Slice<ElementType> to_slice() const
        {
            Slice<int8> l_element = this->vectorOfVector->varying_vector.get_element(this->nestedvector_index);
            VectorOfVector_VectorHeader* l_header = (VectorOfVector_VectorHeader*)l_element.Begin;
            return Slice<ElementType>::build_memory_elementnb((ElementType*)l_element.slide_rv(VectorOfVector_VectorHeader::get_vector_offset()).Begin, l_header->Size);
        };

        inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
        {
            // TODO -> is there a way to simulate this with insert empty instead ? Yes, with a VectorSlice
            VectorOfVector_VectorHeader* l_header = this->vectorOfVector->get_vectorheader(this->nestedvector_index);
            Slice<int8> l_source = l_header->get_vector_to_size<ElementType>().slide_rv(p_break_index).build_asint8();

            this->vectorOfVector->varying_vector.element_movememory(this->nestedvector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)),
                                                                    l_source);
        };

        inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
        {
            // TODO -> is there a way to simulate this with insert empty instead ? Yes, with a VectorSlice
            VectorOfVector_VectorHeader* l_header = this->vectorOfVector->get_vectorheader(this->nestedvector_index);
            Slice<int8> l_source = l_header->get_vector_to_size<ElementType>().slide_rv(p_break_index).build_asint8();

            // TODO -> is there a way to simulate this with insert empty instead ? Yes, with a VectorSlice
            this->vectorOfVector->varying_vector.element_movememory(this->nestedvector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_break_index - p_move_delta, sizeof(ElementType)),
                                                                    l_source);
        };

        inline void copy_memory_at_index(const uimax p_copy_index, const Slice<ElementType>& p_elements)
        {
            this->vectorOfVector->varying_vector.element_writeto(this->nestedvector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_copy_index, sizeof(ElementType)),
                                                                 Slice<ElementType>::build_asint8_memory_elementnb(p_elements.Begin, p_elements.Size));
        };

        inline void copy_memory_at_index_2(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
        {
            this->vectorOfVector->varying_vector.element_writeto_2(this->nestedvector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_copy_index, sizeof(ElementType)),
                                                                   Slice<ElementType>::build_asint8_memory_elementnb(p_elements_1.Begin, p_elements_1.Size),
                                                                   Slice<ElementType>::build_asint8_memory_elementnb(p_elements_2.Begin, p_elements_2.Size));
        };

        inline void copy_memory_at_index_3(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
        {
            this->varying_vector->varying_vector.element_writeto_2(this->nestedvector_index, VectorOfVector_VectorHeader::get_vector_element_offset(p_copy_index, sizeof(ElementType)),
                                                                   Slice<ElementType>::build_asint8_memory_elementnb(p_elements_1.Begin, p_elements_1.Size),
                                                                   Slice<ElementType>::build_asint8_memory_elementnb(p_elements_2.Begin, p_elements_2.Size),
                                                                   Slice<ElementType>::build_asint8_memory_elementnb(p_elements_3.Begin, p_elements_3.Size));
        };

        inline void resize_until_capacity_met(const uimax p_desired_capacity)
        {
            VectorOfVector_VectorHeader* l_vector_header = this->vectorOfVector->get_vectorheader(this->nestedvector_index);
            if (p_desired_capacity > l_vector_header->Capacity)
            {
                // const uimax p_new_size_bytes = sizeof(VectorOfVector_VectorHeader) + (p_new_size * sizeof(ElementType));
                const uimax p_size_delta = (p_desired_capacity - l_vector_header->Capacity) * sizeof(ElementType);
                this->vectorOfVector->varying_vector.element_expand_delta(this->nestedvector_index, p_size_delta);

                // /!\ because we potentially reallocate the p_vector_of_vector, we nee to requery for the VectorOfVector_VectorHeader
                l_vector_header = this->vectorOfVector->get_vectorheader(this->nestedvector_index);
                l_vector_header->Capacity = p_desired_capacity;
            }

            if (l_vector_header->Size > p_desired_capacity)
            {
                l_vector_header->Size = p_desired_capacity;
            }
        };
    };

    inline Element_iVector_v2 get_as_ivector(const uimax p_nested_vector_index)
    {
        return Element_iVector_v2::build(this, p_nested_vector_index);
    };
};