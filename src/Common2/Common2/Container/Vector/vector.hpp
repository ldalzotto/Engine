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

    inline ElementType& get_unchecked(const uimax p_index)
    {
        return this->Memory.Memory[p_index];
    };

    inline void set_unchecked(const uimax p_index, const ElementType& p_element_type)
    {
        this->Memory.get(p_index) = p_element_type;
    };

    inline void set(const uimax p_index, const ElementType& p_element_type)
    {
        this->get(p_index) = p_element_type;
    };

    iVector_functions_forward_declare(Vector<ElementType>);

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
