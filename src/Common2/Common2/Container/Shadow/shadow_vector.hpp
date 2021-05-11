#pragma once

// TODO -> use templates instead of macros
#define ShadowVector(ElementType) ShadowVector_##ElementType

// TODO -> use templates instead of macros
#define sv_static_assert_element_type(p_shadow_vector_type, p_compared_type)                                                                                                                           \
    static_assert(sizeof(typename p_shadow_vector_type::_ElementValue) == sizeof(p_compared_type), "ShadowVector type assertion")

struct ShadowVector_v2
{
    template <class Container> inline static typename Container::_SizeType get_size(Container& p_container)
    {
        return p_container.get_size();
    };

    template <class Container> inline static int8 empty(Container& p_container)
    {
        return p_container.empty();
    };

    template <class Container> inline static void clear(Container& p_container)
    {
        return p_container.clear();
    };

    template <class Container> inline static typename Container::_ElementValue& get(Container& p_container, const uimax p_index)
    {
        return p_container.get(p_index);
    };

    template <class Container> inline static void set(Container& p_container, const uimax p_index, const typename Container::_ElementValue& p_element)
    {
        return p_container.set(p_index, p_element);
    };

    template <class Container> inline static void erase_element_at(Container& p_container, const uimax p_index)
    {
        p_container.erase_element_at(p_index);
    };

    template <class Container> inline static void erase_element_at_always(Container& p_container, const uimax p_index)
    {
        p_container.erase_element_at_always(p_index);
    };

    template <class Container> inline static void push_back_element(Container& p_container, const typename Container::_ElementValue& p_element)
    {
        p_container.push_back_element(p_element);
    };

    template <class Container> inline static void push_back_element_empty(Container& p_container)
    {
        p_container.push_back_element_empty();
    };

    template <class Container> inline static void insert_element_at(Container& p_container, const typename Container::_ElementValue& p_element, const uimax p_index)
    {
        p_container.insert_element_at(p_element, p_index);
    };

    template <class Container> inline static void push_back_array(Container& p_container, const Slice<typename Container::_ElementValue>& p_array)
    {
        p_container.push_back_array(p_array);
    };

    template <class Container> inline static void push_back_array_empty(Container& p_container, const uimax p_element_count)
    {
        p_container.push_back_array_empty(p_element_count);
    };

    template <class Container> inline static void insert_array_at(Container& p_container, const Slice<typename Container::_ElementValue>& p_array, const uimax p_index)
    {
        p_container.insert_array_at(p_array, p_index);
    };

    template <class Container> inline static void pop_back(Container& p_container)
    {
        p_container.pop_back();
    };

    template <class Container> inline static void pop_back_array(Container& p_container, const uimax p_index)
    {
        p_container.pop_back_array(p_index);
    };

    template <class Container> inline static Slice<typename Container::_ElementValue> to_slice(Container& p_container)
    {
        p_container.to_slice();
    };
};

