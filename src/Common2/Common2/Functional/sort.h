#pragma once

// TODO ->Implement Quick sort
#define Sort_Linear_inline(ElementType, p_slice, ComparisonSlot)                                                                                                                                       \
    for (loop(i, 0, p_slice->Size))                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        ElementType* l_left = (ElementType*)Slice__get(p_slice, sizeof(ElementType), i);                                                                                                               \
        for (loop(j, i, p_slice->Size))                                                                                                                                                                \
        {                                                                                                                                                                                              \
            ElementType* l_right = (ElementType*)Slice__get(p_slice, sizeof(ElementType), j);                                                                                                          \
            int8 l_comparison;                                                                                                                                                                         \
            ComparisonSlot;                                                                                                                                                                            \
            if (l_comparison)                                                                                                                                                                          \
            {                                                                                                                                                                                          \
                ElementType l_left_tmp = *l_left;                                                                                                                                                      \
                *l_left = *l_right;                                                                                                                                                                    \
                *l_right = l_left_tmp;                                                                                                                                                                 \
            }                                                                                                                                                                                          \
        }                                                                                                                                                                                              \
    }
