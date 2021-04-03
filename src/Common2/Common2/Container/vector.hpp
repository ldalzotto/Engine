#pragma once

/*
    A Vector is a Span with an imaginary boundary (Size).
    Vector memory is continuous, there is no "hole" between items.
    Vector is reallocated on need.
    Any memory access outside of this imaginary boundary will be in error.
    The Vector expose some safe way to insert/erase data (array or single element).
*/
template <class type_element> struct Vector
{
    uimax Size;
    Span<type_element> Memory;

    inline Slice<type_element> to_slice() const
    {
        return Slice_build_memory_elementnb<type_element>(this->Memory.Memory, this->Size);
    };

    inline void free()
    {
        Span_free(&this->Memory);
        *this = Vector_build_zero_size<type_element>(NULL, 0);
    };

    inline type_element* get_memory()
    {
        return this->Memory.Memory;
    };

    inline uimax get_size()
    {
        return this->Size;
    };

    inline uimax get_capacity()
    {
        return this->Memory.Capacity;
    };

    inline int8 empty()
    {
        return this->Size == 0;
    };

    inline type_element& get(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index);
#endif
        return this->Memory.Memory[p_index];
    };

    inline const type_element& get(const uimax p_index) const
    {
        return ((Vector<type_element>*)this)->get(p_index);
    };

    inline void clear()
    {
        this->Size = 0;
    };

    inline int8 insert_array_at(const Slice<type_element>& p_elements, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

        return this->insert_array_at_unchecked(p_elements, p_index);
    };

    inline int8 insert_element_at(const type_element& p_element, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif

        return this->insert_element_at_unchecked(p_element, p_index);
    };

    inline int8 push_back_array(const Slice<type_element>& p_elements)
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + p_elements.Size);
        Slice_copy_memory_at_index(&this->Memory.slice, this->Size, &p_elements);
        this->Size += p_elements.Size;

        return 1;
    };

    inline int8 push_back_array_2(const Slice<type_element>& p_elements_0, const Slice<type_element>& p_elements_1)
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + p_elements_0.Size + p_elements_1.Size);
        Slice_copy_memory_at_index_2(&this->Memory.slice, this->Size, &p_elements_0, &p_elements_1);
        this->Size += (p_elements_0.Size + p_elements_1.Size);

        return 1;
    };

    inline int8 push_back_array_empty(const uimax p_array_size)
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + p_array_size);
        this->Size += p_array_size;
        return 1;
    };

    inline int8 push_back_element_empty()
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + 1);
        this->Size += 1;
        return 1;
    };

    inline int8 push_back_element(const type_element& p_element)
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + 1);
        this->Memory.Memory[this->Size] = p_element;
        this->Size += 1;

        return 1;
    };

    inline int8 insert_array_at_always(const Slice<type_element>& p_elements, const uimax p_index)
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

    inline int8 insert_element_at_always(const type_element& p_element, const uimax p_index)
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

    template<class Predicate_t>
    inline void erase_if(const Predicate_t& p_predicate)
    {
        for(loop_reverse(i, 0, this->Size))
        {
            if(p_predicate(this->get(i)))
            {
                this->erase_element_at_always(i);
            }
        }
    };

    inline void erase_all_elements_that_matches_element(const type_element& p_compared_element)
    {
        this->erase_if([&](const type_element& p_element){
            return p_element == p_compared_element;
        });
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
        Slice_move_memory_down(&this->Memory.slice, this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        Slice_move_memory_up(&this->Memory.slice, this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline int8 insert_element_at_unchecked(const type_element& p_element, const uimax p_index)
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + 1);
        this->move_memory_down(p_index, 1);
        this->Memory.Memory[p_index] = p_element;
        this->Size += 1;

        return 1;
    };

    inline int8 insert_array_at_unchecked(const Slice<type_element>& p_elements, const uimax p_index)
    {
        Span_resize_until_capacity_met(&this->Memory, this->Size + p_elements.Size);
        this->move_memory_down(p_index, p_elements.Size);
        Slice_copy_memory_at_index(&this->Memory.slice, p_index, &p_elements);

        this->Size += p_elements.Size;

        return 1;
    };

};

#define ShadowVector(ElementType) ShadowVector_##type_element

#define sv_func_get_size() get_size()
#define sv_func_get(p_index) get(p_index)
#define sv_func_erase_element_at(p_index) erase_element_at(p_index)
#define sv_func_erase_element_at_always(p_index) erase_element_at_always(p_index)
#define sv_func_push_back_element(p_element) push_back_element(p_element)
#define sv_func_to_slice() to_slice()

#define sv_c_get_size(p_shadow_vector) (p_shadow_vector)->get_size()
#define sv_c_get(p_shadow_vector, p_index) (p_shadow_vector)->get(p_index)
#define sv_c_erase_element_at_always(p_shadow_vector, p_index) (p_shadow_vector)->erase_element_at_always(p_index)
#define sv_c_push_back_element(p_shadow_vector, p_element) (p_shadow_vector)->push_back_element(p_element)
#define sv_c_to_slice(p_shadow_vector) (p_shadow_vector)->to_slice()

template<class type_element>
inline static Vector<type_element> Vector_build_zero_size(type_element* p_memory, const uimax p_initial_capacity)
{
    return Vector<type_element>{0, Span_build<type_element>(p_memory, p_initial_capacity)};
};

template<class type_element>
inline static Vector<type_element> Vector_allocate(const uimax p_initial_capacity)
{
    return Vector<type_element>{0, Span_allocate<type_element>(p_initial_capacity)};
};

template<class type_element>
inline static Vector<type_element> Vector_allocate_elements(const Slice<type_element>* p_initial_elements)
{
    Vector<type_element> l_vector = Vector_allocate<type_element>(p_initial_elements->Size);
    l_vector.push_back_array(*p_initial_elements);
    return l_vector;
};

template<class type_element>
inline static Vector<type_element> Vector_allocate_capacity_elements(const uimax p_inital_capacity, const Slice<type_element>& p_initial_elements)
{
    Vector<type_element> l_vector = Vector_allocate<type_element>(p_inital_capacity);
    l_vector.push_back_array(p_initial_elements);
    return l_vector;
};