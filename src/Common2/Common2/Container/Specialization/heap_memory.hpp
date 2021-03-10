#pragma once

/*
    A HeapMemory is a Heap where chunks are allocated on the CPU.
*/
struct HeapMemory
{
    Heap _Heap;
    Span<int8> Memory;

    inline static HeapMemory allocate(const uimax p_heap_size)
    {
        return HeapMemory{Heap::allocate(p_heap_size), Span<int8>::allocate(p_heap_size)};
    };

    inline static HeapMemory allocate_default()
    {
        return HeapMemory::allocate(0);
    };

    inline void free()
    {
        this->_Heap.free();
        this->Memory.free();
    };

    inline TokenT(SliceIndex) allocate_element(const Slice<int8>* p_element_bytes)
    {
        HeapA::AllocatedElementReturn l_heap_allocated_element;
        this->handle_heap_allocation_state(this->_Heap.allocate_element(p_element_bytes->Size, &l_heap_allocated_element));
        this->Memory.slice.copy_memory(l_heap_allocated_element.Offset, *p_element_bytes);
        return l_heap_allocated_element.token;
    };

    inline TokenT(SliceIndex) allocate_empty_element(const uimax p_element_size)
    {
        HeapA::AllocatedElementReturn l_heap_allocated_element;
        this->handle_heap_allocation_state(this->_Heap.allocate_element(p_element_size, &l_heap_allocated_element));
        return l_heap_allocated_element.token;
    };

    inline TokenT(SliceIndex) allocate_empty_element_return_chunk(const uimax p_element_size, Slice<int8>* out_chunk)
    {
        HeapA::AllocatedElementReturn l_heap_allocated_element;
        this->handle_heap_allocation_state(this->_Heap.allocate_element(p_element_size, &l_heap_allocated_element));
        *out_chunk = Slice<int8>::build(&this->Memory.Memory[l_heap_allocated_element.Offset], p_element_size);
        return l_heap_allocated_element.token;
    };

    inline TokenT(SliceIndex) allocate_element(const Slice<int8> p_element_bytes)
    {
        return this->allocate_element(&p_element_bytes);
    };

    template <class ELementType> inline TokenT(SliceIndex) allocate_element_typed(const ELementType* p_element)
    {
        return this->allocate_element(Slice<ELementType>::build_asint8_memory_singleelement(p_element));
    };

    template <class ELementType> inline TokenT(SliceIndex) allocate_element_typed(const ELementType p_element)
    {
        return this->allocate_element_typed(&p_element);
    };

    inline void release_element(const TokenT(SliceIndex) p_chunk)
    {
        this->_Heap.release_element(p_chunk);
    };

    inline Slice<int8> get(const TokenT(SliceIndex) p_chunk)
    {
        SliceIndex* l_chunk_slice = this->_Heap.get(p_chunk);
        return Slice<int8>::build_memory_offset_elementnb(this->Memory.Memory, l_chunk_slice->Begin, l_chunk_slice->Size);
    };

    template <class ElementType> inline ElementType* get_typed(const TokenT(SliceIndex) p_chunk)
    {
        return slice_cast_singleelement<ElementType>(this->get(p_chunk));
    };

  private:
    void handle_heap_allocation_state(const HeapA::AllocationState p_allocation_state)
    {
        if ((HeapA::AllocationState_t)p_allocation_state & (HeapA::AllocationState_t)HeapA::AllocationState::HEAP_RESIZED)
        {
#if CONTAINER_MEMORY_TEST
            if (!this->Memory.resize(this->_Heap.Size))
            {
                abort();
            }
#else
            this->Memory.resize(this->_Heap.Size);
#endif
        };

#if CONTAINER_MEMORY_TEST
        if (!((HeapA::AllocationState_t)p_allocation_state & (HeapA::AllocationState_t)HeapA::AllocationState::ALLOCATED))
        {
            abort();
        }
#endif
    };
};