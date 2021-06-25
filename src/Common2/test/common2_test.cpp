
#define __SOCKET_ENABLED 1
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
        l_span_sizet.slice.copy_memory_at_index_2(1, slice_from_slicen(&l_slice_1), slice_from_slicen(&l_slice_2));

        assert_true(l_span_sizet.slice.slide_rv(1).compare(slice_from_slicen(&l_slice_1)));
        assert_true(l_span_sizet.slice.slide_rv(5).compare(slice_from_slicen(&l_slice_2)));

        l_span_sizet.free();
    }
    {
        SliceN<uimax, 4> l_slice = {15, 26, 78, 10};
        l_span_sizet = Span<uimax>::allocate_slice(slice_from_slicen(&l_slice));
        assert_true(l_span_sizet.Capacity == 4);
        assert_true(l_span_sizet.slice.compare(slice_from_slicen(&l_slice)));

        l_span_sizet.free();
    }
};

inline void slice_functional_algorithm_test(){// find
                                              {Slice<int8> l_char_slice = slice_int8_build_rawstr("Don't Count Your Chickens Before They Hatch.");

uimax l_index;
assert_true(Slice_find(l_char_slice, slice_int8_build_rawstr("efor"), &l_index) == 1);
assert_true(l_index == 27);

// no found
l_index = 0;
assert_true(Slice_find(l_char_slice, slice_int8_build_rawstr("eforc"), &l_index) == 0);
}

// last_index_of
{
    Slice<int8> l_path_slice = slice_int8_build_rawstr("This/is/a/path");
    Slice<int8> l_compared = slice_int8_build_rawstr("/");
    uimax l_index;
    assert_true(Slice_last_index_of(l_path_slice, l_compared, &l_index));
    assert_true(l_index == 9);
    assert_true(l_path_slice.get(l_index) == '/');
    assert_true(!Slice_last_index_of(l_path_slice, slice_int8_build_rawstr("m"), &l_index));
}

// last_index_of_not_endofslice
{
    Slice<int8> l_path_slice = slice_int8_build_rawstr("This/is/a/path/");
    Slice<int8> l_compared = slice_int8_build_rawstr("/");
    uimax l_index;
    assert_true(Slice_last_index_of_not_endofslice(l_path_slice, l_compared, &l_index));
    assert_true(l_index == 9);
    assert_true(l_path_slice.get(l_index) == '/');
}
}
;

inline void base64_test()
{

    Span<int8> l_encoded = encode_base64(slice_int8_build_rawstr("abcdef"));
    assert_true(l_encoded.Capacity == 8);
    assert_true(slice_int8_build_rawstr("YWJjZGVm").compare(l_encoded.slice));
    l_encoded.free();
};

template <class Container> inline void ivector_test_v2(iVector<Container> p_vector)
{
    iVector<Container>::Assert::template element_type<uimax>();

    // vector_push_back_array
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 5);

        p_vector.push_back_array(l_elements_slice);
        assert_true(p_vector.get_size() == l_old_size + 5);
        for (loop(i, l_old_size, p_vector.get_size()))
        {
            assert_true(p_vector.get(i) == l_elements[i - l_old_size]);
        }
    }

    // push_back_array_empty
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        p_vector.push_back_array_empty(5);
        assert_true(p_vector.get_size() == (l_old_size + (uimax)5));
    }

    // vector_push_back_element
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        uimax l_element = 25;
        p_vector.push_back_element(l_element);
        assert_true(p_vector.get_size() == l_old_size + 1);
        assert_true(p_vector.get(p_vector.get_size() - 1) == l_element);
    }

    // vector_insert_array_at
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 5);
        p_vector.insert_array_at(l_elements_slice, 0);
        assert_true(p_vector.get_size() == l_old_size + 5);
        for (loop_int16(i, 0, 5))
        {
            assert_true(p_vector.get(i) == (uimax)i);
        }

        p_vector.insert_array_at(l_elements_slice, 3);
        assert_true(p_vector.get_size() == l_old_size + 10);
        for (loop_int16(i, 0, 3))
        {
            assert_true((p_vector.get(i)) == l_elements[i]);
        }
        // Middle insertion
        for (loop_int16(i, 3, 8))
        {
            assert_true((p_vector.get(i)) == l_elements[i - cast(uimax, 3)]);
        }
        for (loop_int16(i, 8, 10))
        {
            assert_true((p_vector.get(i)) == l_elements[i - cast(uimax, 5)]);
        }
    }

    // vector_insert_element_at
    {
        uimax l_element = 20;
        typename Container::_SizeType l_old_size = p_vector.get_size();

        p_vector.insert_element_at(l_element, 7);
        assert_true(p_vector.get(7) == l_element);
        assert_true(p_vector.get_size() == l_old_size + 1);

        p_vector.insert_element_at(cast(uimax, 20), 9);
    }

    // vector_erase_element_at
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        uimax l_erase_index = 1;
        uimax l_element_after = p_vector.get(l_erase_index + 1);
        p_vector.erase_element_at(1);
        assert_true(p_vector.get_size() == l_old_size - 1);
        assert_true(p_vector.get(1) == l_element_after);
    }

    // vector_erase_array_at
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        uimax l_erase_begin_index = 3;
        const uimax l_erase_nb = 6;
        const uimax l_old_element_check_nb = 3;

        uimax l_old_values[l_old_element_check_nb];
        for (loop(i, l_erase_begin_index + l_erase_nb, (l_erase_begin_index + l_erase_nb) + l_old_element_check_nb))
        {
            l_old_values[i - (l_erase_begin_index + l_erase_nb)] = p_vector.get(i);
        }

        p_vector.erase_array_at(l_erase_begin_index, l_erase_nb);

        assert_true(p_vector.get_size() == l_old_size - l_erase_nb);
        for (loop(i, 0, l_old_element_check_nb))
        {
            assert_true(p_vector.get(l_erase_begin_index + i) == l_old_values[i]);
        }
    }

    // vector_pop_back
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        p_vector.pop_back();
        assert_true(p_vector.get_size() == l_old_size - 1);
    }

    // vector_pop_back_array
    {
        typename Container::_SizeType l_old_size = p_vector.get_size();
        p_vector.pop_back_array(3);
        assert_true(p_vector.get_size() == l_old_size - 3);
    }

    // format
    {
        p_vector.clear();
        p_vector.push_back_element(0);
        p_vector.push_back_element(1);
        p_vector.push_back_element(2);
        p_vector.push_back_element(2);
        p_vector.push_back_element(3);
        assert_true(p_vector.get_size() == 5);
        p_vector.erase_all_elements_that_matches_element_v2((uimax)2, Equality::Default{});
        assert_true(p_vector.get_size() == 3);
        assert_true(p_vector.get(2) == 3);
    }

    {
        p_vector.clear();
        p_vector.push_back_element(0);
        p_vector.push_back_element(1);
        p_vector.push_back_element(2);
        p_vector.push_back_element(2);
        p_vector.push_back_element(3);
        assert_true(p_vector.get_size() == 5);
        SliceN<uimax, 2> l_erased_elements = {0, 2};
        p_vector.erase_all_elements_that_matches_any_of_element_v2(slice_from_slicen(&l_erased_elements), Equality::Default{});
        assert_true(p_vector.get_size() == 2);
        assert_true(p_vector.get(0) == 1);
        assert_true(p_vector.get(1) == 3);
    }
};

