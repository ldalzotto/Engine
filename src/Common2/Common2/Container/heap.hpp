#pragma once

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

    using _FreeChunksValue = Vector<SliceIndex>;
    using _FreeChunks = _FreeChunksValue&;

    using _AllocatedChunksValue = Pool<SliceIndex>;
    using _AllocatedChunks = _AllocatedChunksValue&;

    inline static Heap allocate(const uimax p_heap_size)
    {
        Heap l_heap = Heap{Pool<SliceIndex>::allocate(0), Vector<SliceIndex>::allocate(1), p_heap_size};
        l_heap.FreeChunks.push_back_element(SliceIndex::build(0, p_heap_size));
        return l_heap;
    };

    inline void free()
    {
        this->AllocatedChunks.free();
        this->FreeChunks.free();
        this->Size = 0;
    };

    inline void resize(const uimax p_newsize)
    {
        uimax l_old_size = this->Size;
        this->FreeChunks.push_back_element(SliceIndex::build(l_old_size, p_newsize - l_old_size));
        this->Size = p_newsize;
    };

    inline uimax get_size()
    {
        return this->Size;
    };

    inline Vector<SliceIndex>& get_freechunks()
    {
        return this->FreeChunks;
    };

    inline Pool<SliceIndex>& get_allocated_chunks()
    {
        return this->AllocatedChunks;
    };

    inline iHeap<Heap> to_iheap()
    {
        return iHeap<Heap>{*this};
    };

    inline iHeapTypes::AllocationState allocate_element(const uimax p_size, iHeapTypes::AllocatedElementReturn* out_chunk)
    {
        return this->to_iheap().allocate_element(p_size, out_chunk);
    };

    inline iHeapTypes::AllocationState allocate_element_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, iHeapTypes::AllocatedElementReturn* out_chunk)
    {
        return this->to_iheap().allocate_element_with_modulo_offset(p_size, p_modulo_offset, out_chunk);
    };

    inline iHeapTypes::AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, iHeapTypes::AllocatedElementReturn* out_chunk)
    {
        return this->to_iheap().allocate_element_norealloc_with_modulo_offset(p_size, p_modulo_offset, out_chunk);
    };

    inline SliceIndex* get(const Token<SliceIndex> p_chunk)
    {
        return &this->AllocatedChunks.get(p_chunk);
    };

    inline void release_element(const Token<SliceIndex> p_chunk)
    {
        this->FreeChunks.push_back_element(this->AllocatedChunks.get(p_chunk));
        this->AllocatedChunks.release_element(p_chunk);
    };
};

struct HeapPagedToken
{
    uimax PageIndex;
    Token<SliceIndex> token;
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

        inline static AllocatedElementReturn buid_from_HeapAllocatedElementReturn(const uimax p_page_index, const iHeapTypes::AllocatedElementReturn& p_heap_allocated_element_return)
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
    struct Single_iHeap
    {
        HeapPaged* heapPaged;
        uimax index;
        VectorOfVector<SliceIndex>::Element_iVector tmp_shadow_vec;

        using _FreeChunksValue = VectorOfVector<SliceIndex>::Element_iVector;
        using _FreeChunks = _FreeChunksValue&;
        using _AllocatedChunksValue = Pool<SliceIndex>;
        using _AllocatedChunks = _AllocatedChunksValue&;

        inline static Single_iHeap build(HeapPaged* p_heap_paged, const uimax p_index)
        {
            Single_iHeap l_iheap;
            l_iheap.heapPaged = p_heap_paged;
            l_iheap.index = p_index;
            return l_iheap;
        };

        inline VectorOfVector<SliceIndex>::Element_iVector& get_freechunks()
        {
            this->tmp_shadow_vec = this->heapPaged->FreeChunks.element_as_iVector(this->index);
            return tmp_shadow_vec;
        };

        inline Pool<SliceIndex>& get_allocated_chunks()
        {
            return this->heapPaged->AllocatedChunks;
        };

        inline iHeap<Single_iHeap> to_iheap()
        {
            return iHeap<Single_iHeap>{*this};
        };
    };

    inline static HeapPaged allocate_default(const uimax p_page_size)
    {
        return HeapPaged{Pool<SliceIndex>::allocate(0), VectorOfVector<SliceIndex>::allocate_default(), p_page_size};
    };

    inline void free()
    {
        this->AllocatedChunks.free();
        this->FreeChunks.free();
    };

    inline uimax get_page_count()
    {
        return this->FreeChunks.varying_vector.get_size();
    };

    inline AllocationState allocate_element_norealloc_with_modulo_offset(const uimax p_size, const uimax p_modulo_offset, AllocatedElementReturn* out_chunk)
    {
        iHeapTypes::AllocatedElementReturn l_heap_allocated_element_return;

        for (loop(i, 0, this->FreeChunks.varying_vector.get_size()))
        {
            Single_iHeap l_single_iheap = Single_iHeap::build(this, i);
            if ((iHeapTypes::AllocationState_t)l_single_iheap.to_iheap().allocate_element_norealloc_with_modulo_offset( p_size, p_modulo_offset, &l_heap_allocated_element_return) &
                (iHeapTypes::AllocationState_t)iHeapTypes::AllocationState::ALLOCATED)
            {
                *out_chunk = AllocatedElementReturn::buid_from_HeapAllocatedElementReturn(i, l_heap_allocated_element_return);
                return AllocationState::ALLOCATED;
            };
        }

        this->create_new_page();

        Single_iHeap l_single_iheap = Single_iHeap::build(this, this->FreeChunks.varying_vector.get_size() - 1);
        if ((iHeapTypes::AllocationState_t)l_single_iheap.to_iheap().allocate_element_norealloc_with_modulo_offset(p_size, p_modulo_offset, &l_heap_allocated_element_return) &
            (iHeapTypes::AllocationState_t)iHeapTypes::AllocationState::ALLOCATED)
        {
            *out_chunk = AllocatedElementReturn::buid_from_HeapAllocatedElementReturn(this->FreeChunks.varying_vector.get_size() - 1, l_heap_allocated_element_return);
            return (AllocationState)((AllocationState_t)AllocationState::ALLOCATED | (AllocationState_t)AllocationState::PAGE_CREATED);
        };

        return AllocationState::NOT_ALLOCATED;
    };

    inline void release_element(const HeapPagedToken& p_token)
    {
        this->FreeChunks.element_push_back_element(p_token.PageIndex, this->AllocatedChunks.get(p_token.token));
        this->AllocatedChunks.release_element(p_token.token);
    };

    inline SliceIndex* get_sliceindex_only(const HeapPagedToken& p_token)
    {
        return &this->AllocatedChunks.get(p_token.token);
    };

  private:
    inline void create_new_page()
    {
        SliceIndex l_chunk_slice = SliceIndex::build(0, this->PageSize);
        this->FreeChunks.push_back_element(Slice<SliceIndex>::build_memory_elementnb(&l_chunk_slice, 1));
    };
};
