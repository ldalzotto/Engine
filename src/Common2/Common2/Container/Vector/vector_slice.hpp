#pragma once

template <class ElementType> struct VectorSlice
{
    uimax Size;
    Slice<ElementType> Memory;

    using t_Element = ElementType&;
    using t_ElementValue = ElementType;

    inline static VectorSlice<ElementType> build(const Slice<ElementType>& p_memory, const uimax p_initial_size)
    {
        return VectorSlice<ElementType>{p_initial_size, p_memory};
    };

    inline Slice<ElementType> to_slice() const
    {
        return Slice<ElementType>::build_memory_elementnb(this->Memory.Begin, this->Size);
    };

    inline uimax get_size() const
    {
        return this->Size;
    };

    inline void set_size(uimax p_size)
    {
        this->Size = p_size;
    };

    inline ElementType& get_unchecked(const uimax p_index)
    {
        return this->Memory.Begin[p_index];
    };

    inline void set_unchecked(const uimax p_index, const ElementType& p_element_type)
    {
        this->Memory.get(p_index) = p_element_type;
    };

    inline void set(const uimax p_index, const ElementType& p_element_type)
    {
        this->get(p_index) = p_element_type;
    };

    iVector_functions_forward_declare(VectorSlice<ElementType>);

    inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.move_memory_down(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        this->Memory.move_memory_up(this->Size - p_break_index, p_break_index, p_move_delta);
    };

    inline void resize_until_capacity_met(const uimax p_desired_capacity){
        // nothing for slice
    };

    inline void copy_memory_at_index(const uimax p_copy_index, const Slice<ElementType>& p_elements)
    {
        this->Memory.copy_memory_at_index(p_copy_index, p_elements);
    };

    inline void copy_memory_at_index_2(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        this->Memory.copy_memory_at_index_2(p_copy_index, p_elements_1, p_elements_2);
    };

    inline void copy_memory_at_index_3(const uimax p_copy_index, const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
    {
        this->Memory.copy_memory_at_index_3(p_copy_index, p_elements_1, p_elements_2, p_elements_3);
    };
};
