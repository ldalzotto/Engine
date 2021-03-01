#pragma once

#define ShadowHeap_t(Prefix) ShadowHeap_##Prefix
#define sh_get_size() get_size()
#define sh_resize(p_newsize) resize(p_newsize)
#define sh_get_freechunks() get_freechunks()
#define sh_get_allocated_chunks() get_allocated_chunks()

struct HeapA
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
        Token(SliceIndex) token;
        uimax Offset;

        inline static AllocatedElementReturn build(const Token(SliceIndex) p_token, const uimax p_offset)
        {
            return AllocatedElementReturn{p_token, p_offset};
        };
    };

    template <class ShadowHeap_t(s)> static AllocationState allocate_element(ShadowHeap_t(s) & p_heap, const uimax p_size, AllocatedElementReturn* out_chunk);

    template <class ShadowHeap_t(s)>
    static AllocationState allocate_element_with_modulo_offset(ShadowHeap_t(s) & p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);

    template <class ShadowHeap_t(s)>
    static AllocationState allocate_element_norealloc_with_modulo_offset(ShadowHeap_t(s) & p_heap, const uimax p_size, const uimax p_alignement_modulo, HeapA::AllocatedElementReturn* out_chunk);

  private:
    template <class ShadowHeap_t(s)> static int8 _allocate_element(ShadowHeap_t(s) & p_heap, const uimax p_size, HeapA::AllocatedElementReturn* out_return);

    template <class ShadowHeap_t(s)>
    static int8 _allocate_element_with_modulo_offset(ShadowHeap_t(s) & p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);

    template <class ShadowHeap_t(s)> static AllocatedElementReturn _push_chunk(ShadowHeap_t(s) & p_heap, SliceIndex* p_chunk);

    template <class ShadowHeap_t(s)> static void _defragment(ShadowHeap_t(s) & p_heap);
};

/*
    A Heap is a function object that calculates : "what is the offset of the chunk of data you want to allocate ?".
    The Heap object doesn't actually allocate any memory. It is up to the consumer to choose how memory is allocated.
    The heap has been implemented like that to allow flexibility between allocating memory on the CPU or GPU. (or any other logic)
*/
struct Heap
{
    Pool<SliceIndex> AllocatedChunks;
    Vector<SliceIndex> FreeChunks;
    uimax Size;

    static Heap allocate(const uimax p_heap_size);
    void free();
    void sh_resize(const uimax p_newsize);
    uimax sh_get_size();
    Vector<SliceIndex>& sh_get_freechunks();
    Pool<SliceIndex>& sh_get_allocated_chunks();
    HeapA::AllocationState allocate_element(const uimax p_size, HeapA::AllocatedElementReturn* out_chunk);
    HeapA::AllocationState allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);
    HeapA::AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk);
    SliceIndex* get(const Token(SliceIndex) p_chunk);
    void release_element(const Token(SliceIndex) p_chunk);
    HeapA::AllocationState reallocate_element(const Token(SliceIndex) p_chunk, const uimax p_new_size, HeapA::AllocatedElementReturn* out_chunk);
};

struct HeapPagedToken
{
    uimax PageIndex;
    Token(SliceIndex) token;
};

/*
    A HeapPaged is a function object that calculates : "what is the offset of the chunk of data you want to allocate ?".
    It has the same behavior of the Heap. But, allocated chunks are never deallocated.
    Instead, when allocation is not possible with the current chunks, it allocates another chunk.
*/
struct HeapPaged
{
    typedef uint8 AllocationState_t;
    enum class AllocationState
    {
        NOT_ALLOCATED = 0,
        ALLOCATED = 1,
        PAGE_CREATED = 2,
        ALLOCATED_AND_PAGE_CREATED = ALLOCATED | PAGE_CREATED
    };

    struct AllocatedElementReturn
    {
        HeapPagedToken token;
        uimax Offset;

        inline static AllocatedElementReturn buid_from_HeapAllocatedElementReturn(const uimax p_page_index, const HeapA::AllocatedElementReturn& p_heap_allocated_element_return)
        {
            return AllocatedElementReturn{HeapPagedToken{p_page_index, p_heap_allocated_element_return.token}, p_heap_allocated_element_return.Offset};
        };
    };

