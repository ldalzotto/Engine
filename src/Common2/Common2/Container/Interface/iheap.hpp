#pragma once

struct iHeapTypes
{
    using AllocationState_t = int8;
    enum class AllocationState : AllocationState_t
    {
        NOT_ALLOCATED = 1,
        ALLOCATED = 2,
        HEAP_RESIZED = 4,
        ALLOCATED_AND_HEAP_RESIZED = ALLOCATED | HEAP_RESIZED
    };
};

template <class _Heap> struct iHeap
{
    _Heap& heap;

    using t_FreeChunks = typename _Heap::t_FreeChunks;
    using t_FreeChunksRef = typename _Heap::t_FreeChunksRef;

    using t_AllocatedChunks = typename _Heap::t_AllocatedChunks;
    using t_AllocatedChunksRef = typename _Heap::t_AllocatedChunksRef;
    using t_AllocatedChunks_sTokenValue = typename t_AllocatedChunks::sTokenValue;
    using t_AllocatedChunks_sToken = typename t_AllocatedChunks::sToken;

    struct AllocatedElementReturn
    {
        t_AllocatedChunks_sToken token;
        uimax Offset;

        inline static AllocatedElementReturn build(const t_AllocatedChunks_sToken p_token, const uimax p_offset)
        {
            return AllocatedElementReturn{p_token, p_offset};
        };
    };

    inline uimax get_size()
    {
        return this->heap.get_size();
    };

    inline void resize(const uimax p_new_size)
    {
        this->heap.resize(p_new_size);
    };

    inline t_FreeChunksRef get_free_chunks()
    {
        return this->heap.get_freechunks();
    };

    inline t_AllocatedChunksRef get_allocated_chunks()
    {
        return this->heap.get_allocated_chunks();
    };

    inline iHeapTypes::AllocationState allocate_element(const uimax p_size, AllocatedElementReturn* out_chunk)
    {
        if (!_allocate_element(p_size, out_chunk))
        {
            _defragment();
            if (!_allocate_element(p_size, out_chunk))
            {
                uimax l_heap_size = this->get_size();
                this->resize(l_heap_size == 0 ? p_size : ((l_heap_size * 2) + p_size));

#if __DEBUG
                assert_true(
#endif
                    _allocate_element(p_size, out_chunk)
#if __DEBUG
                )
#endif
                    ;

                return iHeapTypes::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
            }
            return iHeapTypes::AllocationState::ALLOCATED;
        }
        return iHeapTypes::AllocationState::ALLOCATED;
    };

    inline iHeapTypes::AllocationState allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk)
    {
        if (!_allocate_element_with_modulo_offset(p_size, p_modulo_offset, out_chunk))
        {
            _defragment();
            if (!_allocate_element_with_modulo_offset(p_size, p_modulo_offset, out_chunk))
            {
                uimax l_heap_size = this->get_size();
                this->resize(l_heap_size == 0 ? (p_modulo_offset > p_size ? p_modulo_offset : p_size) : ((l_heap_size * 2) + (p_modulo_offset > p_size ? p_modulo_offset : p_size)));

#if __DEBUG
                assert_true(
#endif
                    _allocate_element_with_modulo_offset(p_size, p_modulo_offset, out_chunk)
#if __DEBUG
                )
#endif
                    ;

                return iHeapTypes::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
            }
            return iHeapTypes::AllocationState::ALLOCATED;
        }
        return iHeapTypes::AllocationState::ALLOCATED;
    };

    inline iHeapTypes::AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_alignement_modulo, AllocatedElementReturn* out_chunk)
    {
        if (!_allocate_element_with_modulo_offset(p_size, p_alignement_modulo, out_chunk))
        {
            _defragment();
            if (!_allocate_element_with_modulo_offset(p_size, p_alignement_modulo, out_chunk))
            {
                return iHeapTypes::AllocationState::NOT_ALLOCATED;
            }
            return iHeapTypes::AllocationState::ALLOCATED;
        }
        return iHeapTypes::AllocationState::ALLOCATED;
    };

  private:
#if __DEBUG
    inline static void _assert_memory_alignment(const uimax p_alignment_modulo, const AllocatedElementReturn& p_allocated_chunk)
    {
        assert_true((p_allocated_chunk.Offset % p_alignment_modulo) == 0);
    };
