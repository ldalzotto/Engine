#pragma once


namespace v2
{
	enum GPUMemoryType
	{
		// * HostWrite : Allow cpu memory map
		HostWrite = 0,
		// * GPUWrite : Allow writing by copying from a source buffer.
		//              Cannot be accesses from from host.
		GPUWrite = 1
	};

	struct HeapGPU
	{
		Heap heap;
		gcmemory_t gpu_memory;
		int8* mapped_memory;

		static HeapGPU allocate(const gc_t p_transfer_device, const uint32 p_memory_type, const uimax p_memory_size);

		void free(const gc_t p_transfer_device);

		void map(gc_t p_transfer_device, const uimax p_memory_size);

		int8 allocate_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, Token(SliceIndex)* out_token);
		void release_element(const Token(SliceIndex) p_token);
	};

	struct GPUMemoryToken
	{
		uint8 pagedbuffer_index;
		Token(SliceIndex) chunk;
	};

	//TODO -> this can moved to the common module so that the paged heap logic can be reused anywhere
	struct GraphicsCardPagedHeap
	{
		Vector<HeapGPU> page_buffers;
		uimax chunk_size;

		static GraphicsCardPagedHeap allocate(const uimax p_chunk_size);
		void free(const gc_t p_transfer_device);

		int8 allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, GPUMemoryToken* out_chunk);
		int8 allocate_element_and_map_heapmemory_if_created(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, GPUMemoryToken* out_chunk);

		void release_element(const GPUMemoryToken& p_token);
	};

	struct GraphicsCardHeap
	{
		GraphicsCardPagedHeap gpu_write;
		GraphicsCardPagedHeap host_write;

		static GraphicsCardHeap allocate_default(const gc_t p_transfer_device);
		void free(const gc_t p_transfer_device);

		int8 allocate_gpu_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token);
		void release_gpu_write_element(const GPUMemoryToken& p_memory);
		int8 allocate_host_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token);
		void release_host_write_element(const GPUMemoryToken& p_memory);

		SliceIndex* get_gpu_write_element(const GPUMemoryToken& p_token);
		SliceIndex* get_host_write_element(const GPUMemoryToken& p_token);
		Slice<int8> get_host_write_element_as_slice(const GPUMemoryToken& p_token);

		//TODO -> creating a SliceOffset type ?
		void get_host_write_gcmemory_and_offset(const GPUMemoryToken& p_token, gcmemory_t* out_gc_memory, SliceIndex* out_offset);
		void get_gpu_write_gcmemory_and_offset(const GPUMemoryToken& p_token, gcmemory_t* out_gc_memory, SliceIndex* out_offset);

		int8 allocate_element_always(const gc_t p_transfer_device, const GPUMemoryType p_memory_type, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token);
		void release_element_always(const GPUMemoryType p_memory_type, const GPUMemoryToken& p_memory);
	};

	struct TransferDevice
	{
		gc_t device;
		gcqueue_t transfer_queue;

		GraphicsCardHeap heap;

		static TransferDevice allocate(const GPUInstance& p_instance);
		void free();

	};

	struct BufferHost
	{
		struct MappedMemory
		{
			Slice<int8> memory;

			static MappedMemory build_default();

			void map(TransferDevice& p_transfer_device, const GPUMemoryToken& p_memory, const uimax p_element_count);
			void unmap(TransferDevice& p_device);
			void copy_from(const Slice<int8>& p_from);
			int8 is_mapped();
		} memory;
		GPUMemoryToken heap_token;
		VkBuffer buffer;

		static BufferHost allocate(TransferDevice& p_transfer_device, const uimax p_element_count, const uimax p_element_size, const VkBufferUsageFlags p_usage_flags);
		void free(TransferDevice& p_transfer_device);

		void push(const Slice<int8>& p_from);

	private:
		void bind(TransferDevice& p_transfer_device);
		void map(TransferDevice& p_transfer_device, const uimax p_element_count);
		void unmap(TransferDevice& p_transfer_device);
	};

	struct BufferGPU
	{
		GPUMemoryToken heap_token;
		VkBuffer buffer;

		static BufferGPU allocate(TransferDevice& p_transfer_device, const uimax p_element_count, const uimax p_element_size, const VkBufferUsageFlags p_usage_flags);
		void free(TransferDevice& p_transfer_device);

	private:
		void bind(TransferDevice& p_transfer_device);
	};

}





