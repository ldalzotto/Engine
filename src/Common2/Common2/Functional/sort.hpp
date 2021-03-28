#pragma once

struct Sort
{
    // TODO ->Implement Quick sort
    template<class ElementType, class CompareSlot>
    inline static void Linear3(Slice<ElementType>& p_slice, const uimax p_start_index, const CompareSlot& p_compare_slot)
    {
        for (loop(i, p_start_index, p_slice.Size))
        {
            ElementType& l_left = *Slice_get(&p_slice, i);
            for (loop(j, i, p_slice.Size))
            {
                ElementType& l_right = *Slice_get(&p_slice, j);
                if (p_compare_slot(l_left, l_right))
                {
                    ElementType l_left_tmp = l_left;
                    l_left = l_right;
                    l_right = l_left_tmp;
                }
            }
        }
    };
};
