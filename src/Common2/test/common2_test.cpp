
#include "Common2/common2.hpp"

template <class ElementType> inline void assert_span_unitialized(Span<ElementType>* p_span)
{
    assert_true(p_span->Capacity == 0);
    assert_true(p_span->Memory == NULL);
};

inline void slice_span_test()
{
    Span<uimax> l_span_sizet = Span<uimax>::build(NULL, 0);

    // When resizing the span, new memory is allocated
    {
        uimax l_new_capacity = 10;
        l_span_sizet.resize(10);
        assert_true(l_span_sizet.Capacity == l_new_capacity);
        assert_true(l_span_sizet.Memory != NULL);
    }

    // When freeing the span, it's structure is resetted
    {
        l_span_sizet.free();
        assert_span_unitialized(&l_span_sizet);
    }

    // Move memory
    {
        l_span_sizet.resize(10);

        l_span_sizet.get(0) = 3;
        l_span_sizet.get(1) = 5;
        l_span_sizet.get(3) = 10;

        l_span_sizet.slice.move_memory_down(1, 3, 1);
        assert_true(l_span_sizet.get(4) == 10);

        l_span_sizet.slice.move_memory_up(1, 4, 2);
        assert_true(l_span_sizet.get(2) == 10);

        assert_true(l_span_sizet.get(0) == 3);
        assert_true(l_span_sizet.get(1) == 5);

        l_span_sizet.slice.move_memory_up(2, 2, 2);

        assert_true(l_span_sizet.get(0) == 10);
        assert_true(l_span_sizet.get(1) == 10);
    }

    l_span_sizet.free();

    {
        l_span_sizet = Span<uimax>::allocate(10);

        SliceN<uimax, 4> l_slice_1 = {0, 1, 2, 3};
        SliceN<uimax, 4> l_slice_2 = {5, 6, 7, 8};
        l_span_sizet.slice.copy_memory_at_index_2(1, l_slice_1.to_slice(), l_slice_2.to_slice());

        assert_true(l_span_sizet.slice.slide_rv(1).compare(l_slice_1.to_slice()));
        assert_true(l_span_sizet.slice.slide_rv(5).compare(l_slice_2.to_slice()));

        l_span_sizet.free();
    }
    {
        SliceN<uimax, 4> l_slice = {15, 26, 78, 10};
        l_span_sizet = Span<uimax>::allocate_slice(l_slice.to_slice());
        assert_true(l_span_sizet.Capacity == 4);
        assert_true(l_span_sizet.slice.compare(l_slice.to_slice()));

        l_span_sizet.free();
    }
};

inline void vector_test()
{
    Vector<uimax> l_vector_sizet = Vector<uimax>::build_zero_size((uimax*)NULL, 0);

    // vector_push_back_array
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 5);

        l_vector_sizet.push_back_array(l_elements_slice);
        assert_true(l_vector_sizet.Size == l_old_size + 5);
        for (loop(i, l_old_size, l_vector_sizet.Size))
        {
            assert_true(l_vector_sizet.get(i) == l_elements[i - l_old_size]);
        }
    }

    // push_back_array_empty
    {
        uimax l_old_size = l_vector_sizet.Size;
        l_vector_sizet.push_back_array_empty(5);
        assert_true(l_vector_sizet.Size == (l_old_size + (uimax)5));
    }

    // vector_push_back_element
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_element = 25;
        l_vector_sizet.push_back_element(l_element);
        assert_true(l_vector_sizet.Size == l_old_size + 1);
        assert_true(l_vector_sizet.get(l_vector_sizet.Size - 1) == l_element);
    }

    // vector_insert_array_at
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 5);
        l_vector_sizet.insert_array_at(l_elements_slice, 0);
        assert_true(l_vector_sizet.Size == l_old_size + 5);
        for (loop_int16(i, 0, 5))
        {
            assert_true((l_vector_sizet.get(i)) == (uimax)i);
        }

        l_vector_sizet.insert_array_at(l_elements_slice, 3);
        assert_true(l_vector_sizet.Size == l_old_size + 10);
        for (loop_int16(i, 0, 3))
        {
            assert_true((l_vector_sizet.get(i)) == l_elements[i]);
        }
        // Middle insertion
        for (loop_int16(i, 3, 8))
        {
            assert_true((l_vector_sizet.get(i)) == l_elements[i - cast(uimax, 3)]);
        }
        for (loop_int16(i, 8, 10))
        {
            assert_true((l_vector_sizet.get(i)) == l_elements[i - cast(uimax, 5)]);
        }
    }

    // vector_insert_element_at
    {
        uimax l_element = 20;
        uimax l_old_size = l_vector_sizet.Size;

        l_vector_sizet.insert_element_at(l_element, 7);
        assert_true(l_vector_sizet.get(7) == l_element);
        assert_true(l_vector_sizet.Size == l_old_size + 1);

        l_vector_sizet.insert_element_at(cast(uimax, 20), 9);
    }

    // vector_erase_element_at
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_erase_index = 1;
        uimax l_element_after = l_vector_sizet.get(l_erase_index + 1);
        l_vector_sizet.erase_element_at(1);
        assert_true(l_vector_sizet.Size == l_old_size - 1);
        assert_true(l_vector_sizet.get(1) == l_element_after);
    }

    // vector_erase_array_at
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_erase_begin_index = 3;
        const uimax l_erase_nb = 6;
        const uimax l_old_element_check_nb = 3;

        uimax l_old_values[l_old_element_check_nb];
        for (loop(i, l_erase_begin_index + l_erase_nb, (l_erase_begin_index + l_erase_nb) + l_old_element_check_nb))
        {
            l_old_values[i - (l_erase_begin_index + l_erase_nb)] = l_vector_sizet.get(i);
        }

        l_vector_sizet.erase_array_at(l_erase_begin_index, l_erase_nb);

        assert_true(l_vector_sizet.Size == l_old_size - l_erase_nb);
        for (loop(i, 0, l_old_element_check_nb))
        {
            assert_true(l_vector_sizet.get(l_erase_begin_index + i) == l_old_values[i]);
        }
    }

    // vector_pop_back
    {
        uimax l_old_size = l_vector_sizet.Size;
        l_vector_sizet.pop_back();
        assert_true(l_vector_sizet.Size == l_old_size - 1);
    }

    // vector_pop_back_array
    {
        uimax l_old_size = l_vector_sizet.Size;
        l_vector_sizet.pop_back_array(3);
        assert_true(l_vector_sizet.Size == l_old_size - 3);
    }

    // When freeing the vcetor, it's structure is resetted
    {
        l_vector_sizet.free();
        assert_true(l_vector_sizet.Size == 0);
        assert_span_unitialized(&l_vector_sizet.Memory);
    }
};

inline void hashmap_test(){{struct Key{uimax v1, v2, v3;
}
;

HashMap<Key, uimax> l_map = HashMap<Key, uimax>::allocate(2);

// push_key_value
l_map.push_key_value_nothashed(Key{0, 0, 0}, 0);
l_map.push_key_value_nothashed(Key{0, 0, 1}, 1);

assert_true(l_map.get_capacity() == 2);
assert_true(*l_map.get_value_nothashed(Key{0, 0, 0}) == 0);
assert_true(*l_map.get_value_nothashed(Key{0, 0, 1}) == 1);

l_map.push_key_value_nothashed(Key{0, 1, 1}, 2);

assert_true(l_map.get_capacity() == 4);
assert_true(*l_map.get_value_nothashed(Key{0, 0, 0}) == 0);
assert_true(*l_map.get_value_nothashed(Key{0, 0, 1}) == 1);
assert_true(*l_map.get_value_nothashed(Key{0, 1, 1}) == 2);

// put_value
l_map.put_value_nothashed(Key{0, 0, 1}, 10);
assert_true(*l_map.get_value_nothashed(Key{0, 0, 1}) == 10);

// erase_key
assert_true(l_map.has_key_nothashed(Key{0, 0, 1}));
l_map.erase_key_nothashed(Key{0, 0, 1});
assert_true(!l_map.has_key_nothashed(Key{0, 0, 1}));

l_map.free();
}

{
    HashMap<hash_t, uimax> l_map = HashMap<hash_t, uimax>::allocate(2);

    l_map.push_key_value_nothashed(1, 0);
    l_map.push_key_value_nothashed(0, 1);

    assert_true(l_map.get_capacity() == 2);
    assert_true(*l_map.get_value_nothashed(1) == 0);
    assert_true(*l_map.get_value_nothashed(0) == 1);

    l_map.push_key_value_nothashed(12, 2);

    assert_true(l_map.get_capacity() == 8);

    assert_true(*l_map.get_value_nothashed(1) == 0);
    assert_true(*l_map.get_value_nothashed(0) == 1);
    assert_true(*l_map.get_value_nothashed(12) == 2);

    // put_value
    l_map.put_value_nothashed(0, 10);
    assert_true(*l_map.get_value_nothashed(0) == 10);

    // erase_key
    assert_true(l_map.has_key_nothashed(0));
    l_map.erase_key_nothashed(0);
    assert_true(!l_map.has_key_nothashed(0));

    l_map.free();
}
}
;

