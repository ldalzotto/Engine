#pragma once

/*
    A Vector is a Span with an imaginary boundary (Size).
    Vector memory is continuous, there is no "hole" between items.
    Vector is reallocated on need.
    Any memory access outside of this imaginary boundary will be in error.
    The Vector expose some safe way to insert/erase data (array or single element).
*/
typedef struct s_Vector_
{
    uimax Size;
    Span_ Memory;
} Vector_;

inline Vector_ Vector__build(const uimax p_size, Span_ p_span)
{
#if __cplusplus
    return Vector_{.Size = p_size, .Memory = p_span};
#else
    return (Vector_){.Size = p_size, .Memory = p_span};
#endif
};

inline Vector_ Vector__build_default()
{
    return Vector__build(0, Span__build_default());
};

inline Vector_ Vector__build_zero_size(int8* p_memory, const uimax p_initial_capacity)
{
    return Vector__build(0, Span__build(p_memory, p_initial_capacity));
};

inline Vector_ Vector__allocate(const uimax p_initial_capacity, const uimax t_elementtype)
{
    return Vector__build(0, Span__allocate(t_elementtype, p_initial_capacity));
};

inline Vector_ Vector__allocate_slice(const uimax t_elementtype, const Slice_* p_initial_elements)
{
    return Vector__build(0, Span__allocate_slice(t_elementtype, p_initial_elements));
};

inline void Vector__free(Vector_* thiz)
{
    Span__free(&thiz->Memory);
    *thiz = Vector__build_default();
};

inline int8 Vector__empty(Vector_* thiz)
{
    return thiz->Size == 0;
};

inline void Vector__clear(Vector_* thiz)
{
    thiz->Size = 0;
};

inline void Vector__bound_check(Vector_* thiz, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    if (p_index > thiz->Size)
    {
        abort();
    }
#endif
};

inline void Vector__bound_head_check(Vector_* thiz, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    if (p_index == thiz->Size)
    {
        abort();
    }
#endif
};

inline void Vector__bound_lastelement_check(Vector_* thiz, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    if (p_index == thiz->Size - 1)
    {
        abort();
    }
#endif
};

inline int8* Vector__get(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
    Vector__bound_head_check(thiz, p_index);
#endif
    return thiz->Memory.Memory + (p_index * t_thiz_elementtype);
};

inline static void Vector__move_memory_down(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_down(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size - p_break_index, p_break_index, p_move_delta);
};

inline void Vector__move_memory_up(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_break_index, const uimax p_move_delta)
{
    Slice__move_memory_up(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size - p_break_index, p_break_index, p_move_delta);
};

inline static int8 Vector__insert_element_at_unchecked(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element, const uimax p_index)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + 1);
    Vector__move_memory_down(thiz, t_thiz_elementtype, p_index, 1);
    memory_cpy(thiz->Memory.Memory + (p_index * t_thiz_elementtype), p_element, t_thiz_elementtype);
    thiz->Size += 1;
    return 1;
};

inline static int8 Vector__insert_array_at_unchecked(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements, const uimax p_index)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_elements->Size);
    Vector__move_memory_down(thiz, t_thiz_elementtype, p_index, p_elements->Size);
    Slice__copy_memory(&thiz->Memory.slice, t_thiz_elementtype, p_index, p_elements);

    thiz->Size += p_elements->Size;

    return 1;
};

inline int8 Vector__insert_array_at(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
    Vector__bound_head_check(thiz, p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

    return Vector__insert_array_at_unchecked(thiz, t_thiz_elementtype, p_elements, p_index);
};

inline int8 Vector__insert_element_at(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
    Vector__bound_head_check(thiz, p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif

    return Vector__insert_element_at_unchecked(thiz, t_thiz_elementtype, p_element, p_index);
};

inline int8 Vector__push_back_array(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_elements->Size);
    Slice__copy_memory(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size, p_elements);
    thiz->Size += p_elements->Size;

    return 1;
};

inline int8 Vector__push_back_array_2(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements_0, const Slice_* p_elements_1)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_elements_0->Size + p_elements_1->Size);
    Slice__copy_memory_2(&thiz->Memory.slice, t_thiz_elementtype, thiz->Size, p_elements_0, p_elements_1);
    thiz->Size += (p_elements_0->Size + p_elements_1->Size);

    return 1;
};

inline int8 Vector__push_back_array_empty(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_array_size)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + p_array_size);
    thiz->Size += p_array_size;
    return 1;
};

inline int8 Vector__push_back_element_empty(Vector_* thiz, const uimax t_thiz_elementtype)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + 1);
    thiz->Size += 1;
    return 1;
};

inline int8 Vector__push_back_element(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element)
{
    Span__resize_until_capacity_met(&thiz->Memory, t_thiz_elementtype, thiz->Size + 1);
    memory_cpy(thiz->Memory.Memory + (thiz->Size * t_thiz_elementtype), p_element, t_thiz_elementtype);
    thiz->Size += 1;

    return 1;
};

inline int8 Vector__insert_array_at_always(Vector_* thiz, const uimax t_thiz_elementtype, const Slice_* p_elements, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
#endif
    if (p_index == thiz->Size)
    {
        return Vector__push_back_array(thiz, t_thiz_elementtype, p_elements);
    }
    else
    {
        return Vector__insert_array_at_unchecked(thiz, t_thiz_elementtype, p_elements, p_index);
    }
};

inline int8 Vector__insert_element_at_always(Vector_* thiz, const uimax t_thiz_elementtype, const int8* p_element, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
#endif

    if (p_index == thiz->Size)
    {
        return Vector__push_back_element(thiz, t_thiz_elementtype, p_element);
    }
    else
    {
        return Vector__insert_element_at_unchecked(thiz, t_thiz_elementtype, p_element, p_index);
    }
};

inline int8 Vector__erase_array_at(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index, const uimax p_element_nb)
{

#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
    Vector__bound_check(thiz, p_index + p_element_nb);
    Vector__bound_lastelement_check(thiz, p_index); // use vector_pop_back_array
#endif

    Vector__move_memory_up(thiz, t_thiz_elementtype, p_index + p_element_nb, p_element_nb);
    thiz->Size -= p_element_nb;

    return 1;
};

inline int8 Vector__erase_element_at(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
    Vector__bound_lastelement_check(thiz, p_index); // use vector_pop_back
#endif

    Vector__move_memory_up(thiz, t_thiz_elementtype, p_index + 1, 1);
    thiz->Size -= 1;

    return 1;
};

inline int8 Vector__pop_back_array(Vector_* thiz, const uimax p_element_nb)
{
    thiz->Size -= p_element_nb;
    return 1;
};

inline int8 Vector__pop_back(Vector_* thiz)
{
    thiz->Size -= 1;
    return 1;
};

inline int8 Vector__erase_element_at_always(Vector_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    Vector__bound_check(thiz, p_index);
#endif

    if (p_index == thiz->Size - 1)
    {
        return Vector__pop_back(thiz);
    }
    else
    {
        return Vector__erase_element_at(thiz, t_thiz_elementtype, p_index);
    }
};

/*
inline Vector_ Vector__allocate_capacity_slice(const uimax t_elementtype, const uimax p_inital_capacity, const Slice_* p_initial_elements)
{
    Vector_ l_vector = Vector__allocate(p_inital_capacity, t_elementtype);
    l_vector.push_back_array(p_initial_elements);
    return l_vector;
};
*/