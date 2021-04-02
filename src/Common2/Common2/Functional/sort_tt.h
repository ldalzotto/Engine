
#ifdef sort_tt_linear_3

#ifndef pm_compare_func_closure
#define pm_compare_func_closure void*
#endif

inline void pm_func_name(Slice<pm_type_element>* p_slice, const uimax p_start_index, pm_compare_func_closure p_compare_closure)
{
    for (loop(i, p_start_index, p_slice->Size))
    {
        pm_type_element* l_left = Slice_get(p_slice, i);
        for (loop(j, i, p_slice->Size))
        {
            pm_type_element* l_right = Slice_get(p_slice, j);
            if (pm_compare_func_name(l_left, l_right, p_compare_closure))
            {
                pm_type_element l_left_tmp = *l_left;
                *l_left = *l_right;
                *l_right = l_left_tmp;
            }
        }
    }
};

#undef pm_type_element
#undef pm_func_name
#undef pm_compare_func_name
#undef pm_compare_func_closure
#undef sort_tt_linear_3

#endif