inline void pool_test()
{
    Pool<uimax> l_pool_sizet = Pool<uimax>::allocate(10);

    {
        assert_true(l_pool_sizet.get_memory() != NULL);
        assert_true(l_pool_sizet.get_capacity() == 10);
        assert_true(l_pool_sizet.get_size() == 0);
    }

    // pool_alloc_element - allocate new element
    {
        assert_true(l_pool_sizet.get_free_size() == 0);

        uimax l_element = 3;
        Token(uimax) l_token = l_pool_sizet.alloc_element(l_element);

        assert_true(tk_v(l_token) == 0);
        assert_true(l_pool_sizet.get(l_token) == l_element);
    }

    // pool_release_element - release elements
    {
        Token(uimax) l_token = Token(uimax){0};
        l_pool_sizet.release_element(l_token);

        // memory is not deallocated
        assert_true(l_pool_sizet.get_size() == 1);
    }

    // pool_alloc_element - allocating an element while there is free slots
    {
        uimax l_element = 4;
        Token(uimax) l_token = l_pool_sizet.alloc_element(l_element);

        l_pool_sizet.alloc_element(cast(uimax, 10));
        l_pool_sizet.release_element(l_pool_sizet.alloc_element(cast(uimax, 10)));
        l_pool_sizet.alloc_element(cast(uimax, 10));

        assert_true(tk_v(l_token) == 0);
        assert_true(l_pool_sizet.get(l_token) == l_element);
    }

    for (pool_loop(&l_pool_sizet, i))
    {
        l_pool_sizet.get(Token(uimax){i});
    }

    {
        l_pool_sizet.free();
        assert_true(l_pool_sizet.get_size() == 0);
        assert_true(l_pool_sizet.get_capacity() == 0);
        assert_true(l_pool_sizet.get_memory() == 0);
    }
};

inline void varyingslice_test()
{
    Slice<uimax> l_varying_slice_memory = SliceN<uimax, 5>{0, 1, 2, 3, 4}.to_slice();
    Slice<SliceIndex> l_varying_slice_indices =
        SliceN<SliceIndex, 5>{SliceIndex::build(0, sizeof(uimax)), SliceIndex::build(sizeof(uimax), sizeof(uimax)), SliceIndex::build(sizeof(uimax) * 2, sizeof(uimax)),
                              SliceIndex::build(sizeof(uimax) * 3, sizeof(uimax)), SliceIndex::build(sizeof(uimax) * 4, sizeof(uimax))}
            .to_slice();

    VaryingSlice l_varying_slice = VaryingSlice::build(l_varying_slice_memory.build_asint8(), l_varying_slice_indices);

    assert_true(l_varying_slice.get_size() == l_varying_slice_indices.Size);
    for (loop(i, 0, l_varying_slice.get_size()))
    {
        assert_true(l_varying_slice.get_element_typed<uimax>(i).get(0) == l_varying_slice_memory.get(i));
    }
};

inline void varyingvector_test()
{
    VaryingVector l_varyingvector = VaryingVector::allocate_default();

    // varyingvector_push_back
    {
        assert_true(l_varyingvector.memory.Size == 0);

        const int8* l_10_element = "abcdefhikl";
        Slice<int8> l_slice = Slice<int8>::build_memory_elementnb((int8*)l_10_element, 10);
        l_varyingvector.push_back(l_slice);

        assert_true(l_varyingvector.get_size() == 1);
        Slice<int8> l_element_0 = l_varyingvector.get_element(0);
        assert_true(l_element_0.Size == 10);
        assert_true(l_slice.compare(l_element_0));
    }

    // push_back_empty
    {
        uimax l_slice_size = 5;
        l_varyingvector.push_back_empty(l_slice_size);
        assert_true(l_varyingvector.get_last_element().Size == l_slice_size);
    }

    // varyingvector_push_back_element
    {
        uimax l_element = 20;
        l_varyingvector.push_back_element(l_element);

        uimax l_inserted_index = l_varyingvector.get_size() - 1;
        Slice<int8> l_element_inserted = l_varyingvector.get_element(l_inserted_index);

        assert_true(l_element_inserted.Size == sizeof(uimax));
        assert_true(memory_compare(cast(const int8*, &l_element), l_element_inserted.Begin, l_element_inserted.Size));

        Slice<uimax> l_casted_slice = slice_cast<uimax>(l_element_inserted);
        assert_true(l_casted_slice.Size == 1);
    }

    // varyingvector_pop_back
    {

        uimax l_old_size = l_varyingvector.get_size();
        l_varyingvector.pop_back();
        assert_true(l_varyingvector.get_size() == (l_old_size - 1));
    }

    l_varyingvector.free();
    l_varyingvector = VaryingVector::allocate_default();

    // varyingvector_erase_element_at
    {
        for (loop(i, 0, 5))
        {
            l_varyingvector.push_back_element(cast(uimax, i));
        }

        assert_true(l_varyingvector.get_size() == 5);
        l_varyingvector.erase_element_at(2);
        assert_true(l_varyingvector.get_size() == 4);

        assert_true(l_varyingvector.get_element_typed<uimax>(2).get(0) == 3);
        assert_true(l_varyingvector.get_element_typed<uimax>(3).get(0) == 4);
    }

    l_varyingvector.free();
    l_varyingvector = VaryingVector::allocate_default();

    // erase_element_at_always
    {
        for (loop(i, 0, 5))
        {
            l_varyingvector.push_back_element(cast(uimax, i));
        }

        assert_true(l_varyingvector.get_size() == 5);
        l_varyingvector.erase_element_at_always(2);
        l_varyingvector.erase_element_at_always(l_varyingvector.get_size() - 1);
        assert_true(l_varyingvector.get_size() == 3);

        assert_true(l_varyingvector.get_element_typed<uimax>(1).get(0) == 1);
        assert_true(l_varyingvector.get_element_typed<uimax>(2).get(0) == 3);
    }

    l_varyingvector.free();
    l_varyingvector = VaryingVector::allocate_default();

    // varyingvector_erase_array_at
    {
        for (loop(i, 0, 5))
        {
            l_varyingvector.push_back_element(cast(uimax, i));
        }

        assert_true(l_varyingvector.get_size() == 5);
        l_varyingvector.erase_array_at(2, 2);
        assert_true(l_varyingvector.get_size() == 3);

        assert_true(l_varyingvector.get_element_typed<uimax>(2).get(0) == 4);
    }

    l_varyingvector.free();
    l_varyingvector = VaryingVector::allocate_default();

    // varyingvector_element_expand_with_value varyingvector_element_shrink
    {
        for (loop(i, 0, 5))
        {
            l_varyingvector.push_back_element(cast(uimax, i));
        }

        uimax l_inserset_number = 30;
        Slice<int8> l_expansion_slice = Slice<uimax>::build_asint8_memory_elementnb(&l_inserset_number, 1);
        l_varyingvector.element_expand_with_value(2, l_expansion_slice);

        Slice<uimax> l_sizet_element_2 = slice_cast<uimax>(l_varyingvector.get_element(2));
        assert_true(l_sizet_element_2.Size == 2);
        assert_true(l_sizet_element_2.get(1) == l_inserset_number);

        {
            uimax* l_sizet_element_3 = slice_cast_singleelement<uimax>(l_varyingvector.get_element(3));
            assert_true(*l_sizet_element_3 == 3);
        }

        l_varyingvector.element_shrink(2, sizeof(uimax));
        l_sizet_element_2 = slice_cast<uimax>(l_varyingvector.get_element(2));
        assert_true(l_sizet_element_2.Size == 1);
        assert_true(l_sizet_element_2.get(0) == 2);

        {
            uimax* l_sizet_element_3 = slice_cast_singleelement<uimax>(l_varyingvector.get_element(3));
            assert_true(*l_sizet_element_3 == 3);
        }
    }

    // varyingvector_element_writeto
    {
        uimax l_element_0 = 10;
        uimax l_element_1 = 20;
        uimax l_element_2 = 30;

        l_varyingvector.element_expand(2, sizeof(uimax) * 3);
        l_varyingvector.element_writeto(2, 0, Slice<uimax>::build_asint8_memory_singleelement(&l_element_0));
        l_varyingvector.element_writeto(2, 2 * sizeof(uimax), Slice<uimax>::build_asint8_memory_singleelement(&l_element_2));
        l_varyingvector.element_writeto(2, 1 * sizeof(uimax), Slice<uimax>::build_asint8_memory_singleelement(&l_element_1));

        Slice<int8> l_varyingvector_element_2 = l_varyingvector.get_element(2);
        assert_true(*cast(uimax*, l_varyingvector_element_2.Begin) == l_element_0);
        assert_true(*cast(uimax*, l_varyingvector_element_2.slide_rv(sizeof(uimax)).Begin) == l_element_1);
        assert_true(*cast(uimax*, l_varyingvector_element_2.slide_rv(2 * sizeof(uimax)).Begin) == l_element_2);
    }

    {
        for (varyingvector_loop(&l_varyingvector, i))
        {
            l_varyingvector.get_element(i);
        }
    }

    {
        l_varyingvector.free();
        assert_true(l_varyingvector.get_size() == 0);
    }
};

