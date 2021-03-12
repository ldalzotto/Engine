
#include "../Common2/common2.h"

Slice_declare(uimax);
SpanC_declare(uimax);
SpanC_declare_functions(uimax);

inline void assert_span_unitialized(Span_* p_span)
{
    assert_true(p_span->Capacity == 0);
    assert_true(p_span->Memory == NULL);
};

inline void slice_span_test()
{
    Span_uimax l_span_sizet = Span_uimax_build(NULL, 0);
    // When resizing the span, new memory is allocated
    {
        uimax l_new_capacity = 10;
        Span_uimax_resize(&l_span_sizet, 10);
        assert_true(l_span_sizet.Capacity == l_new_capacity);
        assert_true(l_span_sizet.Memory != NULL);
    }

    // When freeing the span, it's structure is resetted
    {
        Span_uimax_free(&l_span_sizet);
        assert_span_unitialized(&l_span_sizet.span);
    }

    /*
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
            l_span_sizet.slice.copy_memory_2(1, l_slice_1.to_slice(), l_slice_2.to_slice());

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
         */
};

/*
inline void vector_test()
{
    Vector<uimax> l_vector_sizet = Vector<uimax>::build_zero_size((uimax*)NULL, 0);

    // vector_push_back_array
    {
        uimax l_old_size = l_vector_sizet.Size;
        uimax l_elements[5] = {0, 1, 2, 3, 4};
        Slice<uimax> l_elements_slice = Slice<uimax>::build(l_elements, 5);

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
        Slice<uimax> l_elements_slice = Slice<uimax>::build(l_elements, 5);
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


*/

inline void Slice_uimax_sort_decroissant(SliceC(uimax) * p_slice)
{
    Sort_Linear_inline(uimax, (&p_slice->slice), { l_comparison = *l_left <= *l_right; });
};

int main()
{
    slice_span_test();

    memleak_ckeck();
}