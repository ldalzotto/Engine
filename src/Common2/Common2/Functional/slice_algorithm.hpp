#pragma once

template<class ElementType>
inline int8 Slice_find(const Slice<ElementType>& thiz, const Slice<ElementType>& p_other, uimax* out_index)
{
#if __DEBUG
    if (p_other.Size > thiz.Size)
    {
        abort();
    }
#endif

    Slice<ElementType> l_target_slice = thiz;
    if (l_target_slice.compare(p_other))
    {
        *out_index = 0;
        return 1;
    };

    for (uimax i = 1; i < thiz.Size - p_other.Size + 1; i++)
    {
        l_target_slice.slide(1);
        if (l_target_slice.compare(p_other))
        {
            *out_index = i;
            return 1;
        };
    };

    *out_index = -1;
    return 0;
};

template<class ElementType>
inline int8 Slice_last_index_of(const Slice<ElementType>& thiz, const Slice<ElementType>& p_other, uimax* out_index)
{
    Slice<ElementType> l_comparison = thiz;
    uimax tmp_index = -1;
    uimax l_current_index = -1;
    *out_index = l_current_index;
    while (l_comparison.Size != 0 && Slice_find(l_comparison, p_other, &tmp_index))
    {
        l_current_index += (tmp_index + 1);
        *out_index = l_current_index;
        l_comparison.slide(tmp_index + 1);
    }

    return (*out_index) != -1;
};

template<class ElementType>
inline int8 Slice_last_index_of_not_endofslice(const Slice<ElementType>& thiz, const Slice<ElementType>& p_other, uimax* out_index)
{
    Slice<ElementType> l_comparison = thiz;
    uimax tmp_index = -1;
    uimax l_current_index = -1;
    *out_index = l_current_index;
    while (l_comparison.Size != 0 && Slice_find(l_comparison, p_other, &tmp_index))
    {
        l_comparison.slide(tmp_index + 1);
        if (l_comparison.Size != 0)
        {
            l_current_index += (tmp_index + 1);
            *out_index = l_current_index;
        }
    }

    return (*out_index) != -1;
};