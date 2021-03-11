#pragma once

struct Sort
{
    // TODO ->Implement Quick sort
    template <class ElementType, class CompareFunc> inline static void Linear2(Slice<ElementType>& p_slice, const uimax p_start_index)
    {
        Slice_* l_slice = &p_slice.slice;
        Sort_Linear_inline(ElementType, l_slice, {l_comparison = CompareFunc::compare(*l_left, *l_right);} );
    };
};

#define sort_linear2_begin(ElementType, StructName)                                                                                                                                                    \
    struct StructName                                                                                                                                                                                  \
    {                                                                                                                                                                                                  \
        inline static int8 compare(const ElementType& p_left, const ElementType& p_right)                                                                                                              \
        {

#define sort_linear2_end(SliceVariable, ElementType, StructName)                                                                                                                                       \
    }                                                                                                                                                                                                  \
    ;                                                                                                                                                                                                  \
    }                                                                                                                                                                                                  \
    ;                                                                                                                                                                                                  \
    auto l_slice_variable_##StructName = (SliceVariable);                                                                                                                                              \
    Sort::Linear2<ElementType, StructName>(l_slice_variable_##StructName, 0);
