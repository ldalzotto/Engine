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

    inline void set_size(const uimax p_size)
    {
        this->Size = p_size;
    };

    inline uimax get_capacity() const
    {
        return this->Memory.Capacity;
    };

    // TODO
    inline int8 empty() const
    {
        return this->Size == 0;
        // return iVector_v2<Vector<ElementType>>{*(Vector<ElementType>*)this}.empty();
    };

    inline iVector<Vector<ElementType>> to_ivector()
    {
        return iVector<Vector<ElementType>>{*this};
    };

    inline ElementType& get_unchecked(const uimax p_index)
    {
        return this->Memory.Memory[p_index];
    };

    inline ElementType& get(const uimax p_index)
    {
        return iVector_v2<Vector<ElementType>>{*this}.get(p_index);
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((Vector<ElementType>*)this)->get(p_index);
    };

    inline void set_unchecked(const uimax p_index, const ElementType& p_element_type)
    {
        this->Memory.get(p_index) = p_element_type;
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
        return iVector_v2<Vector<ElementType>>{*this}.insert_array_at(p_elements, p_index);
    };

    inline int8 insert_element_at(const ElementType& p_element, const uimax p_index)
    {
        return iVector_v2<Vector<ElementType>>{*this}.insert_element_at(p_element, p_index);
    };

    inline int8 push_back_array(const Slice<ElementType>& p_elements)
    {
        return iVector_v2<Vector<ElementType>>{*this}.push_back_array(p_elements);
    };

    inline int8 push_back_array_2(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1)
    {
        return iVector_v2<Vector<ElementType>>{*this}.push_back_array_2(p_elements_0, p_elements_1);
    };

    inline int8 push_back_array_3(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        return iVector_v2<Vector<ElementType>>{*this}.push_back_array_3(p_elements_0, p_elements_1, p_elements_2);
    };

    inline int8 push_back_array_empty(const uimax p_array_size)
    {
        return iVector_v2<Vector<ElementType>>{*this}.push_back_array_empty(p_array_size);
    };

    inline int8 push_back_element_empty()
    {
        return iVector_v2<Vector<ElementType>>{*this}.push_back_element_empty();
    };

    inline int8 push_back_element(const ElementType& p_element)
    {
        return iVector_v2<Vector<ElementType>>{*this}.push_back_element(p_element);
    };

    inline int8 insert_array_at_always(const Slice<ElementType>& p_elements, const uimax p_index)
    {
        return iVector_v2<Vector<ElementType>>{*this}.insert_array_at_always(p_elements, p_index);
    };

    inline int8 insert_element_at_always(const ElementType& p_element, const uimax p_index)
    {
        return iVector_v2<Vector<ElementType>>{*this}.insert_element_at_always(p_element, p_index);
    };

    inline int8 erase_array_at(const uimax p_index, const uimax p_element_nb)
    {
        return iVector_v2<Vector<ElementType>>{*this}.erase_array_at(p_index, p_element_nb);
    };

    inline int8 erase_element_at(const uimax p_index)
    {
        return iVector_v2<Vector<ElementType>>{*this}.erase_element_at(p_index);
    };

    inline int8 pop_back_array(const uimax p_element_nb)
    {
        return iVector_v2<Vector<ElementType>>{*this}.pop_back_array(p_element_nb);
    };

    inline int8 pop_back()
    {
        return iVector_v2<Vector<ElementType>>{*this}.pop_back();
    };

    inline int8 erase_element_at_always(const uimax p_index)
    {
        return iVector_v2<Vector<ElementType>>{*this}.erase_element_at_always(p_index);
    };

    inline void erase_array_at_always(const uimax p_index, const uimax p_size)
    {
        iVector_v2<Vector<ElementType>>{*this}.erase_array_at_always(p_index, p_size);
    };

    template <class Predicate_t> inline void erase_if(const Predicate_t& p_predicate)
    {
        this->to_ivector().erase_if(p_predicate);
    };

    inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.slice.move_memory_down(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.slice.move_memory_up(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void resize_until_capacity_met(const uimax p_desired_capacity)
    {
        this->Memory.resize_until_capacity_met(p_desired_capacity);
    };

    inline void copy_memory_at_index(const uimax p_copy_index, const Slice<ElementType>& p_elements)
    {
        this->Memory.slice.copy_memory_at_index(p_copy_index, p_elements);
    };

    inline void copy_memory_at_index_2(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        this->Memory.slice.copy_memory_at_index_2(p_copy_index, p_elements_1, p_elements_2);
    };

    inline void copy_memory_at_index_3(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
    {
        this->Memory.slice.copy_memory_at_index_3(p_copy_index, p_elements_1, p_elements_2, p_elements_3);
    };
};
