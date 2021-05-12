#pragma once

// TODO -> use templates instead of macros
#define ShadowVector(ElementType) ShadowVector_##ElementType

// TODO -> use templates instead of macros
#define sv_static_assert_element_type(p_shadow_vector_type, p_compared_type)                                                                                                                           \
    static_assert(sizeof(typename p_shadow_vector_type::_ElementValue) == sizeof(p_compared_type), "ShadowVector type assertion")

template <class _Vector> struct ShadowVector_v3
{
    _Vector& vector;

    // using _ElementRef = typename _Vector::_ElementRef;
    using _ElementValue = typename _Vector::_ElementValue;
    using _SizeType = typename _Vector::_SizeType;

    inline _SizeType get_size() const
    {
        return this->vector.get_size();
    };

    inline int8 empty() const
    {
        return this->vector.empty();
    };

    inline void clear()
    {
        this->vector.clear();
    };

    inline _ElementValue& get(const uimax p_index)
    {
        return this->vector.get(p_index);
    };

    inline const _ElementValue& get(const uimax p_index) const
    {
        return this->vector.get(p_index);
    };

    inline void set(const uimax p_index, const _ElementValue& p_element)
    {
        return this->vector.set(p_index, p_element);
    };

    inline void erase_element_at(const uimax p_index)
    {
        this->vector.erase_element_at(p_index);
    };

    inline void erase_element_at_always(const uimax p_index)
    {
        this->vector.erase_element_at_always(p_index);
    };

    inline void push_back_element(const _ElementValue& p_element)
    {
        this->vector.push_back_element(p_element);
    };

    inline void push_back_element_empty()
    {
        this->vector.push_back_element_empty();
    };

    inline void insert_element_at(const _ElementValue& p_element, const uimax p_index)
    {
        this->vector.insert_element_at(p_element, p_index);
    };

    inline void push_back_array(const Slice<_ElementValue>& p_array)
    {
        this->vector.push_back_array(p_array);
    };

    inline void push_back_array_empty(const uimax p_element_count)
    {
        this->vector.push_back_array_empty(p_element_count);
    };

    inline void insert_array_at(const Slice<_ElementValue>& p_array, const uimax p_index)
    {
        this->vector.insert_array_at(p_array, p_index);
    };

    inline void erase_array_at(const uimax p_erase_begin_index, const uimax p_erase_nb)
    {
        this->vector.erase_array_at(p_erase_begin_index, p_erase_nb);
    };

    inline void pop_back()
    {
        this->vector.pop_back();
    };

    inline void pop_back_array(const uimax p_index)
    {
        this->vector.pop_back_array(p_index);
    };

    inline Slice<_ElementValue> to_slice() const
    {
        return this->vector.to_slice();
    };
};

struct ShadowVector_v2
{
    template <class Container> inline static typename Container::_SizeType get_size(const Container& p_container)
    {
        return p_container.get_size();
    };

    template <class Container> inline static int8 empty(const Container& p_container)
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

    template <class Container> inline static const typename Container::_ElementValue& get(const Container& p_container, const uimax p_index)
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
        return p_container.to_slice();
    };
};

// TODO -> use templates instead of macros
#define sv_func_get_size() get_size()
#define sv_func_get(p_index) get(p_index)
#define sv_func_erase_element_at_always(p_index) erase_element_at_always(p_index)
#define sv_func_push_back_element(p_element) push_back_element(p_element)
#define sv_func_push_back_array(p_array) push_back_array(p_array)
#define sv_func_to_slice() to_slice()

#define sv_c_get_size(p_shadow_vector) (p_shadow_vector).sv_func_get_size()
#define sv_c_get(p_shadow_vector, p_index) (p_shadow_vector).sv_func_get(p_index)
#define sv_c_erase_element_at_always(p_shadow_vector, p_index) (p_shadow_vector).sv_func_erase_element_at_always(p_index)
#define sv_c_push_back_array(p_shadow_vector, p_array) (p_shadow_vector).sv_func_push_back_array(p_array)

struct VectorAlgorithm
{
    template <class Container> inline static typename Container::_ElementValue pop_back_return(ShadowVector_v3<Container>& p_container)
    {
        typename Container::_ElementValue l_return_value = p_container.get(p_container.get_size() - 1);
        p_container.pop_back();
        return l_return_value;
    };

    template <class Container, class Predicate_t> inline static void erase_if_v2(ShadowVector_v3<Container>& p_vector, const Predicate_t& p_predicate_func)
    {
        for (loop_reverse(i, 0, p_vector.get_size()))
        {
            typename Container::_ElementValue& l_element = p_vector.get(i);
            if (p_predicate_func(l_element))
            {
                p_vector.erase_element_at_always(i);
            }
        }
    };

    template <class Container, class ComparedElementType, class EqualityFunc>
    inline static void erase_all_elements_that_matches_element_v2(ShadowVector_v3<Container>& p_vector, const ComparedElementType& p_compared_element, const EqualityFunc& p_equality_func)
    {
        erase_if_v2(p_vector, [&](const typename Container::_ElementValue& p_element) {
            return p_equality_func(p_element, p_compared_element);
        });
    };

    template <class Container, class ElementTypeCompared, class EqualityFunc>
    inline static void erase_all_elements_that_matches_any_of_element_v2(ShadowVector_v3<Container>& p_container, const Slice<ElementTypeCompared>& p_compared_elements,
                                                                         const EqualityFunc& p_equality_func)
    {
        erase_if_v2(p_container, [&](const typename Container::_ElementValue& p_element) {
            for (loop(i, 0, p_compared_elements.get_size()))
            {
                if (p_equality_func(p_element, p_compared_elements.get(i)))
                {
                    return 1;
                }
            };
            return 0;
        });
    };
};