    Pool<SliceIndex> AllocatedChunks;
    VectorOfVector<SliceIndex> FreeChunks;
    uimax PageSize;

    /*
        A ShadowHeap_t that wraps a sigle Heap of the HeapPaged to use it on Heap generic algorithms.
    */
    struct SingleShadowHeap
    {
        HeapPaged* heapPaged;
        uimax index;
        VectorOfVector<SliceIndex>::Element_ShadowVector tmp_shadow_vec;

        inline static SingleShadowHeap build(HeapPaged* p_heap_paged, const uimax p_index)
        {
            SingleShadowHeap l_shadow_heap;
            l_shadow_heap.heapPaged = p_heap_paged;
            l_shadow_heap.index = p_index;
            return l_shadow_heap;
        };

        inline VectorOfVector<SliceIndex>::Element_ShadowVector& sh_get_freechunks()
        {
            this->tmp_shadow_vec = this->heapPaged->FreeChunks.element_as_shadow_vector(this->index);
            return tmp_shadow_vec;
        };

        inline Pool<SliceIndex>& sh_get_allocated_chunks()
        {
            return this->heapPaged->AllocatedChunks;
        };
    };

    static HeapPaged allocate_default(const uimax p_page_size);
    void free();
    uimax get_page_count();
    AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk);
    void release_element(const HeapPagedToken& p_token);
    SliceIndex* get_sliceindex_only(const HeapPagedToken& p_token);

  private:
    void create_new_page();
};

template <class ShadowHeap_t(s)> inline HeapA::AllocationState HeapA::allocate_element(ShadowHeap_t(s) & p_heap, const uimax p_size, HeapA::AllocatedElementReturn* out_chunk)
{
    if (!_allocate_element(p_heap, p_size, out_chunk))
    {
        _defragment(p_heap);
        if (!_allocate_element(p_heap, p_size, out_chunk))
        {
            uimax l_heap_size = p_heap.sh_get_size();
            p_heap.sh_resize(l_heap_size == 0 ? p_size : ((l_heap_size * 2) + p_size));

#if CONTAINER_MEMORY_TEST
            assert_true(
#endif
                _allocate_element(p_heap, p_size, out_chunk)
#if CONTAINER_MEMORY_TEST
            )
#endif
                ;

            return HeapA::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
        }
        return HeapA::AllocationState::ALLOCATED;
    }
    return HeapA::AllocationState::ALLOCATED;
}

template <class ShadowHeap_t(s)>
inline HeapA::AllocationState HeapA::allocate_element_with_modulo_offset(ShadowHeap_t(s) & p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
{
    if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk))
    {
        _defragment(p_heap);
        if (!_allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk))
        {
            uimax l_heap_size = p_heap.sh_get_size();
            p_heap.sh_resize(l_heap_size == 0 ? (p_modulo_offset > p_size ? p_modulo_offset : p_size) : ((l_heap_size * 2) + (p_modulo_offset > p_size ? p_modulo_offset : p_size)));

#if CONTAINER_MEMORY_TEST
            assert_true(
#endif
                _allocate_element_with_modulo_offset(p_heap, p_size, p_modulo_offset, out_chunk)
#if CONTAINER_MEMORY_TEST
            )
#endif
                ;

            return HeapA::AllocationState::ALLOCATED_AND_HEAP_RESIZED;
        }
        return HeapA::AllocationState::ALLOCATED;
    }
    return HeapA::AllocationState::ALLOCATED;
}

template <class ShadowHeap_t(s)>
inline HeapA::AllocationState HeapA::allocate_element_norealloc_with_modulo_offset(ShadowHeap_t(s) & p_heap, const uimax p_size, const uimax p_alignement_modulo,
                                                                                   HeapA::AllocatedElementReturn* out_chunk)
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
}

template <class ShadowHeap_t(s)> inline int8 HeapA::_allocate_element(ShadowHeap_t(s) & p_heap, const uimax p_size, HeapA::AllocatedElementReturn* out_return)
{
#if CONTAINER_BOUND_TEST
    assert_true(p_size != 0);
#endif

    auto& l_free_chunks = p_heap.sh_get_freechunks();
    for (uimax i = 0; i < l_free_chunks.sv_get_size(); i++)
    {
        SliceIndex& l_free_chunk = l_free_chunks.sv_get(i);
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
            l_free_chunks.sv_erase_element_at_always(i);
            return 1;
        }
    }

    return 0;
}