inline void vectorofvector_test()
{
    VectorOfVector<uimax> l_vectorofvector_uimax = VectorOfVector<uimax>::allocate_default();

    // vectorofvector_push_back vectorofvector_push_back_element
    {
        Span<uimax> l_sizets = Span<uimax>::allocate(10);
        for (loop(i, 0, l_sizets.Capacity))
        {
            l_sizets.slice.get(i) = i;
        }

        l_vectorofvector_uimax.push_back();

        l_vectorofvector_uimax.push_back_element(l_sizets.slice);
        uimax l_requested_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;
        Slice<uimax> l_element = l_vectorofvector_uimax.get(l_requested_index);

        l_vectorofvector_uimax.push_back();

        assert_true(l_vectorofvector_uimax.get_vectorheader(l_requested_index)->Capacity == l_sizets.Capacity);
        for (loop(i, 0, l_sizets.Capacity))
        {
            assert_true(l_element.get(i) == i);
        }

        l_sizets.free();
    }

    // vectorofvector_element_push_back_element
    {
        uimax l_index;
        for (loop(i, 0, 2))
        {
            l_vectorofvector_uimax.push_back();

            uimax l_element = 30;
            l_index = l_vectorofvector_uimax.varying_vector.get_size() - 2;
            l_vectorofvector_uimax.element_push_back_element(l_index, l_element);
            Slice<uimax> l_element_nested = l_vectorofvector_uimax.get(l_index);
            assert_true(l_element_nested.Size == 1);
            assert_true(l_element_nested.get(0) == l_element);

            l_element = 35;
            l_vectorofvector_uimax.element_clear(l_index);
            l_vectorofvector_uimax.element_push_back_element(l_index, l_element);
            assert_true(l_element_nested.Size == 1);
            assert_true(l_element_nested.get(0) == l_element);
        }
    }

    // vectorofvector_element_insert_element_at
    {
        uimax l_elements[3] = {100, 120, 140};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);
        l_vectorofvector_uimax.push_back_element(l_elements_slice);
        uimax l_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;

        uimax l_inserted_element = 200;
        l_vectorofvector_uimax.element_insert_element_at(l_index, 1, l_inserted_element);

        Slice<uimax> l_vector = l_vectorofvector_uimax.get(l_index);
        assert_true(l_vector.Size == 4);
        assert_true(l_vector.get(0) == l_elements[0]);
        assert_true(l_vector.get(1) == l_inserted_element);
        assert_true(l_vector.get(2) == l_elements[1]);
        assert_true(l_vector.get(3) == l_elements[2]);
    }

    // vectorofvector_element_erase_element_at
    {
        uimax l_elements[3] = {100, 120, 140};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);
        l_vectorofvector_uimax.push_back_element(l_elements_slice);

        // uimax l_inserted_element = 200;
        uimax l_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;
        l_vectorofvector_uimax.element_erase_element_at(l_index, 1);
        Slice<uimax> l_vector = l_vectorofvector_uimax.get(l_index);
        assert_true(l_vector.Size == 2);
        assert_true(l_vector.get(0) == l_elements[0]);
        assert_true(l_vector.get(1) == l_elements[2]);
    }

    // vectorofvector_element_push_back_array
    {
        uimax l_initial_elements[3] = {1, 2, 3};
        {
            Slice<uimax> l_initial_elements_slice = Slice<uimax>::build_memory_elementnb(l_initial_elements, 3);
            l_vectorofvector_uimax.push_back_element(l_initial_elements_slice);
        }

        uimax l_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;

        uimax l_elements[3] = {100, 120, 140};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);

        uimax l_old_size = 0;
        {
            Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
            l_old_size = l_vector_element.Size;
        }

        l_vectorofvector_uimax.element_push_back_array(l_index, l_elements_slice);

        {
            Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
            assert_true(l_vector_element.Size == l_old_size + 3);
            for (loop(i, 0, 3))
            {
                assert_true(l_vector_element.get(i) == l_initial_elements[i]);
            }
            for (loop(i, l_old_size, l_old_size + 3))
            {
                assert_true(l_vector_element.get(i) == l_elements[i - l_old_size]);
            }
        }

        l_vectorofvector_uimax.element_erase_element_at(l_index, 4);
        l_vectorofvector_uimax.element_push_back_array(l_index, l_elements_slice);

        {
            Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
            assert_true(l_vector_element.Size == 8);
            for (loop(i, 0, 3))
            {
                assert_true(l_vector_element.get(i) == l_initial_elements[i]);
            }

            assert_true(l_vector_element.get(3) == l_elements[0]);
            assert_true(l_vector_element.get(4) == l_elements[2]);
            assert_true(l_vector_element.get(5) == l_elements[0]);
            assert_true(l_vector_element.get(6) == l_elements[1]);
            assert_true(l_vector_element.get(7) == l_elements[2]);
        }

        l_vectorofvector_uimax.element_clear(l_index);
        l_vectorofvector_uimax.element_push_back_array(l_index, l_elements_slice);
        {
            Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
            assert_true(l_vector_element.Size == 3);
            assert_true(l_vectorofvector_uimax.get_vectorheader(l_index)->Capacity == 8);
            assert_true(l_vector_element.get(0) == l_elements[0]);
            assert_true(l_vector_element.get(1) == l_elements[1]);
            assert_true(l_vector_element.get(2) == l_elements[2]);
        }
    }

    // insert_empty_at
    {
        l_vectorofvector_uimax.free();
        l_vectorofvector_uimax = VectorOfVector<uimax>::allocate_default();

        l_vectorofvector_uimax.push_back();
        l_vectorofvector_uimax.element_push_back_element(0, 10);

        l_vectorofvector_uimax.insert_empty_at(0);

        assert_true(l_vectorofvector_uimax.varying_vector.get_size() == 2);
        assert_true(l_vectorofvector_uimax.get_vectorheader(0)->Capacity == 0);
    }

    {
        l_vectorofvector_uimax.free();
        l_vectorofvector_uimax = VectorOfVector<uimax>::allocate_default();

        uimax l_initial_elements_0[3] = {1, 2, 3};
        uimax l_initial_elements_1[3] = {4, 5, 6};
        {
            Slice<uimax> l_initial_elements_0_slice = Slice<uimax>::build_memory_elementnb(l_initial_elements_0, 3);
            l_vectorofvector_uimax.push_back_element(l_initial_elements_0_slice);
            Slice<uimax> l_initial_elements_1_slice = Slice<uimax>::build_memory_elementnb(l_initial_elements_1, 3);
            l_vectorofvector_uimax.push_back_element(l_initial_elements_1_slice);
        }

        {
            VectorOfVector<uimax>::Element_ShadowVector l_shadow = l_vectorofvector_uimax.element_as_shadow_vector(0);
            l_shadow.push_back_element(9);
            assert_true(l_shadow.get_size() == 4);
            assert_true(l_shadow.get(3) == 9);
            assert_true(l_shadow.get(2) == l_initial_elements_0[2]);

            assert_true(l_vectorofvector_uimax.get(1).get(0) == l_initial_elements_1[0]);
            assert_true(l_vectorofvector_uimax.get(1).get(1) == l_initial_elements_1[1]);
        }
    }

    l_vectorofvector_uimax.free();
};

