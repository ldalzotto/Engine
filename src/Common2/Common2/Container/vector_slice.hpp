#pragma once

template <class ElementType> struct VectorSlice
{
    uimax Size;
    Slice<ElementType> Memory;

    inline static VectorSlice<ElementType> build(const Slice<ElementType>& p_memory, const uimax p_initial_size)
    {
        return VectorSlice<ElementType>{p_initial_size, p_memory};
    };

    inline Slice<ElementType> to_slice() const
    {
        return Slice<ElementType>::build_memory_elementnb(this->Memory.Begin, this->Size);
    };

    inline ElementType* get_memory()
    {
        return this->Memory.Begin;
    };

    inline uimax get_size()
    {
        return this->Size;
    };

    inline uimax get_capacity()
    {
        return this->Memory.Size;
    };

    inline int8 empty()
    {
        return this->Size == 0;
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
        return ((VectorSlice<ElementType>*)this)->get(p_index);
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
#if __DEBUG
        assert_true(this->Size + p_elements.Size <= this->Memory.Size);
#endif
        this->Memory.copy_memory_at_index(this->Size, p_elements);
        this->Size += p_elements.Size;

        return 1;
    };

    inline int8 push_back_array_2(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1)
    {
#if __DEBUG
        assert_true(this->Size + p_elements_0.Size + p_elements_1.Size <= this->Memory.Size);
#endif
        this->Memory.slice.copy_memory_at_index_2(this->Size, p_elements_0, p_elements_1);
        this->Size += (p_elements_0.Size + p_elements_1.Size);

        return 1;
    };

    inline int8 push_back_array_3(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
#if __DEBUG
        assert_true(this->Size + p_elements_0.Size + p_elements_1.Size + p_elements_2.Size <= this->Memory.Size);
#endif
        this->Memory.slice.copy_memory_at_index_3(this->Size, p_elements_0, p_elements_1, p_elements_2);
        this->Size += (p_elements_0.Size + p_elements_1.Size + p_elements_2.Size);

        return 1;
    };

    inline int8 push_back_array_empty(const uimax p_array_size)
    {
#if __DEBUG
        assert_true(this->Size + p_array_size <= this->Memory.Size);
#endif
        this->Size += p_array_size;
        return 1;
    };

    inline int8 push_back_element_empty()
    {
#if __DEBUG
        assert_true(this->Size + 1 <= this->Memory.Size);
#endif
        this->Size += 1;
        return 1;
    };

    inline int8 push_back_element(const ElementType& p_element)
    {
#if __DEBUG
        assert_true(this->Size + 1 <= this->Memory.Size);
#endif
        this->Memory.Begin[this->Size] = p_element;
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
        this->Memory.move_memory_down(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.move_memory_up(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline int8 insert_element_at_unchecked(const ElementType& p_element, const uimax p_index)
    {
        this->move_memory_down(p_index, 1);
        this->Memory[p_index] = p_element;
        this->Size += 1;

        return 1;
    };

    inline int8 insert_array_at_unchecked(const Slice<ElementType>& p_elements, const uimax p_index)
    {
        this->move_memory_down(p_index, p_elements.Size);
        this->Memory.copy_memory_at_index(p_index, p_elements);

        this->Size += p_elements.Size;

        return 1;
    };
};
