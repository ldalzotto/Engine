#pragma once


inline HeapGPU HeapGPU::allocate(const gc_t p_gc, const uint32 p_memory_type, const uimax p_memory_size)
{
	HeapGPU l_heap_gpu;
	l_heap_gpu.heap = v2::Heap::allocate(p_memory_size);

	VkMemoryAllocateInfo l_memoryallocate_info{};
	l_memoryallocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	l_memoryallocate_info.allocationSize = p_memory_size;
	l_memoryallocate_info.memoryTypeIndex = p_memory_type;

	vkAllocateMemory(p_gc, &l_memoryallocate_info, NULL, &l_heap_gpu.memory);

	return l_heap_gpu;
};

inline void HeapGPU::free(const gc_t p_gc)
{
	vkFreeMemory(p_gc, this->memory, NULL);
};

inline int8 HeapGPU::allocate_element(const gc_t p_gc, const uimax p_size, const uimax p_alignement_constraint, Token(SliceIndex)* out_token)
{
	v2::Heap::AllocatedElementReturn l_allocation;
	v2::Heap::AllocationState l_allocation_state = this->heap.allocate_element_norealloc_with_alignment(p_size, p_alignement_constraint, &l_allocation);
	if ((v2::Heap::AllocationState_t)l_allocation_state & (v2::Heap::AllocationState_t)v2::Heap::AllocationState::ALLOCATED)
	{
		*out_token = l_allocation.token;
		return 1;
	};

	return 0;
};

inline void HeapGPU::release_element(const Token(SliceIndex) p_token)
{
	this->heap.release_element(p_token);
};



inline GraphicsCardPagedHeap GraphicsCardPagedHeap::allocate(const uimax p_chunk_size)
{
	return GraphicsCardPagedHeap{
		v2::Vector<HeapGPU>::allocate(0),
		p_chunk_size
	};
};

inline void GraphicsCardPagedHeap::free(const gc_t p_gc)
{
	for (loop(i, 0, this->page_buffers.Size))
	{
		this->page_buffers.get(i).free(p_gc);
	}
	this->page_buffers.free();
};

inline int8 GraphicsCardPagedHeap::allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, const uint32 p_memory_type, gc_t p_gc, GPUMemoryToken* out_chunk)
{
	for (loop(i, 0, this->page_buffers.Size))
	{
		if (this->page_buffers.get(i).allocate_element(p_gc, p_size, p_alignmenent_constaint, &out_chunk->chunk))
		{
			out_chunk->pagedbuffer_index = (uint8)i;
			return 1;
		}
	}

	this->page_buffers.push_back_element(HeapGPU::allocate(p_gc, p_memory_type, this->chunk_size));
	int8 l_success = this->page_buffers.get(this->page_buffers.Size - 1).allocate_element(p_gc, p_size, p_alignmenent_constaint, &out_chunk->chunk);
	out_chunk->pagedbuffer_index = (uint8)this->page_buffers.Size - 1;
	return l_success;
};

inline void GraphicsCardPagedHeap::release_element(const GPUMemoryToken& p_token)
{
	this->page_buffers.get(p_token.pagedbuffer_index).release_element(p_token.chunk);
};

inline GraphicsCardHeap GraphicsCardHeap::allocate_default(const gc_t p_gc)
{
	return GraphicsCardHeap{
		GraphicsCardPagedHeap::allocate(16000000),
		GraphicsCardPagedHeap::allocate(16000000)
	};
};

inline void GraphicsCardHeap::free(const gc_t p_gc)
{
	this->gpu_write.free(p_gc);
	this->host_write.free(p_gc);
};

inline int8 GraphicsCardHeap::allocate_gpu_write_element(const gc_t p_gc, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token)
{
	return this->gpu_write.allocate_element(p_size, p_alignement_constraint, 7, p_gc, out_token);
};

inline void GraphicsCardHeap::release_gpu_write_element(const GPUMemoryToken& p_memory)
{
	this->gpu_write.release_element(p_memory);
};

inline int8 GraphicsCardHeap::allocate_host_write_element(const gc_t p_gc, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token)
{
	return this->host_write.allocate_element(p_size, p_alignement_constraint, 8, p_gc, out_token);
};

inline void GraphicsCardHeap::release_host_write_element(const GPUMemoryToken& p_memory)
{
	this->host_write.release_element(p_memory);
};

inline int8 GraphicsCardHeap::allocate_element_always(const gc_t p_gc, const uint32 p_memory_type, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token)
{
	switch (p_memory_type)
	{
	case 7:
		return allocate_gpu_write_element(p_gc, p_size, p_alignement_constraint, out_token);
	case 8:
		return allocate_host_write_element(p_gc, p_size, p_alignement_constraint, out_token);
	}
};

inline void GraphicsCardHeap::release_element_always(const uint32 p_memory_type, const GPUMemoryToken& p_memory)
{
	switch (p_memory_type)
	{
	case 7:
		return release_gpu_write_element(p_memory);
		break;
	case 8:
		return release_host_write_element(p_memory);
		break;
	}
};