inline void poolofvector_test()
{
    PoolOfVector<uimax> l_pool_of_vector = PoolOfVector<uimax>::allocate_default();

    // poolofvector_alloc_vector poolofvector_element_push_back_element poolofvector_release_vector
    {
        PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector();

        uimax l_element = 100;
        l_pool_of_vector.element_push_back_element(l_vector_0, l_element);

        Slice<uimax> l_vector_mem = l_pool_of_vector.get_vector(l_vector_0);
        assert_true(l_vector_mem.Size == 1);
        assert_true(l_vector_mem.get(0) == l_element);

        l_pool_of_vector.release_vector(l_vector_0);

        PoolOfVectorToken<uimax> l_vector_0_new = l_pool_of_vector.alloc_vector();
        assert_true(tk_v(l_vector_0_new) == tk_v(l_vector_0));
        l_vector_mem = l_pool_of_vector.get_vector(l_vector_0);
        assert_true(l_vector_mem.Size == 0);
    }

    // poolofvector_alloc_vector_with_values
    {
        uimax l_elements[3] = {100, 200, 300};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);
        PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector_with_values(l_elements_slice);

        Slice<uimax> l_vector_mem = l_pool_of_vector.get_vector(l_vector_0);
        assert_true(l_vector_mem.Size == 3);
        for (loop(i, 0, 3))
        {
            assert_true(l_vector_mem.get(i) == l_elements[i]);
        }
    }

    {
        PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector();
        PoolOfVector<uimax>::Element_ShadowVector l_shadow = l_pool_of_vector.get_element_as_shadow_vector(l_vector_0);

        assert_true(l_shadow.get_size() == 0);
        l_shadow.push_back_element(10);
        assert_true(l_shadow.get_size() == 1);
        assert_true(l_shadow.get(0) == 10);
    }

    l_pool_of_vector.free();
    l_pool_of_vector = PoolOfVector<uimax>::allocate_default();

    // Element_ShadowVector
    {
        PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector();
        auto l_shadow_vector_0 = l_pool_of_vector.get_element_as_shadow_vector(l_vector_0);

        uimax l_el_0 = 10;
        uimax l_el_1 = 20;

        assert_true(l_shadow_vector_0.get_size() == 0);
        l_shadow_vector_0.push_back_element(l_el_0);
        assert_true(l_shadow_vector_0.get_size() == 1);
        assert_true(l_shadow_vector_0.get(0) == l_el_0);
        l_shadow_vector_0.push_back_element(l_el_1);
        assert_true(l_shadow_vector_0.get_size() == 2);
        assert_true(l_shadow_vector_0.get(1) == l_el_1);

        PoolOfVectorToken<uimax> l_vector_1 = l_pool_of_vector.alloc_vector();
        auto l_shadow_vector_1 = l_pool_of_vector.get_element_as_shadow_vector(l_vector_1);
        l_shadow_vector_1.push_back_element(30);
        l_shadow_vector_1.push_back_element(40);
        l_shadow_vector_1.push_back_element(50);

        assert_true(l_shadow_vector_0.get_size() == 2);
        assert_true(l_shadow_vector_1.get_size() == 3);

        Slice<uimax> l_shadow_vector_0_slice = l_shadow_vector_0.to_slice();
        assert_true(l_shadow_vector_0_slice.Size == 2);
        assert_true(l_shadow_vector_0_slice.get(0) == l_el_0);
        assert_true(l_shadow_vector_0_slice.get(1) == l_el_1);
    }

    l_pool_of_vector.free();
};

inline void pool_hashed_counted_test()
{
    PoolHashedCounted<uimax, uimax> l_pool_hashed_counted = PoolHashedCounted<uimax, uimax>::allocate_default();
    {
        Token(uimax) l_value_token = l_pool_hashed_counted.increment_or_allocate(10, 100);

        assert_true(l_pool_hashed_counted.pool.get(l_value_token) == 100);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));

        l_pool_hashed_counted.increment_or_allocate(10, 100);

        PoolHashedCounted<uimax, uimax>::CountElement* l_count_element = l_pool_hashed_counted.CountMap.get_value_nothashed(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 2);

        l_count_element = l_pool_hashed_counted.decrement(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 1);
    }

    l_pool_hashed_counted.free();
    l_pool_hashed_counted = PoolHashedCounted<uimax, uimax>::allocate_default();

    // increment or allocate stateful
    {
        PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine l_increment_or_allocate_sm = PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine::build(l_pool_hashed_counted);
        l_increment_or_allocate_sm.start(10);
        while (l_increment_or_allocate_sm.state != PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine::State::END)
        {
            if (l_increment_or_allocate_sm.state == PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine::State::ALLOCATE)
            {
                l_increment_or_allocate_sm.allocate(10, 100);
            }
        }
        l_increment_or_allocate_sm.assert_ended();

        Token(uimax) l_value_token = l_increment_or_allocate_sm.allocated_token;

        assert_true(l_pool_hashed_counted.pool.get(l_value_token) == 100);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));

        l_increment_or_allocate_sm = PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine::build(l_pool_hashed_counted);
        l_increment_or_allocate_sm.start(10);
        while (l_increment_or_allocate_sm.state != PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine::State::END)
        {
            if (l_increment_or_allocate_sm.state == PoolHashedCounted<uimax, uimax>::IncrementOrAllocateStateMachine::State::ALLOCATE)
            {
                l_increment_or_allocate_sm.allocate(10, 100);
            }
        }
        l_increment_or_allocate_sm.assert_ended();

        PoolHashedCounted<uimax, uimax>::CountElement* l_count_element = l_pool_hashed_counted.CountMap.get_value_nothashed(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 2);

        l_count_element = l_pool_hashed_counted.decrement(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 1);
    }

    l_pool_hashed_counted.free();
};

