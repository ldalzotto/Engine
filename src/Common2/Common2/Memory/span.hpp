#pragma once

/*
    A Span is a heap allocated chunk of memory.
    Span can allocate memory, be resized and freed.
*/
template <class type_element> struct Span
{
    union
    {
        struct
        {
            uimax Capacity;
            type_element* Memory;
        };
        Slice<type_element> slice;
    };
};

template <class type_element>
inline Span<type_element> Span_build_default()
{
    Span<type_element> l_return;
    l_return.slice = Slice_build_default<type_element>();
    return l_return;
};

template <class type_element>
inline Span<type_element> Span_build(type_element* p_memory, const uimax p_capacity)
{
    return Span<type_element>{p_capacity, p_memory};
};

template <class type_element>
inline Span<type_element> Span_allocate(const uimax p_capacity)
{
    return Span<type_element>{p_capacity, (type_element*)heap_malloc(p_capacity * sizeof(type_element))};
};

template <class type_element>
inline Span<type_element> Span_allocate_slice(const Slice<type_element>* p_elements)
{
    Span<type_element> l_span = Span_allocate<type_element>(p_elements->Size);
    Slice_copy_memory(&l_span.slice, p_elements);
    return l_span;
};

template <class type_element>
inline Span<type_element> Span_allocate_slice_2(const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2)
{
    Span<type_element> l_span = Span_allocate<type_element>(p_elements_1->Size + p_elements_2->Size);
    Slice_copy_memory_at_index_2(&l_span.slice, 0, p_elements_1, p_elements_2);
    return l_span;
};

template <class type_element>
inline Span<type_element> Span_allocate_slice_3(const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2, const Slice<type_element>* p_elements_3)
{
    Span<type_element> l_span = Span_allocate<type_element>(p_elements_1->Size + p_elements_2->Size + p_elements_3->Size);
    Slice_copy_memory_at_index_3(&l_span.slice, 0, p_elements_1, p_elements_2, p_elements_3);
    return l_span;
};

template <class type_element>
inline Span<type_element> Span_callocate(const uimax p_capacity)
{
    return Span<type_element>{p_capacity, (type_element*)heap_calloc(p_capacity * sizeof(type_element))};
};

template <class type_element>
inline static type_element* Span_get(Span<type_element>* thiz, const uimax p_index)
{
#if __DEBUG
    assert_true(p_index < thiz->Capacity);
#endif
    return &thiz->Memory[p_index];
};

template <class type_element>
inline static type_element* Span_get(const Span<type_element>* thiz, const uimax p_index)
{
    return Span_get((Span<type_element>*)(thiz), p_index);
};

template <class type_element>
inline int8 Span_resize(Span<type_element>* thiz, const uimax p_new_capacity)
{
    if (p_new_capacity > thiz->Capacity)
    {
        type_element* l_newMemory = (type_element*)heap_realloc((int8*)thiz->Memory, p_new_capacity * sizeof(type_element));
        if (l_newMemory != NULL)
        {
            *thiz = Span_build<type_element>(l_newMemory, p_new_capacity);
            return 1;
        }
        return 0;
    }
    return 1;
};

template <class type_element>
inline Span<type_element> Span_move_to_value(Span<type_element>* thiz)
{
    Span<type_element> l_return = *thiz;
    *thiz = Span_build<type_element>(NULL, 0);
    return l_return;
};

template <class type_element>
inline void Span_resize_until_capacity_met(Span<type_element>* thiz, const uimax p_desired_capacity)
{
    uimax l_resized_capacity = thiz->Capacity;

    if (l_resized_capacity >= p_desired_capacity)
    {
        return;
    }

    if (l_resized_capacity == 0)
    {
        l_resized_capacity = 1;
    }

    while (l_resized_capacity < p_desired_capacity)
    {
        l_resized_capacity *= 2;
    }

    Span_resize(thiz, l_resized_capacity);
};

template <class type_element>
inline void Span_free(Span<type_element>* thiz)
{
    heap_free((int8*)thiz->Memory);
    *thiz = Span_build<type_element>(NULL, 0);
};