inline void vector_test()
{
    Vector<uimax> l_vector_sizet = Vector<uimax>::build_zero_size((uimax*)NULL, 0);
    iVector<Vector<uimax>> l_vector_sizet_sahdow = l_vector_sizet.to_ivector();
    // shadow_vector_test(l_vector_sizet);
    ivector_test_v2(l_vector_sizet_sahdow);
    l_vector_sizet.free();
    // When freeing the vcetor, it's structure is resetted
    {
        assert_true(l_vector_sizet.Size == 0);
        assert_span_unitialized(&l_vector_sizet.Memory);
    }
};

inline void vector_slice_test()
{
    Span<uimax> l_vector_sizet_buffer = Span<uimax>::allocate(100);
    VectorSlice<uimax> l_vector_sizet = VectorSlice<uimax>::build(l_vector_sizet_buffer.slice, 0);
    iVector<VectorSlice<uimax>> l_vector_sizet_shadow = l_vector_sizet.to_ivector();
    ivector_test_v2(l_vector_sizet_shadow);
    l_vector_sizet_buffer.free();
};

inline void hashmap_test()
{
    struct Key
    {
        uimax v1, v2, v3;
    };
    {

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
};

inline void pool_test()
{
    Pool<uimax> l_pool_sizet = Pool<uimax>::allocate(10);

    {
        assert_true(l_pool_sizet.get_memory_raw() != NULL);
        assert_true(l_pool_sizet.get_capacity() == 10);
        assert_true(l_pool_sizet.get_size() == 0);
    }

    // pool_alloc_element - allocate new element
    {
        assert_true(l_pool_sizet.get_free_size() == 0);

        uimax l_element = 3;
        Token<uimax> l_token = l_pool_sizet.alloc_element(l_element);

        assert_true(token_value(l_token) == 0);
        assert_true(l_pool_sizet.get(l_token) == l_element);
    }

    // pool_release_element - release elements
    {
        Token<uimax> l_token = Token<uimax>{0};
        l_pool_sizet.release_element(l_token);

        // memory is not deallocated
        assert_true(l_pool_sizet.get_size() == 1);
    }

    // pool_alloc_element - allocating an element while there is free slots
    {
        uimax l_element = 4;
        Token<uimax> l_token = l_pool_sizet.alloc_element(l_element);

        l_pool_sizet.alloc_element(cast(uimax, 10));
        l_pool_sizet.release_element(l_pool_sizet.alloc_element(cast(uimax, 10)));
        l_pool_sizet.alloc_element(cast(uimax, 10));

        assert_true(token_value(l_token) == 0);
        assert_true(l_pool_sizet.get(l_token) == l_element);
    }

    for (pool_loop(&l_pool_sizet, i))
    {
        l_pool_sizet.get(Token<uimax>{i});
    }

    {
        l_pool_sizet.free();
        assert_true(l_pool_sizet.get_size() == 0);
        assert_true(l_pool_sizet.get_capacity() == 0);
        assert_true(l_pool_sizet.get_memory_raw() == 0);
    }
};

inline void varyingslice_test()
{
    SliceN<uimax, 5> l_varying_slice_memory_arr = SliceN<uimax, 5>{0, 1, 2, 3, 4};
    Slice<uimax> l_varying_slice_memory = slice_from_slicen(&l_varying_slice_memory_arr);
    SliceN<SliceIndex, 5> l_varying_slice_indices_arr = {SliceIndex::build(0, sizeof(uimax)), SliceIndex::build(sizeof(uimax), sizeof(uimax)), SliceIndex::build(sizeof(uimax) * 2, sizeof(uimax)),
                                                         SliceIndex::build(sizeof(uimax) * 3, sizeof(uimax)), SliceIndex::build(sizeof(uimax) * 4, sizeof(uimax))};
    const Slice<SliceIndex> l_varying_slice_indices = slice_from_slicen(&l_varying_slice_indices_arr);

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

        l_vectorofvector_uimax.push_back_element_empty();

        l_vectorofvector_uimax.push_back_element(l_sizets.slice);
        uimax l_requested_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;
        Slice<uimax> l_element = l_vectorofvector_uimax.get(l_requested_index);

        l_vectorofvector_uimax.push_back_element_empty();

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
            l_vectorofvector_uimax.push_back_element_empty();

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

        l_vectorofvector_uimax.push_back_element_empty();
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
            VectorOfVector<uimax>::Element_iVector l_shadow = l_vectorofvector_uimax.element_as_iVector(0);
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
        assert_true(token_value(l_vector_0_new) == token_value(l_vector_0));
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
        PoolOfVector<uimax>::Element_iVector l_shadow = l_pool_of_vector.get_element_as_iVector(l_vector_0);

        assert_true(l_shadow.get_size() == 0);
        l_shadow.push_back_element(10);
        assert_true(l_shadow.get_size() == 1);
        assert_true(l_shadow.get(0) == 10);
    }

    l_pool_of_vector.free();

    // Element_ShadowVector
    {
        l_pool_of_vector = PoolOfVector<uimax>::allocate_default();
        PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector();
        PoolOfVector<uimax>::Element_iVector l_element_ivector_0 = l_pool_of_vector.get_element_as_iVector(l_vector_0);

        uimax l_el_0 = 10;
        uimax l_el_1 = 20;

        assert_true(l_element_ivector_0.get_size() == 0);
        l_element_ivector_0.push_back_element(l_el_0);
        assert_true(l_element_ivector_0.get_size() == 1);
        assert_true(l_element_ivector_0.get(0) == l_el_0);
        l_element_ivector_0.push_back_element(l_el_1);
        assert_true(l_element_ivector_0.get_size() == 2);
        assert_true(l_element_ivector_0.get(1) == l_el_1);

        PoolOfVectorToken<uimax> l_vector_1 = l_pool_of_vector.alloc_vector();
        PoolOfVector<uimax>::Element_iVector l_element_ivector_1 = l_pool_of_vector.get_element_as_iVector(l_vector_1);
        l_element_ivector_1.push_back_element(30);
        l_element_ivector_1.push_back_element(40);
        l_element_ivector_1.push_back_element(50);

        assert_true(l_element_ivector_0.get_size() == 2);
        assert_true(l_element_ivector_1.get_size() == 3);

        Slice<uimax> l_ivector_0_slice = l_element_ivector_0.to_slice();
        assert_true(l_ivector_0_slice.Size == 2);
        assert_true(l_ivector_0_slice.get(0) == l_el_0);
        assert_true(l_ivector_0_slice.get(1) == l_el_1);

        l_pool_of_vector.free();
    }

    // The PoolOfVector be "linkable" to a Pool. This means that if we call allocation and release of both of the structure at the same time, then we will have the same pool layout
    {
        l_pool_of_vector = PoolOfVector<uimax>::allocate_default();
        Pool<uimax> l_pool = Pool<uimax>::allocate(0);

        Token<Slice<uimax>> l_vec_0 = l_pool_of_vector.alloc_vector();
        Token<uimax> l_val_0 = l_pool.alloc_element(0);

        Token<Slice<uimax>> l_vec_1 = l_pool_of_vector.alloc_vector();
        Token<uimax> l_val_1 = l_pool.alloc_element(0);

        Token<Slice<uimax>> l_vec_2 = l_pool_of_vector.alloc_vector();
        Token<uimax> l_val_2 = l_pool.alloc_element(0);

        assert_true(token_equals(l_val_0, l_vec_0));
        assert_true(token_equals(l_val_1, l_vec_1));
        assert_true(token_equals(l_val_2, l_vec_2));

        l_pool_of_vector.release_vector(l_vec_1);
        l_pool.release_element(l_val_1);

        Token<Slice<uimax>> l_vec_3 = l_pool_of_vector.alloc_vector();
        Token<uimax> l_val_3 = l_pool.alloc_element(0);

        assert_true(token_equals(l_val_3, l_vec_3));
        assert_true(token_equals(l_val_3, l_val_1));
        assert_true(token_equals(l_vec_3, l_vec_1));

        l_pool.free();
        l_pool_of_vector.free();
    }
};