#endif

    inline int8 _allocate_element(const uimax p_size, AllocatedElementReturn* out_return)
    {
#if __DEBUG
        assert_true(p_size != 0);
#endif

        t_FreeChunksRef __free_chunks = this->get_free_chunks();
        iVector<t_FreeChunks> l_free_chunks = iVector<t_FreeChunks>{__free_chunks};

        for (loop_reverse(i, 0, l_free_chunks.get_size()))
        {
            SliceIndex& l_free_chunk = l_free_chunks.get(i);
            if (l_free_chunk.Size > p_size)
            {
                SliceIndex l_new_allocated_chunk;
                l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
                *out_return = _push_chunk(&l_new_allocated_chunk);
                return 1;
            }
            else if (l_free_chunk.Size == p_size)
            {
                *out_return = _push_chunk(&l_free_chunk);
                l_free_chunks.erase_element_at_always(i);
                return 1;
            }
        }

        return 0;
    };

    inline int8 _allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk)
    {
#if __DEBUG
        assert_true(p_size != 0);
#endif

        t_FreeChunksRef __free_chunks = this->get_free_chunks();
        iVector<t_FreeChunks> l_free_chunks = iVector<t_FreeChunks>{__free_chunks};
        // using ShadowVector(free_chunks) = decltype(sh_c_get_freechunks(p_heap));
        // ShadowVector(free_chunks)& l_free_chunks = sh_c_get_freechunks(p_heap);
        for (loop_reverse(i, 0, l_free_chunks.get_size()))
        // for (uimax i = 0; i < sv_c_get_size(&l_free_chunks); i++)
        {
            SliceIndex& l_free_chunk = l_free_chunks.get(i);

            if (l_free_chunk.Size > p_size)
            {
                uimax l_offset_modulo = (l_free_chunk.Begin % p_modulo_offset);
                if (l_offset_modulo == 0)
                {
                    // create one free chunk (after)
                    SliceIndex l_new_allocated_chunk;
                    l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
                    *out_chunk = _push_chunk(&l_new_allocated_chunk);

#if __DEBUG
                    _assert_memory_alignment(p_modulo_offset, *out_chunk);
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
                        *out_chunk = _push_chunk(&l_new_allocated_chunk);

                        l_free_chunks.push_back_element(l_new_free_chunk);

#if __DEBUG
                        _assert_memory_alignment(p_modulo_offset, *out_chunk);
#endif
                        return 1;
                    }
                    else if (l_free_chunk.Size == (p_size + l_chunk_offset_delta)) // offsetted chunk end matches perfectly the end of the free chunk
                    {
                        SliceIndex l_new_allocated_chunk;
                        l_free_chunk.slice_two(l_free_chunk.Begin + l_chunk_offset_delta, &l_free_chunk, &l_new_allocated_chunk);
                        *out_chunk = _push_chunk(&l_new_allocated_chunk);

                        return 1;
                    }
                }
            }
            else if (l_free_chunk.Size == p_size)
            {
                uimax l_offset_modulo = (l_free_chunk.Begin % p_modulo_offset);
                if (l_offset_modulo == 0)
                {
                    *out_chunk = _push_chunk(&l_free_chunk);
                    l_free_chunks.erase_element_at_always(i);
#if __DEBUG
                    _assert_memory_alignment(p_modulo_offset, *out_chunk);
#endif
                    return 1;
                }
            }
        }

        return 0;
    };

    inline AllocatedElementReturn _push_chunk(SliceIndex* p_chunk)
    {
        t_AllocatedChunksRef __allocated_chunks = this->get_allocated_chunks();
        return AllocatedElementReturn::build(__allocated_chunks.to_ipool().allocate_element_v2(*p_chunk), p_chunk->Begin);
    };

    inline void _defragment()
    {
        t_FreeChunksRef __free_chunks = this->get_free_chunks();
        iVector<t_FreeChunks> l_free_chunks = iVector<t_FreeChunks>{__free_chunks};

        if (l_free_chunks.get_size() > 0)
        {
            {
                struct FreeChunkSortByAddress
                {
                    inline int8 operator()(const SliceIndex& p_left, const SliceIndex& p_right) const
                    {
                        return p_left.Begin > p_right.Begin;
                    };
                };
                Slice<SliceIndex> l_freechunks_slice = l_free_chunks.to_slice();
                Sort::Linear3(l_freechunks_slice, 0, FreeChunkSortByAddress{});
            }

            SliceIndex& l_compared_chunk = l_free_chunks.get(0);
            for (loop(i, 1, l_free_chunks.get_size()))
            {
                SliceIndex& l_chunk = l_free_chunks.get(i);
                if ((l_compared_chunk.Begin + l_compared_chunk.Size) == l_chunk.Begin)
                {
                    l_compared_chunk.Size += l_chunk.Size;
                    l_free_chunks.erase_element_at_always(i);
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
