#pragma once

template <class _Vector> struct iVector
{
    using Element = typename _Vector::t_Element;
    using ElementValue = typename _Vector::t_ElementValue;

    struct Assert
    {
        template <class ComparedType> inline static constexpr int8 element_type()
        {
#if __DEBUG
            static_assert(sizeof(ElementValue) == sizeof(ComparedType), "iVector_v2 : element_type");
#endif
            return 0;
        };
    };

    _Vector& vector;

    inline uimax get_size() const
    {
        return vector.get_size();
    };

    inline void set_size(const uimax p_new_size)
    {
        vector.set_size(p_new_size);
    };

    inline uimax get_capacity() const
    {
        return vector.get_capacity();
    };

    inline ElementValue& get(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index);
#endif
        return this->get_unchecked(p_index);
    };

    inline const ElementValue& get(const uimax p_index) const
    {
        return ((iVector<_Vector>*)this)->get(p_index);
    };

    inline void set_unchecked(const uimax p_index, const ElementValue& p_element)
    {
        vector.set_unchecked(p_index, p_element);
    };

    inline void set(const uimax p_index, const ElementValue& p_element)
    {
        vector.set(p_index, p_element);
    };

    inline int8 empty() const
    {
        return get_size() == 0;
    };

    inline void clear()
    {
        this->set_size(0);
    };

    inline Slice<ElementValue> to_slice() const
    {
        return vector.to_slice();
    };

    inline int8 insert_array_at(const Slice<ElementValue>& p_elements, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

        return this->insert_array_at_unchecked(p_elements, p_index);
    };

    inline int8 insert_element_at(const ElementValue& p_element, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif
        return this->insert_element_at_unchecked(p_element, p_index);
    };

    inline int8 insert_array_at_always(const Slice<ElementValue>& p_elements, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif
        if (p_index == get_size())
        {
            return this->push_back_array(p_elements);
        }
        else
        {
            return this->insert_array_at_unchecked(p_elements, p_index);
        }
    };

    inline int8 insert_element_at_always(const ElementValue& p_element, const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif

        if (p_index == get_size())
        {
            return this->push_back_element(p_element);
        }
        else
        {
            return this->insert_element_at_unchecked(p_element, p_index);
        }
    };

    inline int8 push_back_array(const Slice<ElementValue>& p_elements)
    {
        this->resize_until_capacity_met(get_size() + p_elements.Size);
        this->copy_memory_at_index(get_size(), p_elements);
        set_size(get_size() + p_elements.Size);

        return 1;
    };

    inline int8 push_back_array_2(const Slice<ElementValue>& p_elements_0, const Slice<ElementValue>& p_elements_1)
    {
        this->resize_until_capacity_met(get_size() + p_elements_0.Size + p_elements_1.Size);
        this->copy_memory_at_index_2(get_size(), p_elements_0, p_elements_1);
        set_size(get_size() + (p_elements_0.Size + p_elements_1.Size));

        return 1;
    };

    inline int8 push_back_array_3(const Slice<ElementValue>& p_elements_0, const Slice<ElementValue>& p_elements_1, const Slice<ElementValue>& p_elements_2)
    {
        this->resize_until_capacity_met(get_size() + p_elements_0.Size + p_elements_1.Size + p_elements_2.Size);
        this->copy_memory_at_index_3(get_size(), p_elements_0, p_elements_1, p_elements_2);
        set_size(get_size() + (p_elements_0.Size + p_elements_1.Size + p_elements_2.Size));

        return 1;
    };

    inline int8 push_back_array_empty(const uimax p_array_size)
    {
        this->resize_until_capacity_met(get_size() + p_array_size);
        set_size(get_size() + p_array_size);
        return 1;
    };

    inline int8 push_back_element_empty()
    {
        this->resize_until_capacity_met(get_size() + 1);
        set_size(get_size() + 1);
        return 1;
    };

    inline int8 push_back_element(const ElementValue& p_element)
    {
        this->resize_until_capacity_met(get_size() + 1);
        this->set_unchecked(get_size(), p_element);
        set_size(get_size() + 1);

        return 1;
    };

    inline int8 pop_back_array(const uimax p_element_nb)
    {
        set_size(get_size() - p_element_nb);
        return 1;
    };

    inline int8 pop_back()
    {
        set_size(get_size() - 1);
        return 1;
    };

    inline ElementValue pop_back_return()
    {
        ElementValue l_return = this->get(get_size() - 1);
        this->pop_back();
        return l_return;
    };

    inline int8 erase_array_at(const uimax p_index, const uimax p_element_nb)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_check(p_index + p_element_nb);
        this->bound_lastelement_check(p_index); // use vector_pop_back_array