template <class ShadowHeap_t(s)>
inline int8 HeapA::_allocate_element_with_modulo_offset(ShadowHeap_t(s) & p_heap, const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
{
#if CONTAINER_BOUND_TEST
    assert_true(p_size != 0);
#endif

    auto& l_free_chunks = p_heap.sh_get_freechunks();
    for (uimax i = 0; i < l_free_chunks.sv_get_size(); i++)
    {
        SliceIndex& l_free_chunk = l_free_chunks.sv_get(i);

        if (l_free_chunk.Size > p_size)
        {
            uimax l_offset_modulo = (l_free_chunk.Begin % p_modulo_offset);
            if (l_offset_modulo == 0)
            {
                // create one free chunk (after)
                SliceIndex l_new_allocated_chunk;
                l_free_chunk.slice_two(l_free_chunk.Begin + p_size, &l_new_allocated_chunk, &l_free_chunk);
                *out_chunk = _push_chunk(p_heap, &l_new_allocated_chunk);
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

                    l_free_chunks.sv_push_back_element(l_new_free_chunk);

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
            uimax l_offset_modulo = (l_free_chunk.Size % p_modulo_offset);
            if (l_offset_modulo == 0)
            {
                *out_chunk = _push_chunk(p_heap, &l_free_chunk);
                l_free_chunks.sv_erase_element_at(i);

                return 1;
            }
        }
    }

    return 0;
}

template <class ShadowHeap_t(s)> inline HeapA::AllocatedElementReturn HeapA::_push_chunk(ShadowHeap_t(s) & p_heap, SliceIndex* p_chunk)
{
    return HeapA::AllocatedElementReturn::build(p_heap.sh_get_allocated_chunks().alloc_element(*p_chunk), p_chunk->Begin);
}

template <class ShadowHeap_t(s)> inline void HeapA::_defragment(ShadowHeap_t(s) & p_heap)
{
    auto& l_free_chunks = p_heap.sh_get_freechunks();
    if (l_free_chunks.sv_get_size() > 0)
    {
        sort_linear2_begin(SliceIndex, defragment_sort);
        return p_left.Begin > p_right.Begin;
        sort_linear2_end(l_free_chunks.sv_to_slice(), SliceIndex, defragment_sort);

        SliceIndex& l_compared_chunk = l_free_chunks.sv_get(0);
        for (loop(i, 1, l_free_chunks.sv_get_size()))
        {
            SliceIndex& l_chunk = l_free_chunks.sv_get(i);
            if ((l_compared_chunk.Begin + l_compared_chunk.Size) == l_chunk.Begin)
            {
                l_compared_chunk.Size += l_chunk.Size;
                l_free_chunks.sv_erase_element_at_always(i);
                i -= 1;
            }
            else
            {
                l_compared_chunk = l_chunk;
            }
        }
    }
}

inline Heap Heap::allocate(const uimax p_heap_size)
{
    Heap l_heap = Heap{Pool<SliceIndex>::allocate(0), Vector<SliceIndex>::allocate(1), p_heap_size};
    l_heap.FreeChunks.push_back_element(SliceIndex::build(0, p_heap_size));
    return l_heap;
}

inline void Heap::free()
{
    this->AllocatedChunks.free();
    this->FreeChunks.free();
    this->Size = 0;
}

inline void Heap::sh_resize(const uimax p_newsize)
{
    uimax l_old_size = this->Size;
    this->FreeChunks.push_back_element(SliceIndex::build(l_old_size, p_newsize - l_old_size));
    this->Size = p_newsize;
}

inline uimax Heap::sh_get_size()
{
    return this->Size;
}

inline Vector<SliceIndex>& Heap::sh_get_freechunks()
{
    return this->FreeChunks;
}

inline Pool<SliceIndex>& Heap::sh_get_allocated_chunks()
{
    return this->AllocatedChunks;
}

inline HeapA::AllocationState Heap::allocate_element(const uimax p_size, HeapA::AllocatedElementReturn* out_chunk)
{
    return HeapA::allocate_element(*this, p_size, out_chunk);
}

inline HeapA::AllocationState Heap::allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
{
    return HeapA::allocate_element_with_modulo_offset(*this, p_size, p_modulo_offset, out_chunk);
}

inline HeapA::AllocationState Heap::allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, HeapA::AllocatedElementReturn* out_chunk)
{
    return HeapA::allocate_element_norealloc_with_modulo_offset(*this, p_size, p_modulo_offset, out_chunk);
}

inline SliceIndex* Heap::get(const Token(SliceIndex) p_chunk)
{
    return &this->AllocatedChunks.get(p_chunk);
}

inline void Heap::release_element(const Token(SliceIndex) p_chunk)
{
    this->FreeChunks.push_back_element(this->AllocatedChunks.get(p_chunk));
    this->AllocatedChunks.release_element(p_chunk);
}

inline HeapA::AllocationState Heap::reallocate_element(const Token(SliceIndex) p_chunk, const uimax p_new_size, HeapA::AllocatedElementReturn* out_chunk)
{
    HeapA::AllocationState l_allocation = this->allocate_element(p_new_size, out_chunk);
    if ((HeapA::AllocationState_t)l_allocation & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED)
    {
        this->release_element(p_chunk);
    }
    return l_allocation;
}

inline HeapPaged HeapPaged::allocate_default(const uimax p_page_size)
{
    return HeapPaged{Pool<SliceIndex>::allocate(0), VectorOfVector<SliceIndex>::allocate_default(), p_page_size};
}

inline void HeapPaged::free()
{
    this->AllocatedChunks.free();
    this->FreeChunks.free();
}

inline uimax HeapPaged::get_page_count()
{
    return this->FreeChunks.varying_vector.get_size();
}

inline HeapPaged::AllocationState HeapPaged::allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk)
{
    HeapA::AllocatedElementReturn l_heap_allocated_element_return;

    for (loop(i, 0, this->FreeChunks.varying_vector.get_size()))
    {
        SingleShadowHeap l_single_shadow_heap = SingleShadowHeap::build(this, i);
        if ((HeapA::AllocationState_t)HeapA::allocate_element_norealloc_with_modulo_offset(l_single_shadow_heap, p_size, p_modulo_offset, &l_heap_allocated_element_return) &
            (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED)
        {
            *out_chunk = AllocatedElementReturn::buid_from_HeapAllocatedElementReturn(i, l_heap_allocated_element_return);
            return AllocationState::ALLOCATED;
        };
    }

    this->create_new_page();

    SingleShadowHeap l_single_shadow_heap = SingleShadowHeap::build(this, this->FreeChunks.varying_vector.get_size() - 1);
    if ((HeapA::AllocationState_t)HeapA::allocate_element_norealloc_with_modulo_offset(l_single_shadow_heap, p_size, p_modulo_offset, &l_heap_allocated_element_return) &
        (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED)
    {
        *out_chunk = AllocatedElementReturn::buid_from_HeapAllocatedElementReturn(this->FreeChunks.varying_vector.get_size() - 1, l_heap_allocated_element_return);
        return (AllocationState)((AllocationState_t)AllocationState::ALLOCATED | (AllocationState_t)AllocationState::PAGE_CREATED);
    };

    return AllocationState::NOT_ALLOCATED;
}

inline void HeapPaged::release_element(const HeapPagedToken& p_token)
{
    this->FreeChunks.element_push_back_element(p_token.PageIndex, this->AllocatedChunks.get(p_token.token));
    this->AllocatedChunks.release_element(p_token.token);
}

inline SliceIndex* HeapPaged::get_sliceindex_only(const HeapPagedToken& p_token)
{
    return &this->AllocatedChunks.get(p_token.token);
}

inline void HeapPaged::create_new_page()
{
    SliceIndex l_chunk_slice = SliceIndex::build(0, this->PageSize);
    this->FreeChunks.push_back_element(Slice<SliceIndex>::build_memory_elementnb(&l_chunk_slice, 1));
}
