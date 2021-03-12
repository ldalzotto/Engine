#pragma once

/*
    A Span is a heap allocated chunk of memory.
    Span can allocate memory, be resized and freed.
*/
typedef struct s_Span_
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

inline Span_ Span__build(int8* p_memory, const uimax p_capacity)
{
#if __cplusplus
    return Span_{p_capacity, p_memory};
#else
    return (Span_){p_capacity, p_memory};
#endif
};

inline Span_ Span__build_default()
{
#if __cplusplus
    return Span_{.slice = Slice__build_default()};
#else
    return (Span_){.slice = Slice__build_default()};
#endif
};

inline Span_ Span__allocate(const uimax t_elementtype, const uimax p_capacity)
{
    return Span__build(heap_malloc(p_capacity * t_elementtype), p_capacity);
};

inline void Span__free(Span_* thiz)
{
    heap_free(thiz->Memory);
    *thiz = Span__build_default();
};

inline Span_ Span__allocate_slice(const uimax t_elementtype, const Slice_* p_elements)
{
    Span_ l_span = Span__allocate(t_elementtype, p_elements->Size);
    Slice__memcpy(&l_span.slice, p_elements, t_elementtype);
    return l_span;
};

inline Span_ Span__allocate_slice_2(const uimax t_elementtype, const Slice_* p_elements_1, const Slice_* p_elements_2)
{
    Span_ l_span = Span__allocate(t_elementtype, p_elements_1->Size + p_elements_2->Size);
    Slice__copy_memory_2(&l_span.slice, t_elementtype, 0, p_elements_1, p_elements_2);
    return l_span;
};

inline Span_ Span__allocate_slice_3(const uimax t_elementtype, const Slice_* p_elements_1, const Slice_* p_elements_2, const Slice_* p_elements_3)
{
    Span_ l_span = Span__allocate(t_elementtype, p_elements_1->Size + p_elements_2->Size + p_elements_3->Size);
    Slice__copy_memory_3(&l_span.slice, t_elementtype, 0, p_elements_1, p_elements_2, p_elements_3);
    return l_span;
};

inline Span_ Span__callocate(const uimax t_elementtype, const uimax p_capacity)
{
    return Span__build(heap_calloc(p_capacity * t_elementtype), p_capacity);
};

inline int8* Span__get(Span_* thiz, const uimax t_thiz_elementtype, const uimax p_index)
{
#if CONTAINER_BOUND_TEST
    assert_true(p_index < thiz->Capacity);
#endif
    return thiz->Memory + (p_index * t_thiz_elementtype);
};

inline int8 Span__resize(Span_* thiz, const uimax t_thiz_elementtype, const uimax p_new_capacity)
{
    if (p_new_capacity > thiz->Capacity)
    {
        int8* l_newMemory = heap_realloc(thiz->Memory, p_new_capacity * t_thiz_elementtype);
        if (l_newMemory != NULL)
        {
            *thiz = Span__build(l_newMemory, p_new_capacity);
            return 1;
        }
        return 0;
    }
    return 1;
};

inline Span_ Span__move(Span_* thiz)
{
    Span_ l_return = *thiz;
    *thiz = Span__build_default();
    return l_return;
};

inline void Span__resize_until_capacity_met(Span_* thiz, const uimax t_thiz_elementtype, const uimax p_desired_capacity)
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

    Span__resize(thiz, t_thiz_elementtype, l_resized_capacity);
};

#define SpanC(ElementType) Span_##ElementType

#define SpanC_declare(ElementType)                                                                                                                                                                     \
    typedef struct s_Span_##ElementType                                                                                                                                                                \
    {                                                                                                                                                                                                  \
        union                                                                                                                                                                                          \
        {                                                                                                                                                                                              \
            struct                                                                                                                                                                                     \
            {                                                                                                                                                                                          \
                union                                                                                                                                                                                  \
                {                                                                                                                                                                                      \
                    struct                                                                                                                                                                             \
                    {                                                                                                                                                                                  \
                        uimax Capacity;                                                                                                                                                                \
                        ElementType* Memory;                                                                                                                                                           \
                    };                                                                                                                                                                                 \
                    SliceC(ElementType) slice;                                                                                                                                                         \
                };                                                                                                                                                                                     \
            };                                                                                                                                                                                         \
            Span_ span;                                                                                                                                                                                \
        };                                                                                                                                                                                             \
    } SpanC(ElementType)

#define SpanC_build(ElementType) Span_##ElementType##_build
#define SpanC_free(ElementType) Span_##ElementType##_free
#define SpanC_resize(ElementType) Span_##ElementType##_resize
#define SpanC_get(ElementType) Span_##ElementType##_get

#define SpanC_declare_functions(ElementType)                                                                                                                                                           \
    inline SpanC(ElementType) SpanC_build(ElementType)(ElementType * p_memory, const uimax p_capacity)                                                                                                 \
    {                                                                                                                                                                                                  \
        return (SpanC(ElementType)){.span = Span__build((int8*)p_memory, p_capacity)};                                                                                                                 \
    };                                                                                                                                                                                                 \
    inline void SpanC_free(ElementType)(SpanC(ElementType) * thiz)                                                                                                                                     \
    {                                                                                                                                                                                                  \
        Span__free(&thiz->span);                                                                                                                                                                       \
    };                                                                                                                                                                                                 \
    inline int8 SpanC_resize(ElementType)(SpanC(ElementType) * thiz, const uimax p_new_capacity)                                                                                                       \
    {                                                                                                                                                                                                  \
        return Span__resize(&thiz->span, sizeof(ElementType), p_new_capacity);                                                                                                                         \
    };                                                                                                                                                                                                 \
    inline ElementType* SpanC_get(ElementType)(SpanC(ElementType) * thiz, const uimax p_index)                                                                                                         \
    {                                                                                                                                                                                                  \
        return (ElementType*)Span__get(&thiz->span, sizeof(ElementType), p_index);                                                                                                                     \
    };                                                                                                                                                                                                 \
    \