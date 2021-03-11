#pragma once


template<class ElementType>
inline hash_t HashSlice(const Slice<ElementType>& p_value)
{
    return HashSlice_(&p_value.slice, sizeof(ElementType));
};
