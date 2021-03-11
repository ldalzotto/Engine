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
    union{
        struct
        {
            uimax Size;
            Span<ElementType> Memory;
        };
        Vector_ vector;
    };

    inline static Vector<ElementType> build_zero_size(ElementType* p_memory, const uimax p_initial_capacity)
    {
        return Vector<ElementType>{0, Span<ElementType>::build(p_memory, p_initial_capacity)};
    };

    inline static Vector<ElementType> allocate(const uimax p_initial_capacity)
    {
        return Vector<ElementType>{.vector = Vector__allocate(p_initial_capacity, sizeof(ElementType))};
    };

    inline static Vector<ElementType> allocate_slice(const Slice<ElementType>& p_initial_elements)
    {
        return Vector<ElementType>{.vector = Vector__allocate_slice(sizeof(ElementType), &p_initial_elements.slice)};
    };

    inline static Vector<ElementType> allocate_capacity_slice(const uimax p_inital_capacity, const Slice<ElementType>& p_initial_elements)
    {
        Vector<ElementType> l_vector = Vector<ElementType>::allocate(p_inital_capacity);
        l_vector.push_back_array(p_initial_elements);
        return l_vector;
    };

    inline Slice<ElementType> to_slice() const
    {
        return Slice<ElementType>::build(this->Memory.Memory, this->Size);
    };

    inline void free()
    {
        Vector__free(&this->vector);
    };

    inline ElementType* get_memory()
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
        return Vector__empty(&this->vector);
    };

    inline ElementType& get(const uimax p_index)
    {
        return *(ElementType*)Vector__get(&this->vector, sizeof(ElementType), p_index);
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((Vector<ElementType>*)this)->get(p_index);
    };

    inline void clear()
    {
        Vector__clear(&this->vector);
    };

    inline int8 insert_array_at(const Slice<ElementType>& p_elements, const uimax p_index)
    {
        return Vector__insert_array_at(&this->vector, sizeof(ElementType), &p_elements.slice, p_index);
    };

    inline int8 insert_element_at(const ElementType& p_element, const uimax p_index)
    {
        return Vector__insert_element_at(&this->vector, sizeof(ElementType), (int8*)&p_element, p_index);
    };

    inline int8 push_back_array(const Slice<ElementType>& p_elements)
    {
        return Vector__push_back_array(&this->vector, sizeof(ElementType), &p_elements.slice);
    };

    inline int8 push_back_array_2(const Slice<ElementType>& p_elements_0, const Slice<ElementType>& p_elements_1)
    {
        return Vector__push_back_array_2(&this->vector, sizeof(ElementType), &p_elements_0.slice, &p_elements_1.slice);
    };

    inline int8 push_back_array_empty(const uimax p_array_size)
    {
        return Vector__push_back_array_empty(&this->vector, sizeof(ElementType), p_array_size);
    };

    inline int8 push_back_element_empty()
    {
        return Vector__push_back_element_empty(&this->vector, sizeof(ElementType));
    };

    inline int8 push_back_element(const ElementType& p_element)
    {
        return Vector__push_back_element(&this->vector, sizeof(ElementType), (int8*)&p_element);
    };

    inline int8 insert_array_at_always(const Slice<ElementType>& p_elements, const uimax p_index)
    {
        return Vector__insert_array_at_always(&this->vector, sizeof(ElementType), &p_elements.slice, p_index);
    };

    inline int8 insert_element_at_always(const ElementType& p_element, const uimax p_index)
    {
        return Vector__insert_element_at_always(&this->vector, sizeof(ElementType), (int8*)&p_element, p_index);
    };

    inline int8 erase_array_at(const uimax p_index, const uimax p_element_nb)
    {
        return Vector__erase_array_at(&this->vector, sizeof(ElementType), p_index, p_element_nb);
    };

    inline int8 erase_element_at(const uimax p_index)
    {
        return Vector__erase_element_at(&this->vector, sizeof(ElementType), p_index);
    };

    inline int8 pop_back_array(const uimax p_element_nb)
    {
        return Vector__pop_back_array(&this->vector, p_element_nb);
    };

    inline int8 pop_back()
    {
        return Vector__pop_back(&this->vector);
    };

    inline int8 erase_element_at_always(const uimax p_index)
    {
        return Vector__erase_element_at_always(&this->vector, sizeof(ElementType), p_index);
    };

  private:
    inline void bound_check(const uimax p_index)
    {
        Vector__bound_check(&this->vector, p_index);
    };

    inline void bound_head_check(const uimax p_index)
    {
        Vector__bound_head_check(&this->vector, p_index);
    };

    inline void bound_lastelement_check(const uimax p_index)
    {
        Vector__bound_lastelement_check(&this->vector, p_index);
    };

    inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
    {
        Vector__move_memory_down(&this->vector, sizeof(ElementType), p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        Vector__move_memory_up(&this->vector, sizeof(ElementType), p_break_index, p_move_delta);
    };

    inline int8 insert_element_at_unchecked(const ElementType& p_element, const uimax p_index)
    {
        return Vector__insert_element_at_unchecked(&this->vector, sizeof(ElementType), (const int8*)&p_element, p_index);
    };

    inline int8 insert_array_at_unchecked(const Slice<ElementType>& p_elements, const uimax p_index)
    {
        return Vector__insert_array_at_unchecked(&this->vector, sizeof(ElementType), &p_elements.slice, p_index);
    };

#define ShadowVector(ElementType) ElementType##_##ShadowVector
#define ShadowVector_t(ElementType, Prefix) ElementType##_##ShadowVector##_##Prefix

#define sv_get_size() get_size()
#define sv_get(p_index) get(p_index)
#define sv_erase_element_at(p_index) erase_element_at(p_index)
#define sv_erase_element_at_always(p_index) erase_element_at_always(p_index)
#define sv_push_back_element(p_element) push_back_element(p_element)
#define sv_to_slice() to_slice()
};