#endif

        this->move_memory_up(p_index + p_element_nb, p_element_nb);
        set_size(get_size() - p_element_nb);

        return 1;
    };

    inline int8 erase_element_at(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
        this->bound_lastelement_check(p_index); // use vector_pop_back
#endif

        this->move_memory_up(p_index + 1, 1);
        set_size(get_size() - 1);

        return 1;
    };

    inline int8 erase_element_at_always(const uimax p_index)
    {
#if __DEBUG
        this->bound_check(p_index);
#endif

        if (p_index == get_size() - 1)
        {
            return this->pop_back();
        }
        else
        {
            return this->erase_element_at(p_index);
        }
    };

    inline void erase_array_at_always(const uimax p_index, const uimax p_size)
    {
        uimax l_insert_head = p_index + p_size;
#if __DEBUG
        this->bound_check(l_insert_head);
#endif

        if (l_insert_head == get_size() - 1)
        {
            this->pop_back_array(p_size);
        }
        else
        {
            this->erase_array_at(p_index, p_size);
        }
    };

    template <class Predicate_t> inline void erase_if(const Predicate_t& p_predicate_func)
    {
        for (loop_reverse(i, 0, this->get_size()))
        {
            ElementValue& l_element = this->get(i);
            if (p_predicate_func(l_element))
            {
                this->erase_element_at_always(i);
            }
        }
    };

    template <class ComparedElementType, class EqualityFunc> inline void erase_all_elements_that_matches_element_v2(const ComparedElementType& p_compared_element, const EqualityFunc& p_equality_func)
    {
        this->erase_if([&](const ElementValue& p_element) {
            return p_equality_func(p_element, p_compared_element);
        });
    };

    template <class ElementTypeCompared, class EqualityFunc>
    inline void erase_all_elements_that_matches_any_of_element_v2(const Slice<ElementTypeCompared>& p_compared_elements, const EqualityFunc& p_equality_func)
    {
        this->erase_if([&](const ElementValue& p_element) {
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

  private:
    inline void move_memory_down(const uimax p_break_index, const uimax p_move_delta)
    {
        vector.move_memory_down(p_break_index, p_move_delta);
    };

    inline void move_memory_up(const uimax p_break_index, const uimax p_move_delta)
    {
        vector.move_memory_up(p_break_index, p_move_delta);
    };

    inline void copy_memory_at_index(const uimax p_copy_index, const Slice<ElementValue>& p_elements)
    {
        vector.copy_memory_at_index(p_copy_index, p_elements);
    };

    inline void copy_memory_at_index_2(const uimax p_copy_index, const Slice<ElementValue>& p_elements_1, const Slice<ElementValue>& p_elements_2)
    {
        vector.copy_memory_at_index_2(p_copy_index, p_elements_1, p_elements_2);
    };

    inline void copy_memory_at_index_3(const uimax p_copy_index, const Slice<ElementValue>& p_elements_1, const Slice<ElementValue>& p_elements_2, const Slice<ElementValue>& p_elements_3)
    {
        vector.copy_memory_at_index_3(p_copy_index, p_elements_1, p_elements_2, p_elements_3);
    };

    inline void resize_until_capacity_met(const uimax p_desired_capacity)
    {
        vector.resize_until_capacity_met(p_desired_capacity);
    };

    inline void bound_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index > get_size())
        {
            abort();
        }
#endif
    };

    inline void bound_head_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index == get_size())
        {
            abort();
        }