inline void pool_hashed_counted_test()
{
    PoolHashedCounted<uimax, uimax> l_pool_hashed_counted = PoolHashedCounted<uimax, uimax>::allocate_default();
    {
        Token<uimax> l_value_token = l_pool_hashed_counted.increment_or_allocate_v2(10, []() {
            return 100;
        });

        assert_true(l_pool_hashed_counted.pool.get(l_value_token) == 100);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));

        l_pool_hashed_counted.increment_or_allocate_v2(10, []() {
            return 100;
        });

        PoolHashedCounted<uimax, uimax>::CountElement* l_count_element = l_pool_hashed_counted.CountMap.get_value_nothashed(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 2);

        l_count_element = l_pool_hashed_counted.decrement_nothashed(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 1);
    }

    l_pool_hashed_counted.free();
    l_pool_hashed_counted = PoolHashedCounted<uimax, uimax>::allocate_default();

    // increment or allocate stateful
    {
        Token<uimax> l_value_token = l_pool_hashed_counted.increment_or_allocate_v2(10, []() {
            return 100;
        });
        assert_true(l_pool_hashed_counted.pool.get(l_value_token) == 100);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));

        l_value_token = l_pool_hashed_counted.increment_or_allocate_v2(10, []() {
            return 100;
        });

        PoolHashedCounted<uimax, uimax>::CountElement* l_count_element = l_pool_hashed_counted.CountMap.get_value_nothashed(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 2);

        l_count_element = l_pool_hashed_counted.decrement_nothashed(10);
        assert_true(l_pool_hashed_counted.CountMap.has_key_nothashed(10));
        assert_true(l_count_element->counter == 1);
    }

    l_pool_hashed_counted.free();
};