inline void ntree_test()
{
    NTree<uimax> l_uimax_tree = NTree<uimax>::allocate_default();

    Token(uimax) l_root = l_uimax_tree.push_root_value(cast(uimax, 0));
    l_uimax_tree.push_value(cast(uimax, 1), l_root);
    Token(uimax) l_2_node = l_uimax_tree.push_value(cast(uimax, 2), l_root);
    Token(uimax) l_3_node = l_uimax_tree.push_value(cast(uimax, 3), l_root);

    l_uimax_tree.push_value(cast(uimax, 4), l_2_node);
    l_uimax_tree.push_value(cast(uimax, 5), l_2_node);

    Token(uimax) l_6_node = l_uimax_tree.push_value(cast(uimax, 6), l_3_node);

    {
        assert_true(l_uimax_tree.Memory.get_size() == 7);
        assert_true(l_uimax_tree.Indices.get_size() == 7);

        // testing the root
        {
            NTree<uimax>::Resolve l_root_element = l_uimax_tree.get(l_root);
            assert_true((*l_root_element.Element) == 0);
            assert_true(tk_v(l_root_element.Node->parent) == (token_t)-1);
            assert_true(tk_v(l_root_element.Node->index) == (token_t)0);
            assert_true(tk_v(l_root_element.Node->childs) != (token_t)-1);

            Slice<Token(NTreeNode)> l_childs_indices = l_uimax_tree.get_childs(l_root_element.Node->childs);
            assert_true(l_childs_indices.Size == 3);
            for (loop(i, 0, l_childs_indices.Size))
            {
                assert_true(l_uimax_tree.get_value(tk_bf(uimax, l_childs_indices.get(i))) == i + 1);
            }
        }

        // testing one leaf
        {
            NTree<uimax>::Resolve l_2_element = l_uimax_tree.get(l_2_node);
            assert_true((*l_2_element.Element) == 2);
            assert_true(tk_v(l_2_element.Node->parent) == (token_t)0);
            assert_true(tk_v(l_2_element.Node->index) == (token_t)2);
            assert_true(tk_v(l_2_element.Node->childs) != (token_t)-1);

            Slice<Token(NTreeNode)> l_childs_indices = l_uimax_tree.get_childs(l_2_element.Node->childs);
            assert_true(l_childs_indices.Size == 2);
            for (loop(i, 0, l_childs_indices.Size))
            {
                assert_true(l_uimax_tree.get_value(tk_bf(uimax, l_childs_indices.get(i))) == i + 4);
            }
        }
    }

    // traversing test
    {
        uimax l_counter = 0;

        l_uimax_tree.traverse3(tk_b(NTreeNode, 0), [&l_counter](const NTree<uimax>::Resolve& p_node) {
            l_counter += 1;
            *(p_node.Element) += 1;
        });

        assert_true(l_counter == 7);

        assert_true(l_uimax_tree.get_value(l_root) == 1);
        assert_true(l_uimax_tree.get_value(l_2_node) == 3);
        assert_true(l_uimax_tree.get_value(l_3_node) == 4);
        assert_true(l_uimax_tree.get_value(l_6_node) == 7);
    }

    // removal test
    {
        l_uimax_tree.remove_node_recursively(tk_bf(NTreeNode, l_2_node));

        NTree<uimax>::Resolve l_root_node = l_uimax_tree.get(l_root);
        Slice<Token(NTreeNode)> l_root_node_childs = l_uimax_tree.get_childs(l_root_node.Node->childs);
        assert_true(l_root_node_childs.Size == 2);

        {
            uimax l_counter = 0;
            l_uimax_tree.traverse3(tk_b(NTreeNode, 0), [&l_counter](const NTree<uimax>::Resolve& p_node) {
                l_counter += 1;
                *p_node.Element += 1;
            });

            assert_true(l_counter == 4);
        }
    }

    // add_child
    {
        l_2_node = l_uimax_tree.push_value(cast(uimax, 2), l_root);
        Token(uimax) l_2_1_node = l_uimax_tree.push_value(cast(uimax, 3), l_2_node);
        Token(uimax) l_2_2_node = l_uimax_tree.push_value(cast(uimax, 3), l_2_node);

        assert_true(l_uimax_tree.add_child(l_3_node, l_2_2_node));

        Slice<Token(NTreeNode)> l_2_node_childs = l_uimax_tree.get_childs_from_node(tk_bf(NTreeNode, l_2_node));
        assert_true(l_2_node_childs.Size == 1);
        assert_true(tk_v(l_2_node_childs.get(0)) == tk_v(l_2_1_node));

        Slice<Token(NTreeNode)> l_3_node_childs = l_uimax_tree.get_childs_from_node(tk_bf(NTreeNode, l_3_node));
        assert_true(l_3_node_childs.Size == 2);
        assert_true(tk_v(l_3_node_childs.get(1)) == tk_v(l_2_2_node));

        assert_true(tk_v(l_uimax_tree.get(l_2_2_node).Node->parent) == tk_v(l_3_node));
    }

    l_uimax_tree.free();
};

inline void assert_heap_integrity(Heap* p_heap)
{
    uimax l_calculated_size = 0;
    for (loop(i, 0, p_heap->AllocatedChunks.get_size()))
    {
        // Token(SliceIndex)* l_chunk = ;
        if (!p_heap->AllocatedChunks.is_element_free(Token(SliceIndex){i}))
        {
            l_calculated_size += p_heap->AllocatedChunks.get(Token(SliceIndex){i}).Size;
        };
    }

    for (loop(i, 0, p_heap->FreeChunks.Size))
    {
        l_calculated_size += p_heap->FreeChunks.get(i).Size;
    }

    assert_true(l_calculated_size == p_heap->Size);
};

inline void asset_heappaged_integrity(HeapPaged* p_heap_paged)
{
    uimax l_calculated_size = 0;
    for (loop(i, 0, p_heap_paged->AllocatedChunks.get_size()))
    {
        if (!p_heap_paged->AllocatedChunks.is_element_free(Token(SliceIndex){i}))
        {
            l_calculated_size += p_heap_paged->AllocatedChunks.get(Token(SliceIndex){i}).Size;
        };
    }

    for (loop(i, 0, p_heap_paged->FreeChunks.varying_vector.get_size()))
    {
        for (loop(j, 0, p_heap_paged->FreeChunks.get(i).Size))
        {
            l_calculated_size += p_heap_paged->FreeChunks.get(i).get(j).Size;
        }
    }

    assert_true(l_calculated_size == (p_heap_paged->PageSize * p_heap_paged->get_page_count()));
};

inline void sort_test(){{uimax l_sizet_array[10] = {10, 9, 8, 2, 7, 4, 10, 35, 9, 4};
uimax l_sorted_sizet_array[10] = {35, 10, 10, 9, 9, 8, 7, 4, 4, 2};
Slice<uimax> l_slice = Slice<uimax>::build_memory_elementnb(l_sizet_array, 10);

struct TestSorter
{
    inline int8 operator()(const uimax& p_left, const uimax& p_right) const
    {
        return p_left < p_right;
    };
};
Sort::Linear3(l_slice, 0, TestSorter{});

assert_true(memcmp(l_sizet_array, l_sorted_sizet_array, sizeof(uimax) * 10) == 0);
}
}
;

