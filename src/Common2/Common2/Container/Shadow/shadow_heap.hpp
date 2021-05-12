#pragma once

struct ShadowHeap_v2
{
    template <class Heap> inline static uimax get_size(const Heap& p_heap)
    {
        return p_heap.get_size();
    };

    template <class Heap> inline static void resize(Heap& p_heap, const uimax p_new_size)
    {
        p_heap.resize(p_new_size);
    };

    template <class Heap> inline static typename Heap::_FreeChunks get_free_chunks(Heap& p_heap)
    {
        return p_heap.get_freechunks();
    };

    template <class Heap> inline static typename Heap::_AllocatedChunks get_allocated_chunks(Heap& p_heap)
    {
        return p_heap.get_allocated_chunks();
    };
};

#define ShadowHeap(Prefix) ShadowHeap_##Prefix

#define sh_func_get_size() get_size()
#define sh_func_resize(p_newsize) resize(p_newsize)
#define sh_func_get_freechunks() get_freechunks()
#define sh_func_get_allocated_chunks() get_allocated_chunks()

#define sh_c_get_size(p_shadow_heap) (p_shadow_heap).sh_func_get_size()
#define sh_c_resize(p_shadow_heap, p_newsize) (p_shadow_heap).sh_func_resize(p_newsize)
#define sh_c_get_freechunks(p_shadow_heap) (p_shadow_heap).sh_func_get_freechunks()
#define sh_c_get_allocated_chunks(p_shadow_heap) (p_shadow_heap).sh_func_get_allocated_chunks()

struct HeapAlgorithms
{
    enum class AllocationState
    {
        NOT_ALLOCATED = 1,
        ALLOCATED = 2,
        HEAP_RESIZED = 4,
        ALLOCATED_AND_HEAP_RESIZED = ALLOCATED | HEAP_RESIZED
    };
    using AllocationState_t = uint8;

    struct AllocatedElementReturn
    {
        Token<SliceIndex> token;
        uimax Offset;

        inline static AllocatedElementReturn build(const Token<SliceIndex> p_token, const uimax p_offset)
        {
            return AllocatedElementReturn{p_token, p_offset};
        };
    };

    template <class ShadowHeap(s)> inline static AllocationState allocate_element(ShadowHeap(s) & p_heap, const uimax p_size, AllocatedElementReturn* out_chunk)
    {
        if (!_allocate_element(p_heap, p_size, out_chunk))
        {
            _defragment(p_heap);
            if (!_allocate_element(p_heap, p_size, out_chunk))
            {
                uimax l_heap_size = sh_c_get_size(p_heap);
                sh_c_resize(p_heap, l_heap_size == 0 ? p_size : ((l_heap_size * 2) + p_size));

#if __DEBUG
                assert_true(
#endif
                    _allocate_element(p_heap, p_size, out_chunk)
#if __DEBUG
                )
#endif
                    ;

                return HeapAlgorithms::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
            }
            return HeapAlgorithms::AllocationState::ALLOCATED;
        }
        return HeapAlgorithms::AllocationState::ALLOCATED;
    };

    template <class ShadowHeap(s)>
    inline static AllocationState allocate_element_with_modulo_offset(ShadowHeap(s) & p_heap, const uimax p_size, const uimax p_modulo_offset, HeapAlgorithms::AllocatedElementReturn* out_chunk)
    {
        if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk))
        {
            _defragment(p_heap);
            if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk))
            {
                uimax l_heap_size = sh_c_get_size(p_heap);
                sh_c_resize(p_heap, l_heap_size == 0 ? (p_modulo_offset > p_size ? p_modulo_offset : p_size) : ((l_heap_size * 2) + (p_modulo_offset > p_size ? p_modulo_offset : p_size)));

#if __DEBUG
                assert_true(
#endif
                    _allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk)
#if __DEBUG
                )
#endif
                    ;

                return HeapAlgorithms::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
            }
            return HeapAlgorithms::AllocationState::ALLOCATED;
        }
        return HeapAlgorithms::AllocationState::ALLOCATED;
    };

    template <class ShadowHeap(s)>
    inline static AllocationState allocate_element_norealloc_with_modulo_offset(ShadowHeap(s) & p_heap, const uimax p_size, const uimax p_alignement_modulo,
                                                                                HeapAlgorithms::AllocatedElementReturn* out_chunk)
    {
        if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_alignement_modulo, out_chunk))
        {
            _defragment(p_heap);
            if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_alignement_modulo, out_chunk))
            {
                return AllocationState::NOT_ALLOCATED;
            }
            return AllocationState::ALLOCATED;
        }
        return AllocationState::ALLOCATED;
    };

  private:
#if __DEBUG
    inline static void _assert_memory_alignment(const uimax p_alignment_modulo, const HeapAlgorithms::AllocatedElementReturn& p_allocated_chunk)
    {
        assert_true((p_allocated_chunk.Offset % p_alignment_modulo) == 0);
    };