inline void ntree_test()
{
    NTree<uimax> l_uimax_tree = NTree<uimax>::allocate_default();

    Token<NTree<uimax>::Node> l_root = l_uimax_tree.push_root_value(cast(uimax, 0));
    l_uimax_tree.push_value(cast(uimax, 1), l_root);
    Token<NTree<uimax>::Node> l_2_node = l_uimax_tree.push_value(cast(uimax, 2), l_root);
    Token<NTree<uimax>::Node> l_3_node = l_uimax_tree.push_value(cast(uimax, 3), l_root);

    l_uimax_tree.push_value(cast(uimax, 4), l_2_node);
    l_uimax_tree.push_value(cast(uimax, 5), l_2_node);

    Token<NTree<uimax>::Node> l_6_node = l_uimax_tree.push_value(cast(uimax, 6), l_3_node);

    {
        assert_true(l_uimax_tree.Memory.get_size() == 7);
        assert_true(l_uimax_tree.Indices.get_size() == 7);

        // testing the root
        {
            NTree<uimax>::Resolve l_root_element = l_uimax_tree.get(l_root);
            assert_true((*l_root_element.Element) == 0);
            assert_true(token_value(l_root_element.Node->parent) == (token_t)-1);
            assert_true(token_value(l_root_element.Node->index) == (token_t)0);
            assert_true(token_value(l_root_element.Node->childs) != (token_t)-1);

            Slice<Token<NTree<uimax>::Node>> l_childs_indices = l_uimax_tree.get_childs(l_root_element.Node->childs);
            assert_true(l_childs_indices.Size == 3);
            for (loop(i, 0, l_childs_indices.Size))
            {
                assert_true(l_uimax_tree.get_value(l_childs_indices.get(i)) == i + 1);
            }
        }

        // testing one leaf
        {
            NTree<uimax>::Resolve l_2_element = l_uimax_tree.get(l_2_node);
            assert_true((*l_2_element.Element) == 2);
            assert_true(token_value(l_2_element.Node->parent) == (token_t)0);
            assert_true(token_value(l_2_element.Node->index) == (token_t)2);
            assert_true(token_value(l_2_element.Node->childs) != (token_t)-1);

            Slice<Token<NTree<uimax>::Node>> l_childs_indices = l_uimax_tree.get_childs(l_2_element.Node->childs);
            assert_true(l_childs_indices.Size == 2);
            for (loop(i, 0, l_childs_indices.Size))
            {
                assert_true(l_uimax_tree.get_value(l_childs_indices.get(i)) == i + 4);
            }
        }
    }

    // traversing test
    {
        uimax l_counter = 0;

        l_uimax_tree.traverse3(token_build<NTree<uimax>::Node>(0), [&l_counter](const NTree<uimax>::Resolve& p_node) {
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
        l_uimax_tree.remove_node_recursively(l_2_node);

        NTree<uimax>::Resolve l_root_node = l_uimax_tree.get(l_root);
        Slice<Token<NTree<uimax>::Node>> l_root_node_childs = l_uimax_tree.get_childs(l_root_node.Node->childs);
        assert_true(l_root_node_childs.Size == 2);

        {
            uimax l_counter = 0;
            l_uimax_tree.traverse3(token_build<NTree<uimax>::Node>(0), [&l_counter](const NTree<uimax>::Resolve& p_node) {
                l_counter += 1;
                *p_node.Element += 1;
            });

            assert_true(l_counter == 4);
        }
    }

    // add_child
    {
        l_2_node = l_uimax_tree.push_value(cast(uimax, 2), l_root);
        Token<NTree<uimax>::Node> l_2_1_node = l_uimax_tree.push_value(cast(uimax, 3), l_2_node);
        Token<NTree<uimax>::Node> l_2_2_node = l_uimax_tree.push_value(cast(uimax, 3), l_2_node);

        assert_true(l_uimax_tree.add_child_silent(l_3_node, l_2_2_node));

        Slice<Token<NTree<uimax>::Node>> l_2_node_childs = l_uimax_tree.get_childs_from_node(l_2_node);
        assert_true(l_2_node_childs.Size == 1);
        assert_true(token_value(l_2_node_childs.get(0)) == token_value(l_2_1_node));

        Slice<Token<NTree<uimax>::Node>> l_3_node_childs = l_uimax_tree.get_childs_from_node(l_3_node);
        assert_true(l_3_node_childs.Size == 2);
        assert_true(token_value(l_3_node_childs.get(1)) == token_value(l_2_2_node));

        assert_true(token_value(l_uimax_tree.get(l_2_2_node).Node->parent) == token_value(l_3_node));
    }

    l_uimax_tree.free();
};

inline void nntree_test()
{
    NNTree<uimax> l_tree = NNTree<uimax>::allocate_default();

    // Testing child add
    // Multiple parent childs
    // Removing a node recursively
    {
        Token<NNTree<uimax>::Node> l_0 = l_tree.push_root_value(1);

        assert_true(token_value(l_0) == 0);
        assert_true(*l_tree.get(l_0).Element == 1);
        assert_true(l_tree.get_childs(l_tree.get(l_0)).Size == 0);
        assert_true(l_tree.get_parents(l_tree.get(l_0)).Size == 0);

        // Adding child nodes to the root node
        SliceN<Token<NNTree<uimax>::Node>, 1> l_parents = {l_0};
        Token<NNTree<uimax>::Node> l_0_1 = l_tree.push_value(2, slice_from_slicen(&l_parents));
        Token<NNTree<uimax>::Node> l_0_2 = l_tree.push_value(3, slice_from_slicen(&l_parents));

        assert_true(l_tree.get_childs(l_tree.get(l_0)).Size == 2);
        assert_true(l_tree.get_parents(l_tree.get(l_0_1)).Size == 1);
        assert_true(*l_tree.get(l_0_1).Element == 2);
        assert_true(l_tree.get_parents(l_tree.get(l_0_2)).Size == 1);
        assert_true(*l_tree.get(l_0_2).Element == 3);

        // We add a node whose parents are all already created nodes
        SliceN<Token<NNTree<uimax>::Node>, 3> l_parents_2 = {l_0_1, l_0_2, l_0};
        Token<NNTree<uimax>::Node> l_0_0_1 = l_tree.push_value(4, slice_from_slicen(&l_parents_2));

        assert_true(l_tree.get_childs(l_tree.get(l_0)).Size == 3);
        assert_true(l_tree.get_childs(l_tree.get(l_0_1)).Size == 1);
        assert_true(l_tree.get_childs(l_tree.get(l_0_2)).Size == 1);

        assert_true(l_tree.get_parents(l_tree.get(l_0_0_1)).Size == 3);
        assert_true(*l_tree.get(l_0_0_1).Element == 4);

        SliceN<Token<NNTree<uimax>::Node>, 1> l_parent_3 = {l_0_1};
        Token<NNTree<uimax>::Node> l_0_0_2 = l_tree.push_value(5, slice_from_slicen(&l_parent_3));
        l_tree.remove_node_recursively(l_0_1);

        assert_true(l_tree.is_node_free(l_0_0_2));
        assert_true(l_tree.is_node_free(l_0_1));
        assert_true(l_tree.get_parents(l_tree.get(l_0_0_1)).Size == 2);
        assert_true(l_tree.get_childs(l_tree.get(l_0)).Size == 2);
    }

    l_tree.free();
    l_tree = NNTree<uimax>::allocate_default();
    {
        Token<NNTree<uimax>::Node> l_0 = l_tree.push_root_value(1);
        SliceN<Token<NNTree<uimax>::Node>, 1> l_first_wave_parent = {l_0};
        Token<NNTree<uimax>::Node> l_1 = l_tree.push_value(2, slice_from_slicen(&l_first_wave_parent));
        Token<NNTree<uimax>::Node> l_2 = l_tree.push_value(3, slice_from_slicen(&l_first_wave_parent));
        Token<NNTree<uimax>::Node> l_3 = l_tree.push_value(4, slice_from_slicen(&l_first_wave_parent));
        SliceN<Token<NNTree<uimax>::Node>, 3> l_second_wave_parent = {l_1, l_2, l_3};
        Token<NNTree<uimax>::Node> l_4 = l_tree.push_value(5, slice_from_slicen(&l_second_wave_parent));
        Token<NNTree<uimax>::Node> l_5 = l_tree.push_value(6, slice_from_slicen(&l_second_wave_parent));
        SliceN<Token<NNTree<uimax>::Node>, 3> l_third_wave_parent = {l_4, l_5};
        Token<NNTree<uimax>::Node> l_6 = l_tree.push_value(7, slice_from_slicen(&l_third_wave_parent));

        uimax l_sum = 0;
        l_tree.traverse_to_bottom_distinct(l_tree.get(l_0), [&](const NNTree<uimax>::Resolve& p_node) {
            l_sum += *p_node.Element;
        });
        assert_true(l_sum == (1 + 2 + 3 + 4 + 5 + 6 + 7));
    }
    l_tree.free();
    l_tree = NNTree<uimax>::allocate_default();

    // remove_node_recursively on a chain of nodes
    {
        Token<NNTree<uimax>::Node> l_0 = l_tree.push_root_value(1);
        SliceN<Token<NNTree<uimax>::Node>, 1> l_first_wave_parent = {l_0};
        Token<NNTree<uimax>::Node> l_1 = l_tree.push_value(2, slice_from_slicen(&l_first_wave_parent));
        SliceN<Token<NNTree<uimax>::Node>, 1> l_second_wave_parent = {l_1};
        Token<NNTree<uimax>::Node> l_2 = l_tree.push_value(3, slice_from_slicen(&l_second_wave_parent));

        l_tree.remove_node_recursively(l_0);
        assert_true(l_tree.is_node_free(l_0));
        assert_true(l_tree.is_node_free(l_1));
        assert_true(l_tree.is_node_free(l_2));
    }
    l_tree.free();
};

inline void assert_heap_integrity(Heap* p_heap)
{
    uimax l_calculated_size = 0;
    for (loop(i, 0, p_heap->AllocatedChunks.get_size()))
    {
        // Token<SliceIndex>* l_chunk = ;
        if (!p_heap->AllocatedChunks.is_element_free(Token<SliceIndex>{i}))
        {
            l_calculated_size += p_heap->AllocatedChunks.get(Token<SliceIndex>{i}).Size;
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
        if (!p_heap_paged->AllocatedChunks.is_element_free(Token<SliceIndex>{i}))
        {
            l_calculated_size += p_heap_paged->AllocatedChunks.get(Token<SliceIndex>{i}).Size;
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

inline void assert_heap_memory_alignement(const uimax p_alignment, const iHeapTypes::AllocatedElementReturn& p_chunk)
{
    assert_true((p_chunk.Offset % p_alignment) == 0);
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

        iHeapTypes::AllocatedElementReturn l_chunk_1;
        assert_true((iHeapTypes::AllocationState_t)l_heap.allocate_element(10, &l_chunk_1) & (iHeapTypes::AllocationState_t)iHeapTypes::AllocationState::ALLOCATED);
        assert_true(l_heap.get(l_chunk_1.token)->Begin == 0);
        assert_true(l_heap.get(l_chunk_1.token)->Size == 10);
        assert_heap_integrity(&l_heap);

        iHeapTypes::AllocatedElementReturn l_chunk_0;
        assert_true((iHeapTypes::AllocationState_t)l_heap.allocate_element(5, &l_chunk_0) & (iHeapTypes::AllocationState_t)iHeapTypes::AllocationState::ALLOCATED);
        assert_true(l_heap.get(l_chunk_0.token)->Begin == 10);
        assert_true(l_heap.get(l_chunk_0.token)->Size == 5);
        assert_heap_integrity(&l_heap);

        iHeapTypes::AllocatedElementReturn l_chunk_2;
        l_heap.allocate_element(5, &l_chunk_2);
        assert_heap_integrity(&l_heap);

        // Releasing elements
        l_heap.release_element(l_chunk_0.token);
        l_heap.release_element(l_chunk_2.token);
        assert_heap_integrity(&l_heap);

        // We try to allocate 10 but there is two chunks size 5 free next to each other
        assert_true(l_heap.allocate_element(10, &l_chunk_0) == iHeapTypes::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);

        // The heap is resized
        iHeapTypes::AllocatedElementReturn l_chunk_3;
        assert_true((iHeapTypes::AllocationState_t)l_heap.allocate_element(50, &l_chunk_3) & (iHeapTypes::AllocationState_t)iHeapTypes::AllocationState::ALLOCATED_AND_HEAP_RESIZED);
        assert_true(l_chunk_3.Offset == 20);
        assert_true(l_heap.get(l_chunk_3.token)->Size == 50);
        assert_true(l_heap.Size > l_initial_heap_size);
        assert_heap_integrity(&l_heap);
    }

    l_heap.free();

    l_initial_heap_size = 30;
    l_heap = Heap::allocate(l_initial_heap_size);
    assert_heap_integrity(&l_heap);
    {

        iHeapTypes::AllocatedElementReturn l_allocated_chunk;
        assert_true(l_heap.allocate_element_with_modulo_offset(1, 5, &l_allocated_chunk) == iHeapTypes::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);
        assert_heap_memory_alignement(5, l_allocated_chunk);
        assert_true(l_heap.allocate_element_with_modulo_offset(7, 5, &l_allocated_chunk) == iHeapTypes::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);
        assert_true(l_allocated_chunk.Offset == 5);
        assert_heap_memory_alignement(5, l_allocated_chunk);
        assert_true(l_heap.allocate_element_with_modulo_offset(3, 7, &l_allocated_chunk) == iHeapTypes::AllocationState::ALLOCATED);
        assert_heap_integrity(&l_heap);
        assert_true(l_allocated_chunk.Offset == 14);
        assert_heap_memory_alignement(7, l_allocated_chunk);

        // There was a bug that were causing the heap to align memory chunk with the size of the chunk instead of the desired alignment
        l_heap.release_element(l_allocated_chunk.token);
        assert_heap_integrity(&l_heap);
        assert_true(l_heap.allocate_element_with_modulo_offset(3, 7, &l_allocated_chunk) == iHeapTypes::AllocationState::ALLOCATED);
        assert_true(l_allocated_chunk.Offset == 14);
        assert_heap_memory_alignement(7, l_allocated_chunk);

        assert_true(l_heap.Size == l_initial_heap_size);
    }

    l_heap.free();

    l_heap = Heap::allocate(l_initial_heap_size);
    assert_heap_integrity(&l_heap);
    {
        iHeapTypes::AllocatedElementReturn l_allocated_chunk;

        assert_true(l_heap.AllocatedChunks.get_size() == 0);
        assert_true(l_heap.allocate_element_norealloc_with_modulo_offset(l_initial_heap_size + 10, 0, &l_allocated_chunk) == iHeapTypes::AllocationState::NOT_ALLOCATED);
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
    Token<SliceIndex> l_sigle_sizet_chunk;

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

template <class StringContainer>
inline void istring_test(iString<StringContainer> p_string){

    // append
    {p_string.append(slice_int8_build_rawstr("ABC"));
assert_true(p_string.get_length() == 3);
assert_true(p_string.get(0) == 'A');
assert_true(p_string.get(1) == 'B');
assert_true(p_string.get(2) == 'C');
}

{
    p_string.insert_array_at(slice_int8_build_rawstr("DEA"), 2);
    assert_true(p_string.get_length() == 6);
    assert_true(p_string.get(2) == 'D');
    assert_true(p_string.get(3) == 'E');
    assert_true(p_string.get(4) == 'A');
}

// to_slice
{
    Slice<int8> l_slice = p_string.to_slice();
    assert_true(l_slice.Size == 6);
    assert_true(l_slice.get(1) == 'B');
    assert_true(l_slice.get(5) == 'C');
}
}
;

inline void string_test()
{
    uimax l_initial_string_capacity = 20;
    String l_str = String::allocate(l_initial_string_capacity);

    assert_true(l_str.get(0) == (int8)NULL);
    assert_true(l_str.get_size() == 1);
    assert_true(l_str.get_length() == 0);

    istring_test(l_str.to_istring());

    l_str.free();
};

inline void path_test()
{
    String l_path_str = String::allocate_elements(slice_int8_build_rawstr("E:/path/to/file.json"));
    assert_true(Path::move_up(&l_path_str));
    assert_true(slice_int8_build_rawstr("E:/path/to/").compare(l_path_str.to_slice()));
    assert_true(Path::move_up(&l_path_str));
    assert_true(slice_int8_build_rawstr("E:/path/").compare(l_path_str.to_slice()));
    assert_true(Path::move_up(&l_path_str));
    assert_true(slice_int8_build_rawstr("E:/").compare(l_path_str.to_slice()));
    assert_true(!Path::move_up(&l_path_str));

    l_path_str.free();
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
        "local_position" : {"x" : 16.550000, "y" : "16.650000", "z" : "16.750000"},
        "local_position2" : {"x" : "  17.550000", "y" : 17.650000, "z" : "17.750000"},
        "nodes" : [
            {"local_position" : {"x" : "  10.550000", "y" : "10.650000", "z" : "10.750000"}}, {"local_position" : {"x" : "  11.550000", "y" : "11.650000", "z" : "11.750000"}},
            {"local_position" : {"x" : "  12.550000", "y" : "12.650000", "z" : "12.750000"}}
        ]
    });

String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

JSONDeserializer l_v3 = JSONDeserializer::allocate_default();
l_deserialized.next_object("local_position", &l_v3);

l_v3.next_number("x");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.550000f);
l_v3.next_field("y");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.650000f);
l_v3.next_field("z");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.750000f);

l_deserialized.next_object("local_position2", &l_v3);

l_v3.next_field("x");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.550000f);
l_v3.next_number("y");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.650000f);
l_v3.next_field("z");
assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.750000f);