inline void heap_test()
{
    uimax l_initial_heap_size = 20;
    Heap l_heap = Heap::allocate(l_initial_heap_size);
    assert_heap_integrity(&l_heap);

    {

        HeapA::AllocatedElementReturn l_chunk_1;
        assert_true((HeapA::AllocationState_t)l_heap.allocate_element(10, &l_chunk_1) & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED);
        assert_true(l_heap.get(l_chunk_1.token)->Begin == 0);
        assert_true(l_heap.get(l_chunk_1.token)->Size == 10);
        assert_heap_integrity(&l_heap);

        HeapA::AllocatedElementReturn l_chunk_0;
        assert_true((HeapA::AllocationState_t)l_heap.allocate_element(5, &l_chunk_0) & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED);
        assert_true(l_heap.get(l_chunk_0.token)->Begin == 10);
        assert_true(l_heap.get(l_chunk_0.token)->Size == 5);
        assert_heap_integrity(&l_heap);

        HeapA::AllocatedElementReturn l_chunk_2;
        l_heap.allocate_element(5, &l_chunk_2);
        assert_heap_integrity(&l_heap);

        // Releasing elements
        l_heap.release_element(l_chunk_0.token);
        l_heap.release_element(l_chunk_2.token);
        assert_heap_integrity(&l_heap);

        // We try to allocate 10 but there is two chunks size 5 free next to each other
        assert_true(l_heap.allocate_element(10, &l_chunk_0) == HeapA::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);

        // The heap is resized
        HeapA::AllocatedElementReturn l_chunk_3;
        assert_true((HeapA::AllocationState_t)l_heap.allocate_element(50, &l_chunk_3) & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED_AND_HEAP_RESIZED);
        assert_true(l_chunk_3.Offset == 20);
        assert_true(l_heap.get(l_chunk_3.token)->Size == 50);
        assert_true(l_heap.Size > l_initial_heap_size);
        assert_heap_integrity(&l_heap);
    }

    l_heap.free();

    l_heap = Heap::allocate(l_initial_heap_size);
    assert_heap_integrity(&l_heap);
    {

        HeapA::AllocatedElementReturn l_allocated_chunk;
        assert_true(l_heap.allocate_element_with_modulo_offset(1, 5, &l_allocated_chunk) == HeapA::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);
        assert_true(l_heap.allocate_element_with_modulo_offset(7, 5, &l_allocated_chunk) == HeapA::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);
        assert_true(l_allocated_chunk.Offset == 5);
        assert_true(l_heap.allocate_element_with_modulo_offset(3, 7, &l_allocated_chunk) == HeapA::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);
        assert_true(l_allocated_chunk.Offset == 14);

        assert_true(l_heap.Size == l_initial_heap_size);
    }

    l_heap.free();

    l_heap = Heap::allocate(l_initial_heap_size);
    assert_heap_integrity(&l_heap);
    {
        HeapA::AllocatedElementReturn l_allocated_chunk;

        assert_true(l_heap.AllocatedChunks.get_size() == 0);
        assert_true(l_heap.allocate_element_norealloc_with_modulo_offset(l_initial_heap_size + 10, 0, &l_allocated_chunk) == HeapA::AllocationState::NOT_ALLOCATED);
        assert_true(l_heap.AllocatedChunks.get_size() == 0);
    }

    l_heap.free();
};

// The HeapPaged uses the same allocation functions as the Heap.
// We just test the fact that new pages are created.
inline void heappaged_test()
{
    uimax l_heappaged_chunk_size = 10;
    HeapPaged l_heap_paged = HeapPaged::allocate_default(l_heappaged_chunk_size);

    {
        assert_true(l_heap_paged.PageSize == l_heappaged_chunk_size);
        assert_true(l_heap_paged.get_page_count() == 0);
        asset_heappaged_integrity(&l_heap_paged);

        HeapPaged::AllocatedElementReturn l_allocated_element;
        assert_true(l_heap_paged.allocate_element_norealloc_with_modulo_offset(6, 6, &l_allocated_element) == HeapPaged::AllocationState::ALLOCATED_AND_PAGE_CREATED);
        assert_true(l_heap_paged.get_page_count() == 1);
        assert_true(l_allocated_element.Offset == 0);
        asset_heappaged_integrity(&l_heap_paged);

        assert_true(l_heap_paged.allocate_element_norealloc_with_modulo_offset(6, 6, &l_allocated_element) == HeapPaged::AllocationState::ALLOCATED_AND_PAGE_CREATED);
        assert_true(l_heap_paged.get_page_count() == 2);
        assert_true(l_allocated_element.Offset == 0);
        asset_heappaged_integrity(&l_heap_paged);

        assert_true(l_heap_paged.allocate_element_norealloc_with_modulo_offset(3, 3, &l_allocated_element) == HeapPaged::AllocationState::ALLOCATED);
        assert_true(l_heap_paged.get_page_count() == 2);
        assert_true(l_allocated_element.Offset == 6);
        asset_heappaged_integrity(&l_heap_paged);
    }

    l_heap_paged.free();
}

inline void heap_memory_test()
{
    uimax l_initial_heap_size = 20 * sizeof(uimax);
    HeapMemory l_heap_memory = HeapMemory::allocate(l_initial_heap_size);

    uimax l_element = 10;
    Token(SliceIndex) l_sigle_sizet_chunk;

    // single allocation
    {
        l_sigle_sizet_chunk = l_heap_memory.allocate_element_typed<uimax>(&l_element);
        uimax* l_st = l_heap_memory.get_typed<uimax>(l_sigle_sizet_chunk);
        assert_true(*l_st == l_element);
    }

    // resize
    {
        l_initial_heap_size = l_heap_memory.Memory.Capacity;
        l_heap_memory.allocate_empty_element(30 * sizeof(uimax));
        assert_true(l_heap_memory.Memory.Capacity != l_initial_heap_size);
        assert_true(l_heap_memory._Heap.Size != l_initial_heap_size);
        uimax* l_st = l_heap_memory.get_typed<uimax>(l_sigle_sizet_chunk);
        assert_true(*l_st == l_element);
    }

    l_heap_memory.free();
};

inline void string_test()
{
    uimax l_initial_string_capacity = 20;
    String l_str = String::allocate(l_initial_string_capacity);

    assert_true(l_str.get(0) == (int8)NULL);
    assert_true(l_str.get_size() == 1);
    assert_true(l_str.get_int8_nb() == 0);

    // append
    {
        l_str.append(slice_int8_build_rawstr("ABC"));
        assert_true(l_str.get_int8_nb() == 3);
        assert_true(l_str.get(0) == 'A');
        assert_true(l_str.get(1) == 'B');
        assert_true(l_str.get(2) == 'C');
    }

    {
        l_str.insert_array_at(slice_int8_build_rawstr("DEA"), 2);
        assert_true(l_str.get_int8_nb() == 6);
        assert_true(l_str.get(2) == 'D');
        assert_true(l_str.get(3) == 'E');
        assert_true(l_str.get(4) == 'A');
    }

    // remove_int8s
    {
        l_str.remove_int8s('A');
        assert_true(l_str.get_int8_nb() == 4);
        assert_true(l_str.get(0) == 'B');
        assert_true(l_str.get(3) == 'C');
    }

    // to_slice
    {
        Slice<int8> l_slice = l_str.to_slice();
        assert_true(l_slice.Size == 4);
        assert_true(l_slice.get(0) == 'B');
        assert_true(l_slice.get(3) == 'C');
    }

    l_str.free();
    l_str = String::allocate(l_initial_string_capacity);
    l_str.append(slice_int8_build_rawstr("Don't Count Your Chickens Before They Hatch."));

    // find
    {
        uimax l_index;
        assert_true(l_str.to_slice().find(slice_int8_build_rawstr("efor"), &l_index) == 1);
        assert_true(l_index == 27);

        // no found
        l_index = 0;
        assert_true(l_str.to_slice().find(slice_int8_build_rawstr("eforc"), &l_index) == 0);
    }

    l_str.free();
};

inline void fromstring_test()
{
    assert_true(FromString::afloat32(slice_int8_build_rawstr("245.689")) == 245.689f);
    assert_true(FromString::afloat32(slice_int8_build_rawstr("-245.689")) == -245.689f);
    assert_true(FromString::afloat32(slice_int8_build_rawstr("245689.0")) == 245689.f);
    assert_true(FromString::afloat32(slice_int8_build_rawstr("-245689.0")) == -245689.f);
};

inline void deserialize_json_test(){{

    const int8* l_json = MULTILINE({
        "local_position" : {"x" : "16.550000", "y" : "16.650000", "z" : "16.750000"},
        "local_position2" : {"x" : "  17.550000", "y" : "17.650000", "z" : "17.750000"},
        "nodes" : [
            {"local_position" : {"x" : "  10.550000", "y" : "10.650000", "z" : "10.750000"}}, {"local_position" : {"x" : "  11.550000", "y" : "11.650000", "z" : "11.750000"}},
            {"local_position" : {"x" : "  12.550000", "y" : "12.650000", "z" : "12.750000"}}
        ]
    });

String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

JSONDeserializer l_v3 = JSONDeserializer::allocate_default();
l_deserialized.next_object("local_position", &l_v3);

l_v3.next_field("x");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.550000f);
l_v3.next_field("y");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.650000f);
l_v3.next_field("z");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.750000f);

