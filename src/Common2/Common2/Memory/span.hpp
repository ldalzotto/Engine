#pragma once

typedef struct Span_
{
    union
    {
        struct
        {
            uimax Capacity;
            int8* Memory;
        };
        Slice_ slice;
    };
} Span_;

inline Span_ Span__build_default()
{
    Span_ l_return;
    l_return.slice = Slice__build_default();
    return l_return;
};

inline Span_ Span__build(int8* p_memory, const uimax p_capacity)
{
    return Span_{p_capacity, p_memory};
};

inline Span_ Span__allocate(const uimax p_capacity, const uimax type_element)
{
    return Span_{p_capacity, heap_malloc(p_capacity * type_element)};
};

inline Span_ Span__allocate_slice(const Slice_* p_elements, const uimax type_element)
{
    Span_ l_span = Span__allocate(p_elements->Size, type_element);
    Slice__copy_memory(&l_span.slice, p_elements, type_element);
    return l_span;
};

inline Span_ Span__allocate_slice_2(const Slice_* p_elements_1, const Slice_* p_elements_2, const uimax type_element)
{
    Span_ l_span = Span__allocate(p_elements_1->Size + p_elements_2->Size, type_element);
    Slice__copy_memory_at_index_2(&l_span.slice, 0, p_elements_1, p_elements_2, type_element);
    return l_span;
};

inline Span_ Span__allocate_slice_3(const Slice_* p_elements_1, const Slice_* p_elements_2, const Slice_* p_elements_3, const uimax type_element)
{
    Span_ l_span = Span__allocate(p_elements_1->Size + p_elements_2->Size + p_elements_3->Size, type_element);
    Slice__copy_memory_at_index_3(&l_span.slice, 0, p_elements_1, p_elements_2, p_elements_3, type_element);
    return l_span;
};

inline Span_ Span__callocate(const uimax p_capacity, const uimax type_element)
{
    return Span_{p_capacity, heap_calloc(p_capacity * type_element)};
};

inline int8* Span__get(const Span_* thiz, const uimax p_index, const uimax type_element)
{
#if __DEBUG
    assert_true(p_index < thiz->Capacity);
#endif
    return thiz->Memory + (p_index * type_element);
};

inline int8 Span__resize(Span_* thiz, const uimax p_new_capacity, const uimax type_element)
{
    if (p_new_capacity > thiz->Capacity)
    {
        int8* l_newMemory = heap_realloc((int8*)thiz->Memory, p_new_capacity * type_element);
        if (l_newMemory != NULL)
        {
            *thiz = Span__build(l_newMemory, p_new_capacity);
            return 1;
        }
        return 0;
    }
    return 1;
};

inline Span_ Span__move_to_value(Span_* thiz)
{
    Span_ l_return = *thiz;
    *thiz = Span__build(NULL, 0);
    return l_return;
};

inline void Span__resize_until_capacity_met(Span_* thiz, const uimax p_desired_capacity, const uimax type_element)
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

    Span__resize(thiz, l_resized_capacity, type_element);
};

inline void Span__free(Span_* thiz)
{
    heap_free((int8*)thiz->Memory);
    *thiz = Span__build(NULL, 0);
};

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
        Span_ span;
    };
};

template <class type_element> inline Span<type_element> Span_build_default()
{
    return Span<type_element>{.span = Span__build_default()};
};

template <class type_element> inline Span<type_element> Span_build(type_element* p_memory, const uimax p_capacity)
{
    return Span<type_element>{.span = Span__build((int8*)p_memory, p_capacity)};
};

template <class type_element> inline Span<type_element> Span_allocate(const uimax p_capacity)
{
    return Span<type_element>{.span = Span__allocate(p_capacity, sizeof(type_element))};
};

template <class type_element> inline Span<type_element> Span_allocate_slice(const Slice<type_element>* p_elements)
{
    return Span<type_element>{.span = Span__allocate_slice(&p_elements->slice, sizeof(type_element))};
};

template <class type_element> inline Span<type_element> Span_allocate_slice_2(const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2)
{
    return Span<type_element>{.span = Span__allocate_slice_2(&p_elements_1->slice, &p_elements_2->slice, sizeof(type_element))};
};

template <class type_element> inline Span<type_element> Span_allocate_slice_3(const Slice<type_element>* p_elements_1, const Slice<type_element>* p_elements_2, const Slice<type_element>* p_elements_3)
{
    return Span<type_element>{.span = Span__allocate_slice_3(&p_elements_1->slice, &p_elements_2->slice, &p_elements_3->slice, sizeof(type_element))};
};

template <class type_element> inline Span<type_element> Span_callocate(const uimax p_capacity)
{
    return Span<type_element>{.span = Span__callocate(p_capacity, sizeof(type_element))};
};

template <class type_element> inline static type_element* Span_get(Span<type_element>* thiz, const uimax p_index)
{
    return (type_element*)Span__get(&thiz->span, p_index, sizeof(type_element));
};

template <class type_element> inline static type_element* Span_get(const Span<type_element>* thiz, const uimax p_index)
{
    return (type_element*)Span__get(&thiz->span, p_index, sizeof(type_element));
};

template <class type_element> inline int8 Span_resize(Span<type_element>* thiz, const uimax p_new_capacity)
{
    return Span__resize(&thiz->span, p_new_capacity, sizeof(type_element));
};

template <class type_element> inline Span<type_element> Span_move_to_value(Span<type_element>* thiz)
{
    return Span<type_element>{.span = Span__move_to_value(&thiz->span)};
};

template <class type_element> inline void Span_resize_until_capacity_met(Span<type_element>* thiz, const uimax p_desired_capacity)
{
    Span__resize_until_capacity_met(&thiz->span, p_desired_capacity, sizeof(type_element));
};

template <class type_element> inline void Span_free(Span<type_element>* thiz)
{
    Span__free(&thiz->span);
};