float32 l_delta = 0.0f;
JSONDeserializer l_array = JSONDeserializer::allocate_default();
JSONDeserializer l_object = JSONDeserializer::allocate_default();
l_deserialized.next_array("nodes", &l_array);
while (l_array.next_array_object(&l_object))
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
l_array.free();
l_object.free();

l_v3.free();
l_deserialized.free();
l_json_str.free();
}

// empty array
{
    const int8* l_json = "{"
                         "\"nodes\":[]}";

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

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
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

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
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);
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
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

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

// array with values
{
    const int8* l_json = MULTILINE({"value_array" : [ "17.001", "18.001", "19.001" ]});
    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

    JSONDeserializer l_array = JSONDeserializer::allocate_default();
    Slice<int8> l_array_plain_value;
    assert_true(l_deserialized.next_array("value_array", &l_array));
    assert_true(l_array.next_array_string_value(&l_array_plain_value));
    assert_true(FromString::afloat32(l_array_plain_value) == 17.001f);
    assert_true(l_array.next_array_string_value(&l_array_plain_value));
    assert_true(FromString::afloat32(l_array_plain_value) == 18.001f);
    assert_true(l_array.next_array_string_value(&l_array_plain_value));
    assert_true(FromString::afloat32(l_array_plain_value) == 19.001f);

    l_deserialized.free();
    l_array.free();
    l_json_str.free();
}
// array with numbers
{
    const int8* l_json = MULTILINE({"value_array" : [ 17.001, 18.001, 19.001 ]});
    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

    JSONDeserializer l_array = JSONDeserializer::allocate_default();
    Slice<int8> l_array_plain_value;
    assert_true(l_deserialized.next_array("value_array", &l_array));
    assert_true(l_array.next_array_number_value(&l_array_plain_value));
    assert_true(FromString::afloat32(l_array_plain_value) == 17.001f);
    assert_true(l_array.next_array_number_value(&l_array_plain_value));
    assert_true(FromString::afloat32(l_array_plain_value) == 18.001f);
    assert_true(l_array.next_array_number_value(&l_array_plain_value));
    assert_true(FromString::afloat32(l_array_plain_value) == 19.001f);

    l_deserialized.free();
    l_array.free();
    l_json_str.free();
}