l_deserialized.next_object("local_position2", &l_v3);

l_v3.next_field("x");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.550000f);
l_v3.next_field("y");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.650000f);
l_v3.next_field("z");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.750000f);

float32 l_delta = 0.0f;
json_deser_iterate_array_start("nodes", &l_deserialized);
{
    json_deser_iterate_array_object.next_object("local_position", &l_v3);
    l_v3.next_field("x");
    assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 10.550000f + l_delta);
    l_v3.next_field("y");
    assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 10.650000f + l_delta);
    l_v3.next_field("z");
    assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 10.750000f + l_delta);
    l_delta += 1;
}
json_deser_iterate_array_end();

l_v3.free();
l_deserialized.free();
l_json_str.free();
}

// empty array
{
    const int8* l_json = "{"
                         "\"nodes\":[]}";

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

    JSONDeserializer l_array = JSONDeserializer::allocate_default(), l_object = JSONDeserializer::allocate_default();
    l_deserialized.next_array("nodes", &l_array);
    l_array.next_array_object(&l_object);

    l_object.free();
    l_array.free();
    l_deserialized.free();
    l_json_str.free();
}

// missed field
{
    const int8* l_json = "{"
                         "\"local_position\":{"
                         "\"x\":\"16.506252\","
                         "\"y\" : \"16.604988\","
                         "\"z\" : \"16.705424\""
                         "}";

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

    JSONDeserializer l_v3 = JSONDeserializer::allocate_default();
    l_deserialized.next_object("local_position", &l_v3);
    l_v3.next_field("x");
    l_v3.next_field("y");
    l_v3.next_field("zz");
    l_v3.next_field("z");
    assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.705424f);

    l_v3.free();
    l_deserialized.free();
    l_json_str.free();
}

// only fields
{
    const int8* l_json = "{"
                         "\"x\":\"16.506252\","
                         "\"y\" : \"16.604988\","
                         "\"z\" : \"16.705424\""
                         "}";

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);
    l_deserialized.next_field("x");
    assert_true(FromString::afloat32(l_deserialized.get_currentfield().value) == 16.506252f);
    l_deserialized.next_field("y");
    assert_true(FromString::afloat32(l_deserialized.get_currentfield().value) == 16.604988f);
    l_deserialized.next_field("z");
    assert_true(FromString::afloat32(l_deserialized.get_currentfield().value) == 16.705424f);
    l_deserialized.free();
    l_json_str.free();
}

// empty array - then filled array
{
    const int8* l_json =
        "{\"empty_array\":[],\"filled_array\":[{\"x\":\"16.506252\", \"y\" : \"16.604988\", \"z\" : \"16.705424\"}, {\"x\":\"17.506252\", \"y\" : \"17.604988\", \"z\" : \"17.705424\"}]}";

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

    JSONDeserializer l_array = JSONDeserializer::allocate_default();
    JSONDeserializer l_array_object = JSONDeserializer::allocate_default();
    assert_true(l_deserialized.next_array("empty_array", &l_array));
    assert_true(!l_array.next_array_object(&l_array_object));
    assert_true(l_deserialized.next_array("filled_array", &l_array));
    assert_true(l_array.next_array_object(&l_array_object));

    l_array_object.next_field("x");
    assert_true(FromString::afloat32(l_array_object.get_currentfield().value) == 16.506252f);
    l_array_object.next_field("y");
    assert_true(FromString::afloat32(l_array_object.get_currentfield().value) == 16.604988f);
    l_array_object.next_field("z");
    assert_true(FromString::afloat32(l_array_object.get_currentfield().value) == 16.705424f);

    assert_true(l_array.next_array_object(&l_array_object));

    l_array_object.next_field("x");
    assert_true(FromString::afloat32(l_array_object.get_currentfield().value) == 17.506252f);
    l_array_object.next_field("y");
    assert_true(FromString::afloat32(l_array_object.get_currentfield().value) == 17.604988f);
    l_array_object.next_field("z");
    assert_true(FromString::afloat32(l_array_object.get_currentfield().value) == 17.705424f);

    l_deserialized.free();
    l_array.free();
    l_array_object.free();
    l_json_str.free();
}

// field - nested objects (uimax)
{
    const int8* l_json = MULTILINE({"field" : "1248", "obj" : {"f1" : "14", "f2" : "15", "obj2": {"f1": "16", "f2": "17"}});

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

    assert_true(l_deserialized.next_field("field"));
    assert_true(FromString::auimax(l_deserialized.get_currentfield().value) == 1248);

    JSONDeserializer l_obj_deserializer = JSONDeserializer::allocate_default();
    assert_true(l_deserialized.next_object("obj", &l_obj_deserializer));
    {
        assert_true(l_obj_deserializer.next_field("f1"));
        assert_true(FromString::auimax(l_obj_deserializer.get_currentfield().value) == 14);
        assert_true(l_obj_deserializer.next_field("f2"));
        assert_true(FromString::auimax(l_obj_deserializer.get_currentfield().value) == 15);

        JSONDeserializer l_obj2_deserializer = JSONDeserializer::allocate_default();
        assert_true(l_obj_deserializer.next_object("obj2", &l_obj2_deserializer));
        {
            assert_true(l_obj2_deserializer.next_field("f1"));
            assert_true(FromString::auimax(l_obj2_deserializer.get_currentfield().value) == 16);
            assert_true(l_obj2_deserializer.next_field("f2"));
            assert_true(FromString::auimax(l_obj2_deserializer.get_currentfield().value) == 17);
        }
        l_obj2_deserializer.free();
    }
    l_obj_deserializer.free();
    l_deserialized.free();
    l_json_str.free();
}
}
;

