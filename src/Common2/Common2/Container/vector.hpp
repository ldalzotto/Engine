#pragma once

/*
    A Vector is a Span with an imaginary boundary (Size).
    Vector memory is continuous, there is no "hole" between items.
    Vector is reallocated on need.
    Any memory access outside of this imaginary boundary will be in error.
    The Vector expose some safe way to insert/erase data (array or single element).
*/
template <class ElementType> struct Vector
{
    uimax Size;
    Span<ElementType> Memory;

    using t_Element = ElementType&;
    using t_ElementValue = ElementType;
    using t_SizeType = uimax;

    inline static Vector<ElementType> build_zero_size(ElementType* p_memory, const uimax p_initial_capacity)
    {
        return Vector<ElementType>{0, Span<ElementType>::build(p_memory, p_initial_capacity)};
    };

    inline static Vector<ElementType> allocate(const uimax p_initial_capacity)
    {
        return Vector<ElementType>{0, Span<ElementType>::allocate(p_initial_capacity)};
    };

    inline static Vector<ElementType> allocate_elements(const Slice<ElementType>& p_initial_elements)
    {
        Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_initial_elements.Size);
        l_vector.push_back_array(p_initial_elements);
        return l_vector;
    };

    inline static Vector<ElementType> allocate_capacity_elements(const uimax p_inital_capacity, const Slice<ElementType>& p_initial_elements)
    {
        Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_inital_capacity);
        l_vector.push_back_array(p_initial_elements);
        return l_vector;
    };

    inline static Vector<ElementType> allocate_capacity_elements_2(const uimax p_inital_capacity, const Slice<ElementType>& p_initial_elements_0, const Slice<ElementType>& p_initial_elements_1)
    {
        Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_inital_capacity);
        l_vector.push_back_array_2(p_initial_elements_0, p_initial_elements_1);
        return l_vector;
    };

    inline static Vector<ElementType> allocate_capacity_elements_3(const uimax p_inital_capacity, const Slice<ElementType>& p_initial_elements_0, const Slice<ElementType>& p_initial_elements_1,
                                                                   const Slice<ElementType>& p_initial_elements_2)
    {
        Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_inital_capacity);
        l_vector.push_back_array_3(p_initial_elements_0, p_initial_elements_1, p_initial_elements_2);
        return l_vector;
    };

    inline Slice<ElementType> to_slice() const
    {
        return Slice<ElementType>::build_memory_elementnb(this->Memory.Memory, this->Size);
    };

    inline void free()
    {
        this->Memory.free();
        *this = Vector<ElementType>::build_zero_size(NULL, 0);
    };

    inline ElementType* get_memory()
    {
        return this->Memory.Memory;
    };

    inline uimax get_size() const
    {
        return this->Size;
    };

    inline uimax get_capacity() const
    {
        return this->Memory.Capacity;
    };

    inline int8 empty() const
    {
        return this->Size == 0;
    };

    inline iVector<Vector<ElementType>> to_ivector()
    {
        return iVector<Vector<ElementType>>{*this};
    };

    inline ElementType& get(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index);