// field - nested objects (uimax)
{
    const int8* l_json = MULTILINE({"field" : "1248", "obj" : {"f1" : "14", "f2" : "15", "obj2" : {"f1" : "16", "f2" : "17"}}});

    String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
    iVector<Vector<int8>> l_json_str_ivector = l_json_str.Memory.to_ivector();
    JSONDeserializer l_deserialized = JSONDeserializer::sanitize_and_start(l_json_str_ivector);

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

    String l_serializer_buffer = String::allocate(0);
    JSONSerializer<String> l_serializer = JSONSerializer<String>::build(l_serializer_buffer.to_istring());
    l_serializer.start();

    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(16.55f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(16.65f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(16.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.coma();

    l_serializer.start_object(slice_int8_build_rawstr("local_position2"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(17.55f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(17.65f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(17.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.coma();

    l_serializer.start_array(slice_int8_build_rawstr("nodes"));

    l_serializer.start_object();
    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(10.55f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(10.65f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(10.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.end_object();
    l_serializer.coma();

    l_serializer.start_object();
    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(11.55f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(11.65f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(11.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.end_object();
    l_serializer.coma();

    l_serializer.start_object();
    l_serializer.start_object(slice_int8_build_rawstr("local_position"));
    l_serializer.push_field(slice_int8_build_rawstr("x"), ToString::afloat32(12.55f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("y"), ToString::afloat32(12.65f, l_float_buffer_slice));
    l_serializer.coma();
    l_serializer.push_field(slice_int8_build_rawstr("z"), ToString::afloat32(12.75f, l_float_buffer_slice));
    l_serializer.end_object();
    l_serializer.end_object();

    l_serializer.end_array();

    l_serializer.end();

    // JSONUtil::remove_spaces(l_serializer.output.Memory);
    String l_compared_json = String::allocate_elements(slice_int8_build_rawstr(l_json));
    iVector<Vector<int8>> l_compared_json_ivector = l_compared_json.Memory.to_ivector();
    JSONUtil::sanitize_json(l_compared_json_ivector);

    assert_true(l_compared_json.to_slice().compare(l_serializer.output.to_slice()));

    l_compared_json.free();
    l_serializer_buffer.free();
};

inline void serialize_deserialize_binary_test()
{
    SliceN<uimax, 5> l_slice_arr = {0, 1, 2, 3, 4};
    Slice<uimax> l_slice = slice_from_slicen(&l_slice_arr);

    Vector<int8> l_binary_data = Vector<int8>::allocate(0);
    BinarySerializer::slice(l_binary_data.to_ivector(), l_slice.build_asint8());

    {
        BinaryDeserializer l_deserializer = BinaryDeserializer::build(l_binary_data.Memory.slice);
        Slice<uimax> l_slice_deserialized = slice_cast<uimax>(l_deserializer.slice());

        assert_true(l_slice.Size == l_slice_deserialized.Size);
        assert_true(l_slice.compare(l_slice_deserialized));
    }

    l_binary_data.free();

    SliceN<uimax, 10> l_buffer;
    VectorSlice<int8> l_binary_data_slice = VectorSlice<int8>::build(slice_from_slicen(&l_buffer).build_asint8(), 0);
    BinarySerializer::slice(l_binary_data_slice.to_ivector(), slice_from_slicen(&l_slice_arr).build_asint8());

    {
        BinaryDeserializer l_deserializer = BinaryDeserializer::build(l_binary_data_slice.to_slice());
        Slice<uimax> l_slice_deserialized = slice_cast<uimax>(l_deserializer.slice());

        assert_true(l_slice.Size == l_slice_deserialized.Size);
        assert_true(l_slice.compare(l_slice_deserialized));
    }
};

inline void database_test()
{
    String l_database_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("asset.db"));
    {
        File l_tmp_file = File::create_or_open(l_database_path.to_slice());
        l_tmp_file.erase();
    }

    // create database and table
    {
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());

        SQLiteQuery l_query = SQLiteQuery::allocate(l_connection, slice_int8_build_rawstr(MULTILINE(create table if not exists test(id integer PRIMARY KEY);)));

        SQliteQueryExecution::execute_sync(l_connection, l_query.statement, []() {
        });

        l_query.free(l_connection);

        l_connection.free();
    }

    {
        File l_tmp_file = File::create_or_open(l_database_path.to_slice());
        l_tmp_file.erase();
    }

    // create/insert/read
    {
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_database_path.to_slice());

        SQLiteQuery l_table_creation_query =
            SQLiteQuery::allocate(l_connection, slice_int8_build_rawstr(MULTILINE(create table if not exists test(id integer PRIMARY KEY, num integer, txt text, data blob);)));

        SQliteQueryExecution::execute_sync(l_connection, l_table_creation_query.statement, []() {
        });
        l_table_creation_query.free(l_connection);

        SliceN<SQLiteQueryPrimitiveTypes, 4> l_parameter_layout_arr = {SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::TEXT,
                                                                       SQLiteQueryPrimitiveTypes::BLOB};
        SQLiteQueryLayout l_parameter_layout = SQLiteQueryLayout::build_slice(slice_from_slicen(&l_parameter_layout_arr));
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

        SQliteQueryExecution::execute_sync(l_connection, l_insersion_query.statement, []() {
        });

        l_insersion_query.free(l_connection);

        SliceN<SQLiteQueryPrimitiveTypes, 3> l_return_layout_arr =
            SliceN<SQLiteQueryPrimitiveTypes, 3>{SQLiteQueryPrimitiveTypes::INT64, SQLiteQueryPrimitiveTypes::TEXT, SQLiteQueryPrimitiveTypes::BLOB};
        SQLiteQueryLayout l_return_layout = SQLiteQueryLayout::build_slice(slice_from_slicen(&l_return_layout_arr));
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

    // Wrong file silent

    {
        String l_non_db_file = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("non_db_file.txt"));
        DatabaseConnection l_connection = DatabaseConnection::allocate(l_non_db_file.to_slice());
        assert_true(DatabaseConnection_is_valid_silent(l_connection) == 0);
        l_connection.free();
        l_non_db_file.free();
    }
};

inline void thread_test()
{
    struct s_my_thread
    {
        struct Input
        {
            int8 arg_1;
            int32 arg_2;
        } input;

        inline int8 operator()()
        {
            assert_true(this->input.arg_1 == 0);
            assert_true(this->input.arg_2 == 5);

            this->input.arg_1 = 1;

            return 0;
        };
    } my_thread;
    my_thread.input = s_my_thread::Input{0, 5};
    thread_native l_t = Thread::spawn_thread(my_thread);
    Thread::wait_for_end_and_terminate(l_t, -1);
    assert_true(my_thread.input.arg_1 == 1);
};

inline void barrier_test()
{
    BarrierTwoStep l_barrier_two_step = BarrierTwoStep{};
    Vector<int8> l_order_result = Vector<int8>::allocate(0);
    struct thread_1
    {
        Vector<int8>* order_result;
        BarrierTwoStep* barrier;

        struct Exec
        {
            thread_1* thiz;
            inline int8 operator()() const
            {
                thiz->barrier->ask_and_wait_for_sync_1();
                thiz->order_result->push_back_element(1);
                thiz->barrier->notify_sync_2();

                thiz->barrier->ask_and_wait_for_sync_1();
                thiz->order_result->push_back_element(3);
                thiz->barrier->notify_sync_2();

                return 0;
            };
        } exec;
    };

    struct thread_2
    {
        Vector<int8>* order_result;
        BarrierTwoStep* barrier;

        struct Exec
        {
            thread_2* thiz;
            inline int8 operator()() const
            {
                while (thiz->barrier->is_opened())
                {
                };

                thiz->order_result->push_back_element(0);
                thiz->barrier->notify_sync_1_and_wait_for_sync_2();
                thiz->order_result->push_back_element(2);
                while (thiz->barrier->is_opened())
                {
                };
                thiz->barrier->notify_sync_1_and_wait_for_sync_2();
                thiz->order_result->push_back_element(4);

                return 0;
            }
        } exec;
    };

    thread_1 l_t1 = thread_1{&l_order_result, &l_barrier_two_step};
    l_t1.exec = thread_1::Exec{&l_t1};
    thread_native l_tt1 = Thread::spawn_thread(l_t1.exec);

    thread_2 l_t2 = thread_2{&l_order_result, &l_barrier_two_step};
    l_t2.exec = thread_2::Exec{&l_t2};
    thread_native l_tt2 = Thread::spawn_thread(l_t2.exec);

    Thread::wait_for_end_and_terminate(l_tt1, -1);
    Thread::wait_for_end_and_terminate(l_tt2, -1);

    SliceN<int8, 4> l_awaited_result = {0, 1, 2, 3};
    assert_true(l_order_result.to_slice().compare(slice_from_slicen(&l_awaited_result)));

    l_order_result.free();
};

inline void file_test()
{
    String l_file_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("file_test.txt"));

    {
        File l_tmp_file = File::create_or_open(l_file_path.to_slice());
        l_tmp_file.erase();
    }

    File l_file = File::create(l_file_path.to_slice());
    assert_true(l_file.is_valid());

    SliceN<int8, 100> l_source_buffer_arr = {0, 1, 2, 3, 4, 5};
    Slice<int8> l_source_buffer = slice_from_slicen(&l_source_buffer_arr);
    SliceN<int8, 100> l_buffer_arr = {};
    Slice<int8> l_buffer = slice_from_slicen(&l_buffer_arr);

    l_file.write_file(l_source_buffer);
    l_file.read_file(&l_buffer);

    assert_true(l_buffer.compare(l_source_buffer));

    l_file.erase();

    l_file_path.free();

    // non null terminated path
    Span<int8> l_file_path_not_null_terminated = Span<int8>::allocate_slice_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("file_test.txtabcd"));
    l_file_path_not_null_terminated.Capacity -= 4;
    {
        File l_tmp_file = File::create_or_open(l_file_path_not_null_terminated.slice);
        l_tmp_file.erase();
    }
    l_file_path_not_null_terminated.free();
};

inline void native_window()
{
    Token<EWindow> l_window = WindowAllocator::allocate(300, 300, slice_int8_build_rawstr("TEST"));
    EWindow& l_allocated_window = WindowAllocator::get_window(l_window);
    assert_true(!l_allocated_window.is_closing);

    {
        uint32 l_native_width, l_native_height;
        window_native_get_window_client_dimensions(l_allocated_window.handle, &l_native_width, &l_native_height);
        assert_true(l_native_width == l_allocated_window.client_width);
        assert_true(l_native_height == l_allocated_window.client_height);

        assert_true(l_allocated_window.client_width == 300);
        assert_true(l_allocated_window.client_height == 300);
    }

    EWindow l_old_window = l_allocated_window;

    window_native_simulate_resize_appevent(l_allocated_window.handle, 400, 400);

    assert_true(l_allocated_window.resize_event.ask);
    assert_true(l_allocated_window.resize_event.new_frame_width == 400);
    assert_true(l_allocated_window.resize_event.new_frame_height == 400);

    l_allocated_window.consume_resize_event();

    assert_true(l_allocated_window.resize_event.ask == 0);
    assert_true(l_old_window.client_width != l_allocated_window.client_width);
    assert_true(l_old_window.client_height != l_allocated_window.client_height);

    {
        uint32 l_native_width, l_native_height;
        window_native_get_window_client_dimensions(l_allocated_window.handle, &l_native_width, &l_native_height);
        assert_true(l_native_width == l_allocated_window.client_width);
        assert_true(l_native_height == l_allocated_window.client_height);
    }

    assert_true(!l_allocated_window.is_closing);

    window_native_simulate_close_appevent(l_allocated_window.handle);
    assert_true(l_allocated_window.is_closing);

    WindowAllocator::get_window(l_window).close();
    assert_true(WindowAllocator::get_window(l_window).is_closing);
    WindowAllocator::free(l_window);
};

inline void socket_server_client_allocation_destruction_v2()
{
    SocketContext::allocate();

    // single server, without client
    {
        SocketServerNonBlocking l_server = SocketServerNonBlocking::allocate(SOCKET_DEFAULT_PORT);
        l_server.accept();
        l_server.step();
        l_server.close();
    }

    // single server, with client
    {
        SocketServerNonBlocking l_server = SocketServerNonBlocking::allocate(SOCKET_DEFAULT_PORT);
        l_server.accept();

        SocketClientNonBlocking l_client = SocketClientNonBlocking::allocate(SOCKET_DEFAULT_PORT);

        int8 l_accept_found = 0;
        while (!l_accept_found)
        {
            l_server.step();
            if (l_server.accept_return.acceped_socket.is_valid())
            {
                l_accept_found = 1;
            }
        }

        l_client.close();

        l_server.step();
        assert_true(!l_server.accept_return.acceped_socket.is_valid());

        l_server.close();
    }

    SocketContext::free();
};

/* Creates a SocketSocketServerSingleClient and a SocketClient. Send data in both direction. */
inline void socket_server_client_send_receive_test_v2()
{
    SocketContext::allocate();

    SocketServerNonBlocking l_server = SocketServerNonBlocking::allocate(SOCKET_DEFAULT_PORT);
    l_server.accept();
    SocketClientNonBlocking l_client = SocketClientNonBlocking::allocate(SOCKET_DEFAULT_PORT);
    SocketClientNonBlocking l_client_server_connection;

    int8 l_accept_found = 0;
    while (!l_accept_found)
    {
        l_server.step();
        if (l_server.accept_return.acceped_socket.is_valid())
        {
            l_client_server_connection = SocketClientNonBlocking::allocate_from_native(l_server.accept_return.acceped_socket);
            l_accept_found = 1;
        }
    }

    uimax l_input_value = 10;
    SocketClientNonBlocking::SendInput l_input;
    l_input.sended_buffer = Slice<uimax>::build_asint8_memory_singleelement(&l_input_value);
    l_client.send(l_input);

    SliceN<uimax, 1> l_client_server_connection_received_value = {};
    SocketClientNonBlocking::ReceiveInput l_receive_input;
    l_receive_input.target_buffer = slice_from_slicen(&l_client_server_connection_received_value).build_asint8();
    l_client_server_connection.receive(l_receive_input);

    SliceN<uimax, 1> l_client_received_value = {};
    SocketClientNonBlocking::ReceiveInput l_client_receive_input;
    l_client_receive_input.target_buffer = slice_from_slicen(&l_client_received_value).build_asint8();

    int8 l_data_received = 0;
    int8 l_server_response_received = 0;

    while (!l_data_received || !l_server_response_received)
    {
        l_client.step();
        l_client_server_connection.step();
        l_server.step();

        if (!l_data_received && l_client_server_connection.receive_state.received_bytes_total == sizeof(l_input_value))
        {
            l_data_received = 1;

            assert_true(l_client_server_connection_received_value.get(0) == 10);
            l_client_server_connection_received_value.get(0) = 15;

            l_client.receive(l_client_receive_input);

            SocketClientNonBlocking::SendInput l_send_input;
            l_send_input.sended_buffer = slice_from_slicen(&l_client_server_connection_received_value).build_asint8();
            l_client_server_connection.send(l_send_input);
        }

        if (!l_server_response_received && l_client.receive_state.received_bytes_total == sizeof(l_input_value))
        {
            l_server_response_received = 1;
            assert_true(l_client_received_value.get(0) == 15);
        }
    }

    // Trying to send on a closed client server

    l_client_server_connection.close();

    l_client_server_connection.send(l_input);

    l_client_received_value = {0};
    l_client.receive(l_client_receive_input);

    l_client.step();
    l_client_server_connection.step();
    l_server.step();

    assert_true(!l_client_server_connection.send_state.enabled && l_client_server_connection.send_state.sended_bytes == 0);

    l_client.close();
    l_server.close();

    SocketContext::free();
};

namespace test_constants
{
static const uimax CLIENT_REQUEST_CODE = 1;
static const uimax SERVER_RESPONSE_CODE = 2;
static const uimax TEST_VALUE_BEFORE = 0;
static const uimax TEST_VALUE_AFTER = 1;
} // namespace test_constants

/* Sending a request to server, but the size of the server buffer is too small -> request is done icnrementally. */
inline void socket_server_buffer_too_small()
{

    SocketContext::allocate();

    SocketServerNonBlocking l_server = SocketServerNonBlocking::allocate(SOCKET_DEFAULT_PORT);
    l_server.accept();
    SocketClientNonBlocking l_client = SocketClientNonBlocking::allocate(SOCKET_DEFAULT_PORT);
    SocketClientNonBlocking l_client_server_connection;

    int8 l_accept_found = 0;
    while (!l_accept_found)
    {
        l_server.step();
        if (l_server.accept_return.acceped_socket.is_valid())
        {
            l_client_server_connection = SocketClientNonBlocking::allocate_from_native(l_server.accept_return.acceped_socket);
            l_accept_found = 1;
        }
    }

    SliceN<uimax, 2> l_input_value = {10, 20};
    SocketClientNonBlocking::SendInput l_input;
    l_input.sended_buffer = slice_from_slicen(&l_input_value).build_asint8();
    l_client.send(l_input);

    SliceN<uimax, 1> l_client_server_connection_received_value = {};
    SocketClientNonBlocking::ReceiveInput l_receive_input;
    l_receive_input.target_buffer = slice_from_slicen(&l_client_server_connection_received_value).build_asint8();
    l_client_server_connection.receive(l_receive_input);

    int8 l_data_received = 0;

    while (!l_data_received)
    {
        l_client.step();
        l_client_server_connection.step();
        l_server.step();

        if (!l_data_received)
        {
            if (l_client_server_connection.receive_state.received_bytes_total == 8)
            {
                assert_true(l_client_server_connection.receive_state.received_bytes == 8);
                assert_true(l_client_server_connection_received_value.get(0) == 10);
            }
            else if (l_client_server_connection.receive_state.received_bytes_total == 16)
            {
                l_data_received = 1;
                assert_true(l_client_server_connection.receive_state.received_bytes == 8);
                assert_true(l_client_server_connection_received_value.get(0) == 20);
            }
        }
    }

    // Trying to send on a closed client server

    l_client_server_connection.close();
    l_client.close();
    l_server.close();

    SocketContext::free();
};

inline void command_buffer_pattern_test()
{
    struct MathOpCommand
    {
        enum class Type
        {
            ADD,
            MUL
        } type;
        uimax number;
    };

    using namespace pattern::cb;
    CommandBufferExecutionFlow<MathOpCommand> l_command_execution = CommandBufferExecutionFlow<MathOpCommand>::allocate_default();

    CommandPool<MathOpCommand> l_command_pool = CommandPool<MathOpCommand>::allocate_default();

    Token<CommandBuffer<MathOpCommand>> l_add_command_buffer_token = l_command_pool.allocate_command_buffer();
    CommandBuffer<MathOpCommand>& l_add_command_buffer = l_command_pool.command_buffers.get(l_add_command_buffer_token);
    l_add_command_buffer.commands.push_back_element(MathOpCommand{MathOpCommand::Type::ADD, 2});
    l_add_command_buffer.commands.push_back_element(MathOpCommand{MathOpCommand::Type::ADD, 3});

    Token<CommandBuffer<MathOpCommand>> l_mul_command_buffer_token = l_command_pool.allocate_command_buffer();
    CommandBuffer<MathOpCommand>& l_mul_command_buffer = l_command_pool.command_buffers.get(l_mul_command_buffer_token);
    l_mul_command_buffer.commands.push_back_element(MathOpCommand{MathOpCommand::Type::MUL, 3});

    Token<NNTree<Token<CommandBuffer<MathOpCommand>>>::Node> l_add_command_execution = l_command_execution.push_command_buffer(l_add_command_buffer_token);
    Semaphore<MathOpCommand> l_semaphore;
    l_semaphore.execute_before = l_add_command_execution;
    l_command_execution.push_command_buffer_with_constraint(l_mul_command_buffer_token, l_semaphore);

    uimax l_result = 2;
    l_command_execution.process_command_buffer_tree(l_command_pool, [&](const Token<CommandBuffer<MathOpCommand>>, CommandBuffer<MathOpCommand>& p_command) {
        for (loop(i, 0, p_command.commands.Size))
        {
            MathOpCommand& l_command = p_command.commands.get(i);
            switch (l_command.type)
            {
            case MathOpCommand::Type::ADD:
                l_result = l_result + l_command.number;
                break;
            case MathOpCommand::Type::MUL:
                l_result = l_result * l_command.number;
                break;
            }
        }
    });

    assert_true(l_result == 21);

    l_command_pool.free_command_buffer(l_add_command_buffer_token);
    l_command_pool.free_command_buffer(l_mul_command_buffer_token);
    l_command_pool.free();
    l_command_execution.free();
};

int main(int argc, int8** argv)
{
    slice_span_test();
    slice_functional_algorithm_test();
    base64_test();
    vector_test();
    vector_slice_test();
    hashmap_test();
    pool_test();
    varyingslice_test();
    varyingvector_test();
    vectorofvector_test();
    poolofvector_test();
    pool_hashed_counted_test();
    ntree_test();
    nntree_test();
    sort_test();
    heap_test();
    heappaged_test();
    heap_memory_test();
    string_test();
    path_test();
    fromstring_test();
    deserialize_json_test();
    serialize_json_test();
    serialize_deserialize_binary_test();
    file_test();
    thread_test();
    socket_server_client_allocation_destruction_v2();
    socket_server_client_send_receive_test_v2();
    socket_server_buffer_too_small();
    native_window();
    database_test();
    command_buffer_pattern_test();

    memleak_ckeck();
}

#include "Common2/common2_external_implementation.hpp"