#endif
    };

    inline void bound_lastelement_check(const uimax p_index)
    {
#if __DEBUG
        if (p_index == get_size() - 1)
        {
            abort();
        }
#endif
    };

    inline ElementValue& get_unchecked(const uimax p_index)
    {
        return vector.get_unchecked(p_index);
    };

    inline int8 insert_element_at_unchecked(const ElementValue& p_element, const uimax p_index)
    {
        this->resize_until_capacity_met(get_size() + 1);
        this->move_memory_down(p_index, 1);
        this->set(p_index, p_element);
        set_size(get_size() + 1);

        return 1;
    };

    inline int8 insert_array_at_unchecked(const Slice<ElementValue>& p_elements, const uimax p_index)
    {
        this->resize_until_capacity_met(get_size() + p_elements.Size);
        this->move_memory_down(p_index, p_elements.Size);
        this->copy_memory_at_index(p_index, p_elements);

        set_size(get_size() + p_elements.Size);

        return 1;
    };
};

#define iVector_v2_functions_forward_declare(VectorType)                                                                                                                                               \
    inline t_ElementValue& get(const uimax p_index)                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.get(p_index);                                                                                                                                                \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline const t_ElementValue& get(const uimax p_index) const                                                                                                                                        \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*(VectorType*)this}.get(p_index);                                                                                                                                   \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 empty() const                                                                                                                                                                          \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*(VectorType*)this}.empty();                                                                                                                                        \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 insert_array_at(const Slice<t_ElementValue>& p_elements, const uimax p_index)                                                                                                          \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.insert_array_at(p_elements, p_index);                                                                                                                        \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 insert_element_at(const t_ElementValue& p_element, const uimax p_index)                                                                                                                \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.insert_element_at(p_element, p_index);                                                                                                                       \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 push_back_array(const Slice<t_ElementValue>& p_elements)                                                                                                                               \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.push_back_array(p_elements);                                                                                                                                 \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 push_back_array_2(const Slice<t_ElementValue>& p_elements_0, const Slice<t_ElementValue>& p_elements_1)                                                                                \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.push_back_array_2(p_elements_0, p_elements_1);                                                                                                               \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 push_back_array_3(const Slice<t_ElementValue>& p_elements_0, const Slice<t_ElementValue>& p_elements_1, const Slice<t_ElementValue>& p_elements_2)                                     \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.push_back_array_3(p_elements_0, p_elements_1, p_elements_2);                                                                                                 \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 push_back_array_empty(const uimax p_array_size)                                                                                                                                        \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.push_back_array_empty(p_array_size);                                                                                                                         \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 push_back_element_empty()                                                                                                                                                              \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.push_back_element_empty();                                                                                                                                   \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 push_back_element(const t_ElementValue& p_element)                                                                                                                                     \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.push_back_element(p_element);                                                                                                                                \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 insert_array_at_always(const Slice<t_ElementValue>& p_elements, const uimax p_index)                                                                                                   \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.insert_array_at_always(p_elements, p_index);                                                                                                                 \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 insert_element_at_always(const t_ElementValue& p_element, const uimax p_index)                                                                                                         \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.insert_element_at_always(p_element, p_index);                                                                                                                \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 erase_array_at(const uimax p_index, const uimax p_element_nb)                                                                                                                          \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.erase_array_at(p_index, p_element_nb);                                                                                                                       \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 erase_element_at(const uimax p_index)                                                                                                                                                  \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.erase_element_at(p_index);                                                                                                                                   \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 pop_back_array(const uimax p_element_nb)                                                                                                                                               \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.pop_back_array(p_element_nb);                                                                                                                                \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 pop_back()                                                                                                                                                                             \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.pop_back();                                                                                                                                                  \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline int8 erase_element_at_always(const uimax p_index)                                                                                                                                           \
    {                                                                                                                                                                                                  \
        return iVector<VectorType>{*this}.erase_element_at_always(p_index);                                                                                                                            \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    inline void erase_array_at_always(const uimax p_index, const uimax p_size)                                                                                                                         \
    {                                                                                                                                                                                                  \
        iVector<VectorType>{*this}.erase_array_at_always(p_index, p_size);                                                                                                                             \
    };                                                                                                                                                                                                 \
                                                                                                                                                                                                       \
    template <class Predicate_t> inline void erase_if(const Predicate_t& p_predicate)                                                                                                                  \
    {                                                                                                                                                                                                  \
        iVector<VectorType>{*this}.erase_if(p_predicate);                                                                                                                                              \
    };
