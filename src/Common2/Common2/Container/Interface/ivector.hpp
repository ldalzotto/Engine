#pragma once

template <class _Vector> struct iVector
{
    _Vector& vector;

    using _ElementValue = typename _Vector::_ElementValue;
    using _SizeType = typename _Vector::_SizeType;

    struct Assert
    {
        template <class ComparedType> inline static constexpr int8 element_type()
        {
#if __DEBUG
            static_assert(sizeof(_ElementValue) == sizeof(ComparedType), "iVector : element_type");
#endif
            return 0;
        };
    };

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

    inline _ElementValue pop_back_return()
    {
        _ElementValue l_return = this->get(this->get_size() - 1);
        this->pop_back();
        return l_return;
    };

    inline void pop_back_array(const uimax p_index)
    {
        this->vector.pop_back_array(p_index);
    };

    inline Slice<_ElementValue> to_slice() const
    {
        return this->vector.to_slice();
    };

    template <class Predicate_t> inline void erase_if(const Predicate_t& p_predicate_func)
    {
        for (loop_reverse(i, 0, this->get_size()))
        {
            _ElementValue& l_element = this->get(i);
            if (p_predicate_func(l_element))
            {
                this->erase_element_at_always(i);
            }
        }
    };

    template <class ComparedElementType, class EqualityFunc> inline void erase_all_elements_that_matches_element_v2(const ComparedElementType& p_compared_element, const EqualityFunc& p_equality_func)
    {
        this->erase_if([&](const _ElementValue& p_element) {
            return p_equality_func(p_element, p_compared_element);
        });
    };

    template <class ElementTypeCompared, class EqualityFunc>
    inline void erase_all_elements_that_matches_any_of_element_v2(const Slice<ElementTypeCompared>& p_compared_elements, const EqualityFunc& p_equality_func)
    {
        this->erase_if([&](const _ElementValue& p_element) {
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
