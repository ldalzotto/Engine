#pragma once

#define ShadowVector(ElementType) ShadowVector_##ElementType

#define sv_static_assert_element_type(p_shadow_vector_type, p_compared_type)                                                                                                                                \
    static_assert(sizeof(decltype(p_shadow_vector_type::CompileType::Element)) == sizeof(p_compared_type), "ShadowVector type assertion")

#define sv_func_get_size() get_size()
#define sv_func_empty() empty()
#define sv_func_clear() clear()
#define sv_func_get(p_index) get(p_index)
#define sv_func_erase_element_at(p_index) erase_element_at(p_index)
#define sv_func_erase_element_at_always(p_index) erase_element_at_always(p_index)
#define sv_func_erase_element_at(p_index) erase_element_at(p_index)
#define sv_func_erase_array_at(p_index, p_element_nb) erase_array_at(p_index, p_element_nb)
#define sv_func_push_back_element(p_element) push_back_element(p_element)
#define sv_func_insert_element_at(p_element, p_index) insert_element_at(p_element, p_index)
#define sv_func_push_back_array(p_array) push_back_array(p_array)
#define sv_func_push_back_array_empty(p_element_count) push_back_array_empty(p_element_count)
#define sv_func_insert_array_at(p_array, p_index) insert_array_at(p_array, p_index)
#define sv_func_pop_back() pop_back()
#define sv_func_pop_back_array(p_element_nb) pop_back_array(p_element_nb)
#define sv_func_to_slice() to_slice()

#define sv_c_get_size(p_shadow_vector) (p_shadow_vector).sv_func_get_size()
#define sv_c_empty(p_shadow_vector) (p_shadow_vector).sv_func_empty()
#define sv_c_clear(p_shadow_vector) (p_shadow_vector).sv_func_clear()
#define sv_c_get(p_shadow_vector, p_index) (p_shadow_vector).sv_func_get(p_index)
#define sv_c_erase_element_at_always(p_shadow_vector, p_index) (p_shadow_vector).sv_func_erase_element_at_always(p_index)
#define sv_c_erase_element_at(p_shadow_vector, p_index) (p_shadow_vector).sv_func_erase_element_at(p_index)
#define sv_c_erase_array_at(p_shadow_vector, p_index, p_element_nb) (p_shadow_vector).sv_func_erase_array_at(p_index, p_element_nb)
#define sv_c_push_back_element(p_shadow_vector, p_element) (p_shadow_vector).sv_func_push_back_element(p_element)
#define sv_c_insert_element_at(p_shadow_vector, p_element, p_index) (p_shadow_vector).sv_func_insert_element_at(p_element, p_index)
#define sv_c_push_back_array(p_shadow_vector, p_array) (p_shadow_vector).sv_func_push_back_array(p_array)
#define sv_c_push_back_array_empty(p_shadow_vector, p_element_count) (p_shadow_vector).sv_func_push_back_array_empty(p_element_count)
#define sv_c_insert_array_at(p_shadow_vector, p_array, p_index) (p_shadow_vector).sv_func_insert_array_at(p_array, p_index)
#define sv_c_pop_back(p_shadow_vector) (p_shadow_vector).sv_func_pop_back()
#define sv_c_pop_back_array(p_shadow_vector, p_element_nb) (p_shadow_vector).sv_func_pop_back_array(p_element_nb)
#define sv_c_to_slice(p_shadow_vector) (p_shadow_vector).sv_func_to_slice()

struct VectorAlgorithm
{
    template <class ShadowVector(ElementType), class Predicate_t> inline static void erase_if(ShadowVector(ElementType) & p_vector, const Predicate_t& p_predicate)
    {
        uimax l_size = sv_c_get_size(p_vector);
        for (loop_reverse(i, 0, l_size))
        {
            if (p_predicate(sv_c_get(p_vector, i)))
            {
                sv_c_erase_element_at_always(p_vector, i);
            }
        }
    };

    template <class ShadowVector(ElementType), class ElementType>
    inline static void erase_all_elements_that_matches_element(ShadowVector(ElementType) & p_vector, const ElementType& p_compared_element)
    {
        sv_static_assert_element_type(ShadowVector(ElementType), ElementType);

        VectorAlgorithm::erase_if(p_vector, [&](const ElementType& p_element) {
            return p_element == p_compared_element;
        });
    };

    template <class ShadowVector(ElementType), class ElementType>
    inline static void erase_all_elements_that_matches_any_of_element(ShadowVector(ElementType) & p_vector, const Slice<ElementType>& p_compared_elements)
    {
        sv_static_assert_element_type(ShadowVector(ElementType), ElementType);

        VectorAlgorithm::erase_if(p_vector, [&](const ElementType& p_element) {
            for (loop(i, 0, p_compared_elements.Size))
            {
                if (p_element == p_compared_elements.get(i))
                {
                    return 1;
                }
            }
            return 0;
        });
    };
};