namespace v2
{
	inline HeapGPU HeapGPU::allocate(const gc_t p_transfer_device, const uint32 p_memory_type, const uimax p_memory_size)
	{
		HeapGPU l_heap_gpu;
		l_heap_gpu.heap = Heap::allocate(p_memory_size);

		VkMemoryAllocateInfo l_memoryallocate_info{};
		l_memoryallocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		l_memoryallocate_info.allocationSize = p_memory_size;
		l_memoryallocate_info.memoryTypeIndex = p_memory_type;

		vkAllocateMemory(p_transfer_device, &l_memoryallocate_info, NULL, &l_heap_gpu.gpu_memory);

		return l_heap_gpu;
	};

	inline void HeapGPU::free(const gc_t p_transfer_device)
	{
		vkFreeMemory(p_transfer_device, this->gpu_memory, NULL);
	};

	inline void HeapGPU::map(gc_t p_transfer_device, const uimax p_memory_size)
	{
		vkMapMemory(p_transfer_device, this->gpu_memory, 0, p_memory_size, 0, (void**)&this->mapped_memory);
	};

	inline int8 HeapGPU::allocate_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, Token(SliceIndex)* out_token)
	{
		Heap::AllocatedElementReturn l_allocation;
		Heap::AllocationState l_allocation_state = this->heap.allocate_element_norealloc_with_alignment(p_size, p_alignement_constraint, &l_allocation);
		if ((Heap::AllocationState_t)l_allocation_state & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED)
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
			Vector<HeapGPU>::allocate(0),
			p_chunk_size
		};
	};

	inline void GraphicsCardPagedHeap::free(const gc_t p_transfer_device)
	{
		for (loop(i, 0, this->page_buffers.Size))
		{
			this->page_buffers.get(i).free(p_transfer_device);
		}
		this->page_buffers.free();
	};

	inline int8 GraphicsCardPagedHeap::allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, GPUMemoryToken* out_chunk)
	{
		for (loop(i, 0, this->page_buffers.Size))
		{
			if (this->page_buffers.get(i).allocate_element(p_transfer_device, p_size, p_alignmenent_constaint, &out_chunk->chunk))
			{
				out_chunk->pagedbuffer_index = (uint8)i;
				return 1;
			}
		}

		HeapGPU l_created_heap = HeapGPU::allocate(p_transfer_device, 8, this->chunk_size);

		// l_created_heap.map(p_transfer_device, this->chunk_size);

		this->page_buffers.push_back_element(l_created_heap);
		int8 l_success = this->page_buffers.get(this->page_buffers.Size - 1).allocate_element(p_transfer_device, p_size, p_alignmenent_constaint, &out_chunk->chunk);
		out_chunk->pagedbuffer_index = (uint8)this->page_buffers.Size - 1;
		return l_success;
	};

	inline int8 GraphicsCardPagedHeap::allocate_element_and_map_heapmemory_if_created(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, GPUMemoryToken* out_chunk)
	{
		for (loop(i, 0, this->page_buffers.Size))
		{
			if (this->page_buffers.get(i).allocate_element(p_transfer_device, p_size, p_alignmenent_constaint, &out_chunk->chunk))
			{
				out_chunk->pagedbuffer_index = (uint8)i;
				return 1;
			}
		}

		HeapGPU l_created_heap = HeapGPU::allocate(p_transfer_device, 8, this->chunk_size);

		l_created_heap.map(p_transfer_device, this->chunk_size);

		this->page_buffers.push_back_element(l_created_heap);
		int8 l_success = this->page_buffers.get(this->page_buffers.Size - 1).allocate_element(p_transfer_device, p_size, p_alignmenent_constaint, &out_chunk->chunk);
		out_chunk->pagedbuffer_index = (uint8)this->page_buffers.Size - 1;
		return l_success;
	};

	inline void GraphicsCardPagedHeap::release_element(const GPUMemoryToken& p_token)
	{
		this->page_buffers.get(p_token.pagedbuffer_index).release_element(p_token.chunk);
	};

	inline GraphicsCardHeap GraphicsCardHeap::allocate_default(const gc_t p_transfer_device)
	{
		return GraphicsCardHeap{
			GraphicsCardPagedHeap::allocate(16000000),
			GraphicsCardPagedHeap::allocate(16000000)
		};
	};

	inline void GraphicsCardHeap::free(const gc_t p_transfer_device)
	{
		this->gpu_write.free(p_transfer_device);
		this->host_write.free(p_transfer_device);
	};

	inline int8 GraphicsCardHeap::allocate_gpu_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token)
	{
		return this->gpu_write.allocate_element(p_size, p_alignement_constraint, p_transfer_device, out_token);
	};

	inline void GraphicsCardHeap::release_gpu_write_element(const GPUMemoryToken& p_memory)
	{
		this->gpu_write.release_element(p_memory);
	};

	inline int8 GraphicsCardHeap::allocate_host_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token)
	{
		return this->host_write.allocate_element_and_map_heapmemory_if_created(p_size, p_alignement_constraint, p_transfer_device, out_token);
	};

	inline void GraphicsCardHeap::release_host_write_element(const GPUMemoryToken& p_memory)
	{
		this->host_write.release_element(p_memory);
	};

	inline SliceIndex* GraphicsCardHeap::get_gpu_write_element(const GPUMemoryToken& p_token)
	{
		return this->gpu_write.page_buffers.get(p_token.pagedbuffer_index).heap.get(p_token.chunk);
	};

	inline SliceIndex* GraphicsCardHeap::get_host_write_element(const GPUMemoryToken& p_token)
	{
		return this->host_write.page_buffers.get(p_token.pagedbuffer_index).heap.get(p_token.chunk);
	};

	inline Slice<int8> GraphicsCardHeap::get_host_write_element_as_slice(const GPUMemoryToken& p_token)
	{
		HeapGPU& l_heap = this->host_write.page_buffers.get(p_token.pagedbuffer_index);
		SliceIndex* l_slice_index = l_heap.heap.get(p_token.chunk);
		return Slice<int8>::build_memory_offset_elementnb(l_heap.mapped_memory, l_slice_index->Begin, l_slice_index->Size);
	};

	inline void GraphicsCardHeap::get_host_write_gcmemory_and_offset(const GPUMemoryToken& p_token, gcmemory_t* out_gc_memory, SliceIndex* out_offset)
	{
		HeapGPU& l_heap = this->host_write.page_buffers.get(p_token.pagedbuffer_index);
		*out_gc_memory = l_heap.gpu_memory;
		*out_offset = *l_heap.heap.get(p_token.chunk);
	};

	inline void GraphicsCardHeap::get_gpu_write_gcmemory_and_offset(const GPUMemoryToken& p_token, gcmemory_t* out_gc_memory, SliceIndex* out_offset)
	{
		HeapGPU& l_heap = this->gpu_write.page_buffers.get(p_token.pagedbuffer_index);
		*out_gc_memory = l_heap.gpu_memory;
		*out_offset = *l_heap.heap.get(p_token.chunk);
	};

	inline int8 GraphicsCardHeap::allocate_element_always(const gc_t p_transfer_device, const GPUMemoryType p_memory_type, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token)
	{
		switch (p_memory_type)
		{
		case GPUMemoryType::GPUWrite:
			return allocate_gpu_write_element(p_transfer_device, p_size, p_alignement_constraint, out_token);
		case GPUMemoryType::HostWrite:
			return allocate_host_write_element(p_transfer_device, p_size, p_alignement_constraint, out_token);
		}
	};

	inline void GraphicsCardHeap::release_element_always(const GPUMemoryType p_memory_type, const GPUMemoryToken& p_memory)
	{
		switch (p_memory_type)
		{
		case GPUMemoryType::GPUWrite:
			return release_gpu_write_element(p_memory);
			break;
		case GPUMemoryType::HostWrite:
			return release_host_write_element(p_memory);
			break;
		}
	};




	inline TransferDevice TransferDevice::allocate(const GPUInstance& p_instance)
	{
		TransferDevice l_transfer_device;

		VkDeviceQueueCreateInfo l_devicequeue_create_info{};
		l_devicequeue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		l_devicequeue_create_info.queueFamilyIndex = p_instance.graphics_card.transfer_queue_family;
		l_devicequeue_create_info.queueCount = 1;
		const float32 l_priority = 1.0f;
		l_devicequeue_create_info.pQueuePriorities = &l_priority;

		// VkPhysicalDeviceFeatures l_devicefeatures;
		VkDeviceCreateInfo l_device_create_info{};
		l_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		l_device_create_info.pQueueCreateInfos = &l_devicequeue_create_info;
		l_device_create_info.queueCreateInfoCount = 1;


#if RENDER_DEBUG

		int8* l_validation_layers_str[1];
		l_validation_layers_str[0] = "VK_LAYER_KHRONOS_validation";
		Slice<const int8*> l_validation_layers = Slice<const int8*>::build_memory_elementnb((const int8**)l_validation_layers_str, 1);


		l_device_create_info.enabledLayerCount = (uint32)l_validation_layers.Size;
		l_device_create_info.ppEnabledLayerNames = l_validation_layers.Begin;
#endif

		l_device_create_info.enabledExtensionCount = 0;

		vk_handle_result(vkCreateDevice(p_instance.graphics_card.device, &l_device_create_info, NULL, &l_transfer_device.device));

		vkGetDeviceQueue(l_transfer_device.device, p_instance.graphics_card.transfer_queue_family, 0, &l_transfer_device.transfer_queue);

		l_transfer_device.heap = GraphicsCardHeap::allocate_default(l_transfer_device.device);

		return l_transfer_device;
	};

	inline void TransferDevice::free()
	{
		this->heap.free(this->device);
		vkDestroyDevice(this->device, NULL);
	};

	inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		String l_severity = String::allocate(100);

		int8 is_error = 0;

		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			l_severity.append(slice_int8_build_rawstr("[Verbose] - "));
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			l_severity.append(slice_int8_build_rawstr("[Warn] - "));
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			l_severity.append(slice_int8_build_rawstr("[Error] - "));
			is_error = true;
			break;
		}
		l_severity.append(slice_int8_build_rawstr("validation layer: "));
		l_severity.append(slice_int8_build_rawstr(pCallbackData->pMessage));
		l_severity.append(slice_int8_build_rawstr("\n"));
		printf(l_severity.get_memory());

		l_severity.free();

		if (is_error)
		{
			abort();
		}

		return VK_FALSE;
	};



	inline BufferHost::MappedMemory BufferHost::MappedMemory::build_default()
	{
		return MappedMemory{
			Slice<int8>::build(NULL, 0,0)
		};
	};

	inline void BufferHost::MappedMemory::map(TransferDevice& p_transfer_device, const GPUMemoryToken& p_memory, const uimax p_element_count)
	{
		if (!this->is_mapped())
		{
			this->memory = p_transfer_device.heap.get_host_write_element_as_slice(p_memory);
		}
	};

	inline void BufferHost::MappedMemory::unmap(TransferDevice& p_device)
	{
		if (this->is_mapped())
		{
			*this = MappedMemory::build_default();
		}
	};

	inline void BufferHost::MappedMemory::copy_from(const Slice<int8>& p_from)
	{
		slice_memcpy(this->memory, p_from);
	};

	inline int8 BufferHost::MappedMemory::is_mapped()
	{
		return this->memory.Begin != NULL;
	};


	inline BufferHost BufferHost::allocate(TransferDevice& p_transfer_device, const uimax p_element_count, const uimax p_element_size, const VkBufferUsageFlags p_usage_flags)
	{
		BufferHost l_buffer_host;

		l_buffer_host.memory = MappedMemory::build_default();

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = p_usage_flags | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		l_buffercreate_info.size = p_element_count * p_element_size;

		vk_handle_result(vkCreateBuffer(p_transfer_device.device, &l_buffercreate_info, NULL, &l_buffer_host.buffer));

		VkMemoryRequirements l_requirements;
		vkGetBufferMemoryRequirements(p_transfer_device.device, l_buffer_host.buffer, &l_requirements);

		p_transfer_device.heap.allocate_host_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_buffer_host.heap_token);
		// p_transfer_device.heap.allocate_element_always(p_transfer_device.device, GPUMemoryType::HostWrite, l_requirements.size, l_requirements.alignment, &l_buffer_host.heap_token);
		l_buffer_host.memory.map(p_transfer_device, l_buffer_host.heap_token, p_element_count);
		l_buffer_host.bind(p_transfer_device);

		return l_buffer_host;
	};

	inline void BufferHost::free(TransferDevice& p_transfer_device)
	{
		if (this->memory.is_mapped())
		{
			this->unmap(p_transfer_device);
		}

		p_transfer_device.heap.release_host_write_element(this->heap_token);
		vkDestroyBuffer(p_transfer_device.device, this->buffer, NULL);
		this->buffer = NULL;
	};

	inline void BufferHost::push(const Slice<int8>& p_from)
	{
		this->memory.copy_from(p_from);
	};


	inline void BufferHost::bind(TransferDevice& p_transfer_device)
	{
#if RENDER_BOUND_TEST
		assert_true(this->memory.is_mapped());
#endif
		gcmemory_t l_memory;
		SliceIndex l_offset;
		p_transfer_device.heap.get_host_write_gcmemory_and_offset(this->heap_token, &l_memory, &l_offset);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, l_memory, l_offset.Begin);
	};

	inline void BufferHost::map(TransferDevice& p_transfer_device, const uimax p_element_count)
	{
		this->memory.map(p_transfer_device, this->heap_token, p_element_count);
	};

	inline void BufferHost::unmap(TransferDevice& p_transfer_device)
	{
		this->memory.unmap(p_transfer_device);
	};


	inline BufferGPU BufferGPU::allocate(TransferDevice& p_transfer_device, const uimax p_element_count, const uimax p_element_size, const VkBufferUsageFlags p_usage_flags)
	{
		BufferGPU l_buffer_gpu;

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = p_usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		l_buffercreate_info.size = p_element_count * p_element_size;

		vk_handle_result(vkCreateBuffer(p_transfer_device.device, &l_buffercreate_info, NULL, &l_buffer_gpu.buffer));

		VkMemoryRequirements l_requirements;
		vkGetBufferMemoryRequirements(p_transfer_device.device, l_buffer_gpu.buffer, &l_requirements);

		p_transfer_device.heap.allocate_gpu_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_buffer_gpu.heap_token);
		l_buffer_gpu.bind(p_transfer_device);

		return l_buffer_gpu;
	};

	inline void BufferGPU::free(TransferDevice& p_transfer_device)
	{
		p_transfer_device.heap.release_gpu_write_element(this->heap_token);
		vkDestroyBuffer(p_transfer_device.device, this->buffer, NULL);
		this->buffer = NULL;
	};

	inline void BufferGPU::bind(TransferDevice& p_transfer_device)
	{
		gcmemory_t l_memory;
		SliceIndex l_offset;
		p_transfer_device.heap.get_gpu_write_gcmemory_and_offset(this->heap_token, &l_memory, &l_offset);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, l_memory, l_offset.Begin);
	};


}
