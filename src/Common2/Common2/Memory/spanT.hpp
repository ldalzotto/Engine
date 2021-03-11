#pragma once

/*
    A Span is a heap allocated chunk of memory.
    Span can allocate memory, be resized and freed.
*/
template <class ElementType> struct Span
{
    union
    {
        struct
        {
            uimax Capacity;
            ElementType* Memory;
        };
        Slice<ElementType> slice;
        Span_ span;
    };

    inline static Span<ElementType> build_default()
    {
        return Span<ElementType>{.span = Span__build_default()};
    };

    inline static Span<ElementType> build(ElementType* p_memory, const uimax p_capacity)
    {
        return Span<ElementType>{.span = Span__build((int8*)p_memory, p_capacity)};
    };

    inline static Span<ElementType> allocate(const uimax p_capacity)
    {
        return Span<ElementType>{.span = Span__allocate(sizeof(ElementType), p_capacity)};
    };

    inline static Span<ElementType> allocate_slice(const Slice<ElementType>& p_elements)
    {
        return Span<ElementType>{.span = Span__allocate_slice(sizeof(ElementType), &p_elements.slice)};
    };

    inline static Span<ElementType> allocate_slice_2(const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2)
    {
        return Span<ElementType>{.span = Span__allocate_slice_2(sizeof(ElementType), &p_elements_1.slice, &p_elements_2.slice)};
    };

    inline static Span<ElementType> allocate_slice_3(const Slice<ElementType>& p_elements_1, const Slice<ElementType>& p_elements_2, const Slice<ElementType>& p_elements_3)
    {
        return Span<ElementType>{.span = Span__allocate_slice_3(sizeof(ElementType), &p_elements_1.slice, &p_elements_2.slice, &p_elements_3.slice)};
    };

    inline static Span<ElementType> callocate(const uimax p_capacity)
    {
        return Span<ElementType>{.span = Span__callocate(sizeof(ElementType), p_capacity)};
    };

    inline ElementType& get(const uimax p_index)
    {
        return *(ElementType*)Span__get(&this->span, sizeof(ElementType), p_index);
    };

    inline int8 resize(const uimax p_new_capacity)
    {
        return Span__resize(&this->span, sizeof(ElementType), p_new_capacity);
    };

    inline const ElementType& get(const uimax p_index) const
    {
        return ((Span<ElementType>*)(this))->get(p_index);
    };

    inline Span<ElementType> move()
    {
        return Span<ElementType>{.span = Span__move(&this->span)};
    };

    inline void resize_until_capacity_met(const uimax p_desired_capacity)
    {
        Span__resize_until_capacity_met(&this->span, sizeof(ElementType), p_desired_capacity);
    };

    inline void free()
    {
        Span__free(&this->span);
    };
};