inline void serialize_json_test()
{
    const int8* l_json = "{"
                         "\"local_position\":{"
                         "\"x\":  \"16.55000\","
                         "\"y\" : \"16.65000\","
                         "\"z\" : \"16.75000\""
                         "},"
                         "\"local_position2\":{"
                         "\"x\":\"  17.55000\","
                         "\"y\" : \"17.65000\","
                         "\"z\" : \"17.75000\""
                         "},"
                         "\"nodes\" : ["
                         "{"
                         "\"local_position\":{"
                         "\"x\":\"  10.55000\","
                         "\"y\" : \"10.65000\","
                         "\"z\" : \"10.75000\""
                         "}"
                         "},"
                         "{"
                         "\"local_position\":{"
                         "\"x\":\"  11.55000\","
                         "\"y\" : \"11.65000\","
                         "\"z\" : \"11.75000\""
                         "}"
                         "},"
                         "{"
                         "\"local_position\":{"
                         "\"x\":\"  12.55000\","
                         "\"y\" : \"12.65000\","
                         "\"z\" : \"12.75000\""
                         "}"
                         "}"
                         "]"
                         "}";

    int8 l_float_buffer[ToString::float32str_size];
    Slice<int8> l_float_buffer_slice = Slice<int8>::build_asint8_memory_elementnb(l_float_buffer, ToString::float32str_size);

    JSONSerializer l_serializer = JSONSerializer::allocate_default();
    l_serializer.start();

    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(16.55f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(16.65f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(16.75f, l_float_buffer_slice));
    l_serializer.end_object();

    l_serializer.start_object(slice_int8_build_rawstr("local_position2"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(17.55f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(17.65f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(17.75f, l_float_buffer_slice));
    l_serializer.end_object();

    l_serializer.start_array(slice_int8_build_rawstr("nodes"));

    l_serializer.start_object();
    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(10.55f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(10.65f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(10.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.end_object();

    l_serializer.start_object();
    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(11.55f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(11.65f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(11.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.end_object();

    l_serializer.start_object();
    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(12.55f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(12.65f, l_float_buffer_slice));
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(12.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.end_object();

    l_serializer.end_array();

    l_serializer.end();

    JSONUtil::remove_spaces(l_serializer.output);
    String l_compared_json = String::allocate_elements(slice_int8_build_rawstr(l_json));
    JSONUtil::remove_spaces(l_compared_json);

    assert_true(l_compared_json.to_slice().compare(l_serializer.output.to_slice()));

    l_compared_json.free();
    l_serializer.free();
};

inline void serialize_deserialize_binary_test()
{
    Slice<uimax> l_slice = SliceN<uimax, 5>{0, 1, 2, 3, 4}.to_slice();

    Vector<int8> l_binary_data = Vector<int8>::allocate(0);

    BinarySerializer::slice(&l_binary_data, l_slice.build_asint8());

    BinaryDeserializer l_deserializer = BinaryDeserializer::build(l_binary_data.Memory.slice);
    Slice<uimax> l_slice_deserialized = slice_cast<uimax>(l_deserializer.slice());

    assert_true(l_slice.Size == l_slice_deserialized.Size);
    assert_true(l_slice.compare(l_slice_deserialized));

    l_binary_data.free();
};

inline void file_test()
{
    String l_file_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_file_path.append(slice_int8_build_rawstr("file_test.txt"));

    {
        File l_tmp_file = File::create_or_open(l_file_path.to_slice());
        l_tmp_file.erase_with_slicepath();
    }

    File l_file = File::create(l_file_path.to_slice());
    assert_true(l_file.is_valid());

    Slice<int8> l_source_buffer = SliceN<int8, 100>{0, 1, 2, 3, 4, 5}.to_slice();
    Slice<int8> l_buffer = SliceN<int8, 100>{}.to_slice();

    l_file.write_file(l_source_buffer);
    l_file.read_file(&l_buffer);

    assert_true(l_buffer.compare(l_source_buffer));

    l_file.erase_with_slicepath();

    l_file_path.free();
};

inline void database_test()
{
    String l_database_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(slice_int8_build_rawstr("asset.db"));
    {
        File l_tmp_file = File::create_or_open(l_database_path.to_slice());
        l_tmp_file.erase_with_slicepath();
    }

    // create database and table
    {
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());

        SQLiteQuery l_query = SQLiteQuery::allocate(l_connection, slice_int8_build_rawstr(MULTILINE(create table if not exists test(id integer PRIMARY KEY);)));

        SQliteQueryExecution::execute_sync(l_connection, l_query.statement, []() {});

        l_query.free(l_connection);

        l_connection.free();
    }

    {
        File l_tmp_file = File::create_or_open(l_database_path.to_slice());
        l_tmp_file.erase_with_slicepath();
    }

    // create/insert/read
    {
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());

        SQLiteQuery l_table_creation_query =
            SQLiteQuery::allocate(l_connection, slice_int8_build_rawstr(MULTILINE(create table if not exists test(id integer PRIMARY KEY, num integer, txt text, data blob);)));

        SQliteQueryExecution::execute_sync(l_connection, l_table_creation_query.statement, []() {});
        l_table_creation_query.free(l_connection);

        SQLiteQueryLayout l_parameter_layout = SQLiteQueryLayout::build_slice(
            SliceN<SQLiteQueryPrimitiveTypes, 4>{SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::BLOB}.to_slice());
        SQLitePreparedQuery l_insersion_query = SQLitePreparedQuery::allocate(l_connection, slice_int8_build_rawstr(
				MULTILINE(
						insert into test(id, num, txt, data)
						values( ?, ?, ?, ?);
				)
		), l_parameter_layout, SQLiteQueryLayout::build_default());

        SQLiteQueryBinder l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(l_insersion_query, l_connection);
        l_binder.bind_int64(10, l_connection);
        l_binder.bind_int64(20, l_connection);
        l_binder.bind_text(slice_int8_build_rawstr("test"), l_connection);

        uimax l_value = 30;
        l_binder.bind_blob(Slice<uimax>::build_asint8_memory_singleelement(&l_value), l_connection);

        SQliteQueryExecution::execute_sync(l_connection, l_insersion_query.statement, []() {});

        l_insersion_query.free(l_connection);

        SQLiteQueryLayout l_return_layout =
            SQLiteQueryLayout::build_slice(SliceN<SQLiteQueryPrimitiveTypes, 3>{SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::BLOB}.to_slice());
        SQLitePreparedQuery l_select_query = SQLitePreparedQuery::allocate(l_connection, slice_int8_build_rawstr(
				MULTILINE(
						select num, txt, data from test where test.id = ?;
				)
		), l_parameter_layout, l_return_layout);

        l_binder = SQLiteQueryBinder::build_default();
        l_binder.bind_sqlitepreparedquery(l_select_query, l_connection);
        l_binder.bind_int64(10, l_connection);

        int64 l_retrieved_id = 0;
        Span<int8> l_retrieved_str = Span<int8>::build_default();
        Span<int8> l_retrieved_value = Span<int8>::build_default();
        SQliteQueryExecution::execute_sync(l_connection, l_select_query.statement, [&l_select_query, &l_retrieved_id, &l_retrieved_str, &l_retrieved_value]() {
            SQLiteResultSet l_result_set = SQLiteResultSet::build_from_prepared_query(l_select_query);
            l_retrieved_id = l_result_set.get_int64(0);
            l_retrieved_str = l_result_set.get_text(1);
            l_retrieved_value = l_result_set.get_blob(2);
        });

        assert_true(l_retrieved_id == 20);
        assert_true(l_retrieved_str.slice.compare(slice_int8_build_rawstr("test")));
        assert_true(l_retrieved_value.slice.compare(Slice<uimax>::build_asint8_memory_singleelement(&l_value)));

        l_retrieved_str.free();
        l_retrieved_value.free();

        l_select_query.free(l_connection);

        l_connection.free();
    }

    l_database_path.free();
};

inline void native_window()
{
    Token(Window) l_window = WindowAllocator::allocate(300, 300, slice_int8_build_rawstr("TEST"));
    Window& l_allocated_window = WindowAllocator::get_window(l_window);
    assert_true(!l_allocated_window.is_closing);

    {
        uint32 l_native_width, l_native_height;
        WindowNative::get_window_client_dimensions(l_allocated_window.handle, &l_native_width, &l_native_height);
        assert_true(l_native_width == l_allocated_window.client_width);
        assert_true(l_native_height == l_allocated_window.client_height);

        assert_true(l_allocated_window.client_width == 300);
        assert_true(l_allocated_window.client_height == 300);
    }

    Window l_old_window = l_allocated_window;

    WindowNative::simulate_resize_appevent(l_allocated_window.handle, 400, 400);

    assert_true(l_allocated_window.resize_event.ask);
    assert_true(l_allocated_window.resize_event.new_frame_width == 400);
    assert_true(l_allocated_window.resize_event.new_frame_height == 400);

    l_allocated_window.consume_resize_event();

    assert_true(l_allocated_window.resize_event.ask == 0);
    assert_true(l_old_window.client_width != l_allocated_window.client_width);
    assert_true(l_old_window.client_height != l_allocated_window.client_height);

    {
        uint32 l_native_width, l_native_height;
        WindowNative::get_window_client_dimensions(l_allocated_window.handle, &l_native_width, &l_native_height);
        assert_true(l_native_width == l_allocated_window.client_width);
        assert_true(l_native_height == l_allocated_window.client_height);
    }

    assert_true(!l_allocated_window.is_closing);
    WindowNative::simulate_close_appevent(l_allocated_window.handle);
    assert_true(l_allocated_window.is_closing);

    WindowAllocator::get_window(l_window).close();
    assert_true(WindowAllocator::get_window(l_window).is_closing);
    WindowAllocator::free(l_window);
};

int main(int argc, int8** argv)
{
    slice_span_test();
    vector_test();
    hashmap_test();
    pool_test();
    varyingslice_test();
    varyingvector_test();
    vectorofvector_test();
    poolofvector_test();
    pool_hashed_counted_test();
    ntree_test();
    sort_test();
    heap_test();
    heappaged_test();
    heap_memory_test();
    string_test();
    fromstring_test();
    deserialize_json_test();
    serialize_json_test();
    serialize_deserialize_binary_test();
    file_test();
    database_test();
    native_window();

    memleak_ckeck();
};