// TODO -> use templates instead of macros
#define sv_func_get_size() get_size()
#define sv_func_empty() empty()
#define sv_func_clear() clear()
#define sv_func_get(p_index) get(p_index)
#define sv_func_set(p_index, p_element) set(p_index, p_element)
#define sv_func_erase_element_at(p_index) erase_element_at(p_index)
#define sv_func_erase_element_at_always(p_index) erase_element_at_always(p_index)
#define sv_func_erase_element_at(p_index) erase_element_at(p_index)
#define sv_func_erase_array_at(p_index, p_element_nb) erase_array_at(p_index, p_element_nb)
#define sv_func_push_back_element(p_element) push_back_element(p_element)
#define sv_func_push_back_element_empty() push_back_element_empty()
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
#define sv_c_set(p_shadow_vector, p_index, p_element) (p_shadow_vector).sv_func_set(p_index, p_element)
#define sv_c_erase_element_at_always(p_shadow_vector, p_index) (p_shadow_vector).sv_func_erase_element_at_always(p_index)
#define sv_c_erase_element_at(p_shadow_vector, p_index) (p_shadow_vector).sv_func_erase_element_at(p_index)
#define sv_c_erase_array_at(p_shadow_vector, p_index, p_element_nb) (p_shadow_vector).sv_func_erase_array_at(p_index, p_element_nb)
#define sv_c_push_back_element(p_shadow_vector, p_element) (p_shadow_vector).sv_func_push_back_element(p_element)
#define sv_c_push_back_element_empty(p_shadow_vector) (p_shadow_vector).sv_func_push_back_element_empty()
#define sv_c_insert_element_at(p_shadow_vector, p_element, p_index) (p_shadow_vector).sv_func_insert_element_at(p_element, p_index)
#define sv_c_push_back_array(p_shadow_vector, p_array) (p_shadow_vector).sv_func_push_back_array(p_array)
#define sv_c_push_back_array_empty(p_shadow_vector, p_element_count) (p_shadow_vector).sv_func_push_back_array_empty(p_element_count)
#define sv_c_insert_array_at(p_shadow_vector, p_array, p_index) (p_shadow_vector).sv_func_insert_array_at(p_array, p_index)
#define sv_c_pop_back(p_shadow_vector) (p_shadow_vector).sv_func_pop_back()
#define sv_c_pop_back_array(p_shadow_vector, p_element_nb) (p_shadow_vector).sv_func_pop_back_array(p_element_nb)
#define sv_c_to_slice(p_shadow_vector) (p_shadow_vector).sv_func_to_slice()

struct VectorAlgorithm
{
    template <class Container> inline static typename Container::_ElementValue pop_back_return(Container& p_container)
    {
        typename Container::_ElementValue l_return_value = p_container.get(ShadowVector_v2::get_size(p_container) - 1);
        p_container.pop_back();
        return l_return_value;
    };

    template <class Container, class ForeachFunc> inline static void iterator(Container& p_container, const ForeachFunc& p_foreach_func)
    {
        typename Container::_SizeType l_size = ShadowVector_v2::get_size(p_container);
        for (loop(i, 0, l_size))
        {
            p_foreach_func(p_container.get(i));
        }
    };

    template <class Container, class ForeachFunc> inline static void backward_iterator_index(Container& p_container, const ForeachFunc& p_foreach_func)
    {
        typename Container::_SizeType l_size = ShadowVector_v2::get_size(p_container);
        for (loop_reverse(i, 0, l_size))
        {
            p_foreach_func(i, p_container.get(i));
        }
    };

    template <class Container, class Predicate_t> inline static void erase_if(Container& p_container, const Predicate_t& p_predicate_func)
    {
        backward_iterator_index(p_container, [&](const uimax i, const typename Container::_ElementValue& p_element) {
            if (p_predicate_func(p_element))
            {
                p_container.erase_element_at_always(i);
            }
        });
    };

    template <class Container, class ComparedElementType, class EqualityFunc>
    inline static void erase_all_elements_that_matches_element(Container& p_container, const ComparedElementType& p_compared_element, const EqualityFunc& p_equality_func)
    {
        erase_if(p_container, [&](const typename Container::_ElementValue& p_element) {
            return p_equality_func(p_element, p_compared_element);
        });
    };

    template <class Container, class ContainerCompared, class EqualityFunc>
    inline static void erase_all_elements_that_matches_any_of_element(Container& p_container, const ContainerCompared& p_compared_elements, const EqualityFunc& p_equality_func)
    {
        erase_if(p_container, [&](const typename Container::_ElementValue& p_element) {
            int8 l_element_erased = 0;
            iterator(p_compared_elements, [&](const typename ContainerCompared::_ElementValue p_compared_element) {
                if (p_equality_func(p_element, p_compared_element))
                {
                    l_element_erased = 1;
                    return;
                }
            });
            return l_element_erased;
        });
    };
};