#endif

    template <class ShadowHeap(s)> inline static int8 _allocate_element(ShadowHeap(s) & p_heap, const uimax p_size, HeapAlgorithms::AllocatedElementReturn* out_return)
    {
#if __DEBUG
        assert_true(p_size != 0);
#endif

        using ShadowVector(free_chunks) = decltype(sh_c_get_freechunks(p_heap));
        ShadowVector(free_chunks)& l_free_chunks = sh_c_get_freechunks(p_heap);
        for (loop_reverse(i, 0, sv_c_get_size(l_free_chunks)))
        {
            SliceIndex& l_free_chunk = sv_c_get(l_free_chunks, i);
            if (l_free_chunk.Size > p_size)
            {
                SliceIndex l_new_allocated_chunk;
                l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
                *out_return = _push_chunk(p_heap, &l_new_allocated_chunk);
                return 1;
            }
            else if (l_free_chunk.Size == p_size)
            {
                *out_return = _push_chunk(p_heap, &l_free_chunk);
                sv_c_erase_element_at_always(l_free_chunks, i);
                return 1;
            }
        }

        return 0;
    };


    template <class Heap>
    inline static int8 _allocate_element_with_modulo_offset(Heap& p_heap, const uimax p_size, const uimax p_modulo_offset, HeapAlgorithms::AllocatedElementReturn* out_chunk)
    {
#if __DEBUG
        assert_true(p_size != 0);
#endif

        typename Heap::_FreeChunks l_free_chunks = ShadowHeap_v2::get_free_chunks(p_heap);
        // using ShadowVector(free_chunks) = decltype(sh_c_get_freechunks(p_heap));
        // ShadowVector(free_chunks)& l_free_chunks = sh_c_get_freechunks(p_heap);
        for (loop_reverse(i, 0, ShadowVector_v2::get_size(l_free_chunks)))
            // for (uimax i = 0; i < sv_c_get_size(&l_free_chunks); i++)
        {
            SliceIndex& l_free_chunk = ShadowVector_v2::get(l_free_chunks, i);

            if (l_free_chunk.Size > p_size)
            {
                uimax l_offset_modulo = (l_free_chunk.Begin % p_modulo_offset);
                if (l_offset_modulo == 0)
                {
                    // create one free chunk (after)
                    SliceIndex l_new_allocated_chunk;
                    l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
                    *out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);

#if __DEBUG
                    HeapAlgorithms::_assert_memory_alignment(p_modulo_offset, *out_chunk);
#endif
                    return 1;
                }
                else
                {
                    uimax l_chunk_offset_delta = p_modulo_offset - l_offset_modulo;
                    // Does the offsetted new memory is able to be allocated in the chunk ?
                    if (l_free_chunk.Size > (p_size + l_chunk_offset_delta)) // offsetted chunk is in the middle of the free chunk
                    {
                        // create two free chunk (before and after)

                        SliceIndex l_new_allocated_chunk, l_new_free_chunk, l_tmp_chunk;
                        l_free_chunk.slice_two(l_free_chunk.Begin + l_chunk_offset_delta, &l_free_chunk, &l_tmp_chunk);
                        l_tmp_chunk.slice_two(l_tmp_chunk.Begin + p_size, &l_new_allocated_chunk, &l_new_free_chunk);
                        *out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);

                        ShadowVector_v2::push_back_element(l_free_chunks, l_new_free_chunk);

#if __DEBUG
                        HeapAlgorithms::_assert_memory_alignment(p_modulo_offset, *out_chunk);
#endif
                        return 1;
                    }
                    else if (l_free_chunk.Size == (p_size + l_chunk_offset_delta)) // offsetted chunk end matches perfectly the end of the free chunk
                    {
                        SliceIndex l_new_allocated_chunk;
                        l_free_chunk.slice_two(l_free_chunk.Begin + l_chunk_offset_delta, &l_free_chunk, &l_new_allocated_chunk);
                        *out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);

                        return 1;
                    }
                }
            }
            else if (l_free_chunk.Size == p_size)
            {
                uimax l_offset_modulo = (l_free_chunk.Begin % p_modulo_offset);
                if (l_offset_modulo == 0)
                {
                    *out_chunk = _push_chunk(p_heap, &l_free_chunk);
                    ShadowVector_v2::erase_element_at_always(l_free_chunks, i);
#if __DEBUG
                    HeapAlgorithms::_assert_memory_alignment(p_modulo_offset, *out_chunk);
#endif
                    return 1;
                }
            }
        }

        return 0;
    };

    template <class Heap> inline static AllocatedElementReturn _push_chunk(Heap& p_heap, SliceIndex* p_chunk)
    {
        typename Heap::_AllocatedChunks l_allocated_chunks = ShadowHeap_v2::get_allocated_chunks(p_heap);
        return HeapAlgorithms::AllocatedElementReturn::build(PoolAlgorithms::allocate_element_v2(l_allocated_chunks, *p_chunk), p_chunk->Begin);
    };

    template <class Heap> inline static void _defragment(Heap& p_heap)
    {
        typename Heap::_FreeChunks l_free_chunks = ShadowHeap_v2::get_free_chunks(p_heap);

        if (ShadowVector_v2::get_size(l_free_chunks) > 0)
        {
            {
                struct FreeChunkSortByAddress
                {
                    inline int8 operator()(const SliceIndex& p_left, const SliceIndex& p_right) const
                    {
                        return p_left.Begin > p_right.Begin;
                    };
                };
                Slice<SliceIndex> l_freechunks_slice = ShadowVector_v2::to_slice(l_free_chunks);
                Sort::Linear3(l_freechunks_slice, 0, FreeChunkSortByAddress{});
            }

            SliceIndex& l_compared_chunk = ShadowVector_v2::get(l_free_chunks, 0);
            for (loop(i, 1, sv_c_get_size(l_free_chunks)))
            {
                SliceIndex& l_chunk = ShadowVector_v2::get(l_free_chunks, i);
                if ((l_compared_chunk.Begin + l_compared_chunk.Size) == l_chunk.Begin)
                {
                    l_compared_chunk.Size += l_chunk.Size;
                    ShadowVector_v2::erase_element_at_always(l_free_chunks, i);
                    i -= 1;
                }
                else
                {
                    l_compared_chunk = l_chunk;
                }
            }
        }
    };
};