#endif
        return this->Memory.Memory[p_index];
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((Vector<ElementType>*)this)->get(p_index);
    };

    inline void set(const uimax p_index, const ElementType& p_element_type)
    {
        this->get(p_index) = p_element_type;
    };

    inline void clear()
    {
        this->Size = 0;
    };

    inline int8 insert_array_at(const Slice<ElementType>& p_elements, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

        return this->insert_array_at_unchecked(p_elements, p_index);
    };

    inline int8 insert_element_at(const ElementType& p_element, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif

        return this->insert_element_at_unchecked(p_element, p_index);
    };

    inline int8 push_back_array(const Slice<ElementType>& p_elements)
    {
        this->Memory.resize_until_capacity_met(this->Size + p_elements.Size);
        this->Memory.slice.copy_memory_at_index(this->Size, p_elements);
        this->Size += p_elements.Size;

        return 1;
    };

    inline int8 push_back_array_2(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1)
    {
        this->Memory.resize_until_capacity_met(this->Size + p_elements_0.Size + p_elements_1.Size);
        this->Memory.slice.copy_memory_at_index_2(this->Size, p_elements_0, p_elements_1);
        this->Size += (p_elements_0.Size + p_elements_1.Size);

        return 1;
    };

    inline int8 push_back_array_3(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        this->Memory.resize_until_capacity_met(this->Size + p_elements_0.Size + p_elements_1.Size + p_elements_2.Size);
        this->Memory.slice.copy_memory_at_index_3(this->Size, p_elements_0, p_elements_1, p_elements_2);
        this->Size += (p_elements_0.Size + p_elements_1.Size + p_elements_2.Size);

        return 1;
    };

    inline int8 push_back_array_empty(const uimax p_array_size)
    {
        this->Memory.resize_until_capacity_met(this->Size + p_array_size);
        this->Size += p_array_size;
        return 1;
    };

    inline int8 push_back_element_empty()
    {
        this->Memory.resize_until_capacity_met(this->Size + 1);
        this->Size += 1;
        return 1;
    };

    inline int8 push_back_element(const ElementType& p_element)
    {
        this->Memory.resize_until_capacity_met(this->Size + 1);
        this->Memory.Memory[this->Size] = p_element;
        this->Size += 1;

        return 1;
    };

    inline int8 insert_array_at_always(const Slice<ElementType>& p_elements, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif
        if (p_index == this->Size)
        {
            return this->push_back_array(p_elements);
        }
        else
        {
            return this->insert_array_at_unchecked(p_elements, p_index);
        }
    };

    inline int8 insert_element_at_always(const ElementType& p_element, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif

        if (p_index == this->Size)
        {
            return this->push_back_element(p_element);
        }
        else
        {
            return this->insert_element_at_unchecked(p_element, p_index);
        }
    };

    inline int8 erase_array_at(const uimax p_index, const uimax p_element_nb)
    {

#if __DEBUG
        this->bound_check(p_index);
        this->bound_check(p_index + p_element_nb);
        this->bound_lastelement_check(p_index); // use vector_pop_back_array
#endif

        this->move_memory_up(p_index + p_element_nb, p_element_nb);
        this->Size -= p_element_nb;

        return 1;
    };

    inline int8 erase_element_at(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_lastelement_check(p_index); // use vector_pop_back
#endif

        this->move_memory_up(p_index + 1, 1);
        this->Size -= 1;

        return 1;
    };

    inline int8 pop_back_array(const uimax p_element_nb)
    {
        this->Size -= p_element_nb;
        return 1;
    };

    inline int8 pop_back()
    {
        this->Size -= 1;
        return 1;
    };

    inline int8 erase_element_at_always(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif

        if (p_index == this->Size - 1)
        {
            return this->pop_back();
        }
        else
        {
            return this->erase_element_at(p_index);
        }
    };

    inline void erase_array_at_always(const uimax p_index, const uimax p_size)
    {
        uimax l_insert_head = p_index + p_size;
#if __DEBUG
        this->bound_check(l_insert_head);
#endif

        if (l_insert_head == this->Size - 1)
        {
            this->pop_back_array(p_size);
        }
        else
        {
            this->erase_array_at(p_index, p_size);
        }
    };

    template <class Predicate_t> inline void erase_if(const Predicate_t& p_predicate)
    {
        this->to_ivector().erase_if(p_predicate);
    };

  private:
    inline void bound_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index > this->Size)
        {
            abort();
        }
#endif
    };

    inline void bound_head_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index == this->Size)
        {
            abort();
        }
#endif
    };

    inline void bound_lastelement_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index == this->Size - 1)
        {
            abort();
        }
#endif
    };

    inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.slice.move_memory_down(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.slice.move_memory_up(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline int8 insert_element_at_unchecked(const ElementType& p_element, const uimax p_index)
    {
        this->Memory.resize_until_capacity_met(this->Size + 1);
        this->move_memory_down(p_index, 1);
        this->Memory.Memory[p_index] = p_element;
        this->Size += 1;

        return 1;
    };

    inline int8 insert_array_at_unchecked(const Slice<ElementType>& p_elements, const uimax p_index)
    {
        this->Memory.resize_until_capacity_met(this->Size + p_elements.Size);
        this->move_memory_down(p_index, p_elements.Size);
        this->Memory.slice.copy_memory_at_index(p_index, p_elements);

        this->Size += p_elements.Size;

        return 1;
    };
};
