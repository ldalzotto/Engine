#pragma once


namespace v2
{
	struct HeapPagedGPU
	{
		struct MemoryGPU
		{
			gcmemory_t gpu_memory;
			int8* mapped_memory;

			static MemoryGPU allocate(const gc_t p_transfer_device, const uint32 p_memory_type, const uimax p_memory_size);
			void free(const gc_t p_transfer_device);
			void map(const gc_t p_transfer_device, const uimax p_memory_size);
		};

		HeapPaged heap;
		Vector<MemoryGPU> gpu_memories;
		uint32 memory_type;

		static HeapPagedGPU allocate_default(const gc_t p_transfer_device, const uimax p_memory_chunk_size, const uint32 p_memory_type);
		void free(const gc_t p_transfer_device);

		int8 allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, HeapPagedToken* out_chunk);

		/*
			Same as allocate_element, but if a new page is allocated, MemoryGPU::map is called.
		*/
		int8 allocate_element_and_map_page_if_created(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, HeapPagedToken* out_chunk);
		void release_element(const HeapPagedToken& p_token);
	};

	/*
		Holds HeapPagedGPU based on the memory_type value.
		Vulkan allows to allocate memory based on their type.
	*/
	struct TransferDeviceHeap
	{
		// Memory that is visible by the GPU only.
		HeapPagedGPU gpu_write;

		// Memory that can be accessed by the CPU.
		HeapPagedGPU host_write;

		static TransferDeviceHeap allocate_default(const gc_t p_transfer_device);
		void free(const gc_t p_transfer_device);

		int8 allocate_gpu_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, HeapPagedToken* out_token);
		void release_gpu_write_element(const HeapPagedToken& p_memory);
		int8 allocate_host_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, HeapPagedToken* out_token);
		void release_host_write_element(const HeapPagedToken& p_memory);

		Slice<int8> get_host_write_element_as_slice(const HeapPagedToken& p_token);

		SliceOffset<int8> get_host_write_gcmemory_and_offset(const HeapPagedToken& p_token);
		SliceOffset<int8> get_gpu_write_gcmemory_and_offset(const HeapPagedToken& p_token);
	};

	/*
		A TransferDevice is a logical instance of the GraphicsCard.
		It holds the command buffer that will execute all commands related to memory allocation or copy
	*/
	struct TransferDevice
	{
		gc_t device;
		gcqueue_t transfer_queue;

		TransferDeviceHeap heap;
		CommandPool command_pool;
		CommandBuffer command_buffer;

		static TransferDevice allocate(const GPUInstance& p_instance);
		void free();

	};

	/*
		A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the CPU.
	*/
	struct BufferHost
	{
		struct MappedMemory
		{
			Slice<int8> memory;

			static MappedMemory build_default();

			void map(TransferDevice& p_transfer_device, const HeapPagedToken& p_memory);
			void unmap(TransferDevice& p_device);
			void copy_from(const Slice<int8>& p_from);
			int8 is_mapped();
		} memory;
		HeapPagedToken heap_token;
		VkBuffer buffer;
		uimax size;

		static BufferHost allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const VkBufferUsageFlags p_usage_flags);
		void free(TransferDevice& p_transfer_device);

		void push(const Slice<int8>& p_from);

		Slice<int8>& get_mapped_memory();

	private:
		void bind(TransferDevice& p_transfer_device);
		void map(TransferDevice& p_transfer_device, const uimax p_element_count);
		void unmap(TransferDevice& p_transfer_device);
	};

	/*
		A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the GPU.
	*/
	struct BufferGPU
	{
		HeapPagedToken heap_token;
		VkBuffer buffer;
		uimax size;

		static BufferGPU allocate(TransferDevice& p_transfer_device, const uimax p_size, const VkBufferUsageFlags p_usage_flags);
		void free(TransferDevice& p_transfer_device);

	private:
		void bind(TransferDevice& p_transfer_device);
	};


	enum class BufferUsageFlag
	{
		READ = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		WRITE = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT
	};

	typedef VkFlags BufferUsageFlags;

	/*
		The BufferAllocator is the front layer that register and execute all allocation, copy of buffers.
	*/
	struct BufferAllocator
	{
		TransferDevice device;

		Pool<BufferHost> host_buffers;
		Pool<BufferGPU> gpu_buffers;

		Vector<Token(BufferHost)> garbage_host_buffers;

		struct HosttoGPU_CopyEvent
		{
			Token(BufferHost) staging_buffer;
			Token(BufferGPU) target_buffer;
		};

		Vector<HosttoGPU_CopyEvent> host_to_gpu_copy_events;

		struct GPUtoHost_CopyEvent
		{
			Token(BufferGPU) source_buffer;
			Token(BufferHost) target_buffer;
		};

		Vector<GPUtoHost_CopyEvent> gpu_to_host_copy_events;

		static BufferAllocator allocate_default(const GPUInstance& p_instance);
		void free();

		Token(BufferHost) allocate_bufferhost(const Slice<int8>& p_value, const VkBufferUsageFlags p_usage_flags);
		Token(BufferHost) allocate_bufferhost_empty(const uimax p_size, const VkBufferUsageFlags p_usage_flags);
		void free_bufferhost(const Token(BufferHost) p_buffer_host);
		Slice<int8>& get_bufferhost_mapped_memory(const Token(BufferHost) p_buffer_host);

		Token(BufferGPU) allocate_buffergpu(const uimax p_size, const VkBufferUsageFlags p_usage_flags);
		void free_buffergpu(const Token(BufferGPU) p_buffer_gpu);
		void write_to_buffergpu(const Token(BufferGPU) p_buffer_gpu, const Slice<int8>& p_value);
		Token(BufferHost) read_from_buffergpu(const Token(BufferGPU) p_buffer_gpu);

		void step();
		void force_command_buffer_execution_sync();

	private:
		void clean_garbage_buffers();
	};

};


namespace v2
{

	inline HeapPagedGPU::MemoryGPU HeapPagedGPU::MemoryGPU::allocate(const gc_t p_transfer_device, const uint32 p_memory_type, const uimax p_memory_size)
	{
		MemoryGPU l_heap_gpu;

		VkMemoryAllocateInfo l_memoryallocate_info{};
		l_memoryallocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		l_memoryallocate_info.allocationSize = p_memory_size;
		l_memoryallocate_info.memoryTypeIndex = p_memory_type;

		vkAllocateMemory(p_transfer_device, &l_memoryallocate_info, NULL, &l_heap_gpu.gpu_memory);

		l_heap_gpu.mapped_memory = NULL;

		return l_heap_gpu;
	};

	inline void HeapPagedGPU::MemoryGPU::free(const gc_t p_transfer_device)
	{
		vkFreeMemory(p_transfer_device, this->gpu_memory, NULL);
	};

	inline void HeapPagedGPU::MemoryGPU::map(const gc_t p_transfer_device, const uimax p_memory_size)
	{
		vk_handle_result(vkMapMemory(p_transfer_device, this->gpu_memory, 0, p_memory_size, 0, (void**)&this->mapped_memory));
	};

	inline HeapPagedGPU HeapPagedGPU::allocate_default(const gc_t p_transfer_device, const uimax p_memory_chunk_size, const uint32 p_memory_type)
	{
		return HeapPagedGPU
		{
			HeapPaged::allocate_default(p_memory_chunk_size),
			Vector<MemoryGPU>::allocate(0),
			p_memory_type
		};
	};

	inline void HeapPagedGPU::free(const gc_t p_transfer_device)
	{
		for (loop(i, 0, this->gpu_memories.Size))
		{
			this->gpu_memories.get(i).free(p_transfer_device);
		}
		this->gpu_memories.free();
		this->heap.free();
	};

	inline int8 HeapPagedGPU::allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, HeapPagedToken* out_chunk)
	{
		HeapPaged::AllocatedElementReturn l_allocated_chunk;
		HeapPaged::AllocationState l_allocation_state = this->heap.allocate_element_norealloc_with_alignment(p_size, p_alignmenent_constaint, &l_allocated_chunk);
		if ((HeapPaged::AllocationState_t)l_allocation_state & (HeapPaged::AllocationState_t)HeapPaged::AllocationState::PAGE_CREATED)
		{
			this->gpu_memories.push_back_element(MemoryGPU::allocate(p_transfer_device, this->memory_type, this->heap.PageSize));
		}

		*out_chunk = l_allocated_chunk.token;

		return (HeapPaged::AllocationState_t)l_allocation_state & (HeapPaged::AllocationState_t)HeapPaged::AllocationState::ALLOCATED;
	};

	inline int8 HeapPagedGPU::allocate_element_and_map_page_if_created(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, HeapPagedToken* out_chunk)
	{
		HeapPaged::AllocatedElementReturn l_allocated_chunk;
		HeapPaged::AllocationState l_allocation_state = this->heap.allocate_element_norealloc_with_alignment(p_size, p_alignmenent_constaint, &l_allocated_chunk);
		if ((HeapPaged::AllocationState_t)l_allocation_state & (HeapPaged::AllocationState_t)HeapPaged::AllocationState::PAGE_CREATED)
		{
			this->gpu_memories.push_back_element(MemoryGPU::allocate(p_transfer_device, this->memory_type, this->heap.PageSize));
			this->gpu_memories.get(l_allocated_chunk.token.PageIndex).map(p_transfer_device, this->heap.PageSize);
		}

		*out_chunk = l_allocated_chunk.token;

		return (HeapPaged::AllocationState_t)l_allocation_state & (HeapPaged::AllocationState_t)HeapPaged::AllocationState::ALLOCATED;
	};

	inline void HeapPagedGPU::release_element(const HeapPagedToken& p_token)
	{
		this->heap.release_element(p_token);
	};

	inline TransferDeviceHeap TransferDeviceHeap::allocate_default(const gc_t p_transfer_device)
	{
		return TransferDeviceHeap{
			HeapPagedGPU::allocate_default(p_transfer_device,16000000, 7),
			HeapPagedGPU::allocate_default(p_transfer_device,16000000, 8)
		};
	};

	inline void TransferDeviceHeap::free(const gc_t p_transfer_device)
	{
		this->gpu_write.free(p_transfer_device);
		this->host_write.free(p_transfer_device);
	};

	inline int8 TransferDeviceHeap::allocate_gpu_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, HeapPagedToken* out_token)
	{
		return this->gpu_write.allocate_element(p_size, p_alignement_constraint, p_transfer_device, out_token);
	};

	inline void TransferDeviceHeap::release_gpu_write_element(const HeapPagedToken& p_memory)
	{
		this->gpu_write.release_element(p_memory);
	};

	inline int8 TransferDeviceHeap::allocate_host_write_element(const gc_t p_transfer_device, const uimax p_size, const uimax p_alignement_constraint, HeapPagedToken* out_token)
	{
		return this->host_write.allocate_element_and_map_page_if_created(p_size, p_alignement_constraint, p_transfer_device, out_token);
	};

	inline void TransferDeviceHeap::release_host_write_element(const HeapPagedToken& p_memory)
	{
		this->host_write.release_element(p_memory);
	};

	inline Slice<int8> TransferDeviceHeap::get_host_write_element_as_slice(const HeapPagedToken& p_token)
	{
		SliceIndex* l_slice_index = this->host_write.heap.get_sliceindex_only(p_token);
		return Slice<int8>::build_memory_offset_elementnb(
			this->host_write.gpu_memories.get(p_token.PageIndex).mapped_memory,
			l_slice_index->Begin, l_slice_index->Size
		);
	};

	inline SliceOffset<int8> TransferDeviceHeap::get_host_write_gcmemory_and_offset(const HeapPagedToken& p_token)
	{
		return SliceOffset<int8>::build_from_sliceindex(
			(int8*)this->host_write.gpu_memories.get(p_token.PageIndex).gpu_memory,
			*this->host_write.heap.get_sliceindex_only(p_token)
		);
	};

	inline SliceOffset<int8> TransferDeviceHeap::get_gpu_write_gcmemory_and_offset(const HeapPagedToken& p_token)
	{
		return SliceOffset<int8>::build_from_sliceindex(
			(int8*)this->gpu_write.gpu_memories.get(p_token.PageIndex).gpu_memory,
			*this->gpu_write.heap.get_sliceindex_only(p_token)
		);
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

		l_transfer_device.heap = TransferDeviceHeap::allocate_default(l_transfer_device.device);

		l_transfer_device.command_pool = CommandPool::allocate(l_transfer_device.device, p_instance.graphics_card.transfer_queue_family);
		l_transfer_device.command_buffer = l_transfer_device.command_pool.allocate_command_buffer(l_transfer_device.device, l_transfer_device.transfer_queue);

		return l_transfer_device;
	};

	inline void TransferDevice::free()
	{
		this->heap.free(this->device);
		this->command_buffer.flush();
		this->command_pool.free(this->device);
		vkDestroyDevice(this->device, NULL);
	};

	inline BufferHost::MappedMemory BufferHost::MappedMemory::build_default()
	{
		return MappedMemory{
			Slice<int8>::build(NULL, 0,0)
		};
	};

	inline void BufferHost::MappedMemory::map(TransferDevice& p_transfer_device, const HeapPagedToken& p_memory)
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


	inline BufferHost BufferHost::allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const VkBufferUsageFlags p_usage_flags)
	{
		BufferHost l_buffer_host;

		l_buffer_host.size = p_buffer_size;
		l_buffer_host.memory = MappedMemory::build_default();

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = p_usage_flags;
		l_buffercreate_info.size = p_buffer_size;

		vk_handle_result(vkCreateBuffer(p_transfer_device.device, &l_buffercreate_info, NULL, &l_buffer_host.buffer));

		VkMemoryRequirements l_requirements;
		vkGetBufferMemoryRequirements(p_transfer_device.device, l_buffer_host.buffer, &l_requirements);

		p_transfer_device.heap.allocate_host_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_buffer_host.heap_token);
		l_buffer_host.memory.map(p_transfer_device, l_buffer_host.heap_token);
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

	inline Slice<int8>& BufferHost::get_mapped_memory()
	{
		return this->memory.memory;
	};

	inline void BufferHost::bind(TransferDevice& p_transfer_device)
	{
#if RENDER_BOUND_TEST
		assert_true(this->memory.is_mapped());
#endif
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_host_write_gcmemory_and_offset(this->heap_token);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline void BufferHost::map(TransferDevice& p_transfer_device, const uimax p_element_count)
	{
		this->memory.map(p_transfer_device, this->heap_token);
	};

	inline void BufferHost::unmap(TransferDevice& p_transfer_device)
	{
		this->memory.unmap(p_transfer_device);
	};


	inline BufferGPU BufferGPU::allocate(TransferDevice& p_transfer_device, const uimax p_size, const VkBufferUsageFlags p_usage_flags)
	{
		BufferGPU l_buffer_gpu;

		l_buffer_gpu.size = p_size;

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = p_usage_flags;
		l_buffercreate_info.size = p_size;

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
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_gpu_write_gcmemory_and_offset(this->heap_token);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline BufferAllocator BufferAllocator::allocate_default(const GPUInstance& p_instance)
	{
		return BufferAllocator{
			TransferDevice::allocate(p_instance),
			Pool<BufferHost>::allocate(0),
			Pool<BufferGPU>::allocate(0),
			Vector<Token(BufferHost)>::allocate(0),
			Vector<HosttoGPU_CopyEvent>::allocate(0)
		};
	};

	inline void BufferAllocator::free()
	{
		this->step();
		this->force_command_buffer_execution_sync();

		this->clean_garbage_buffers();

		this->host_buffers.free();
		this->device.free();
	};

	inline Token(BufferHost) BufferAllocator::allocate_bufferhost(const Slice<int8>& p_value, const VkBufferUsageFlags p_usage_flags)
	{
		BufferHost l_buffer = BufferHost::allocate(this->device, p_value.Size, p_usage_flags);
		l_buffer.push(p_value);
		return this->host_buffers.alloc_element(l_buffer);
	};

	inline Token(BufferHost) BufferAllocator::allocate_bufferhost_empty(const uimax p_size, const VkBufferUsageFlags p_usage_flags)
	{
		return this->host_buffers.alloc_element(BufferHost::allocate(this->device, p_size, p_usage_flags));
	};

	inline void BufferAllocator::free_bufferhost(const Token(BufferHost) p_buffer_host)
	{
		BufferHost& l_buffer = this->host_buffers.get(p_buffer_host);
		l_buffer.free(this->device);
		this->host_buffers.release_element(p_buffer_host);
	};

	inline Slice<int8>& BufferAllocator::get_bufferhost_mapped_memory(const Token(BufferHost) p_buffer_host)
	{
		return this->host_buffers.get(p_buffer_host).get_mapped_memory();
	};

	inline Token(BufferGPU) BufferAllocator::allocate_buffergpu(const uimax p_size, const VkBufferUsageFlags p_usage_flags)
	{
		return this->gpu_buffers.alloc_element(BufferGPU::allocate(this->device, p_size, p_usage_flags));
	};

	inline void BufferAllocator::free_buffergpu(const Token(BufferGPU) p_buffer_gpu)
	{
		for (vector_loop_reverse(&this->gpu_to_host_copy_events, i))
		{
			GPUtoHost_CopyEvent& l_event = this->gpu_to_host_copy_events.get(i);
			if (tk_eq(l_event.source_buffer, p_buffer_gpu))
			{
				this->garbage_host_buffers.push_back_element(l_event.target_buffer);
				this->gpu_to_host_copy_events.erase_element_at(i);
			}
		}

		for (vector_loop_reverse(&this->host_to_gpu_copy_events, i))
		{
			HosttoGPU_CopyEvent& l_event = this->host_to_gpu_copy_events.get(i);
			if (tk_eq(l_event.target_buffer, p_buffer_gpu))
			{
				this->garbage_host_buffers.push_back_element(l_event.staging_buffer);
				this->host_to_gpu_copy_events.erase_element_at(i);
			}
		}

		BufferGPU& l_buffer = this->gpu_buffers.get(p_buffer_gpu);
		l_buffer.free(this->device);
		this->gpu_buffers.release_element(p_buffer_gpu);
	};

	inline void BufferAllocator::write_to_buffergpu(const Token(BufferGPU) p_buffer_gpu, const Slice<int8>& p_value)
	{
		Token(BufferHost) l_staging_buffer = this->allocate_bufferhost(p_value, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		this->host_to_gpu_copy_events.push_back_element(HosttoGPU_CopyEvent{ l_staging_buffer, p_buffer_gpu });
	};

	inline Token(BufferHost) BufferAllocator::read_from_buffergpu(const Token(BufferGPU) p_buffer_gpu)
	{
		BufferGPU& l_buffer_gpu = this->gpu_buffers.get(p_buffer_gpu);
		Token(BufferHost) l_staging_buffer = this->allocate_bufferhost_empty(l_buffer_gpu.size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		this->gpu_to_host_copy_events.push_back_element(GPUtoHost_CopyEvent{ p_buffer_gpu, l_staging_buffer });
		return l_staging_buffer;
	};


	inline void BufferAllocator::step()
	{
		this->clean_garbage_buffers();

		this->device.command_buffer.begin();

		if (this->host_to_gpu_copy_events.Size > 0)
		{
			for (loop(i, 0, this->host_to_gpu_copy_events.Size))
			{
				HosttoGPU_CopyEvent& l_event = this->host_to_gpu_copy_events.get(i);
				BufferHost& l_source_buffer = this->host_buffers.get(l_event.staging_buffer);
				BufferGPU& l_target_buffer = this->gpu_buffers.get(l_event.target_buffer);

#if CONTAINER_MEMORY_TEST
				assert_true(l_source_buffer.size <= l_target_buffer.size);
#endif

				VkBufferCopy l_buffer_copy{};
				l_buffer_copy.size = l_source_buffer.size;
				vkCmdCopyBuffer(this->device.command_buffer.command_buffer,
					l_source_buffer.buffer,
					l_target_buffer.buffer,
					1,
					&l_buffer_copy
				);

				this->garbage_host_buffers.push_back_element(l_event.staging_buffer);
			}

			this->host_to_gpu_copy_events.clear();
		}

		if (this->gpu_to_host_copy_events.Size > 0)
		{

			for (loop(i, 0, this->gpu_to_host_copy_events.Size))
			{
				GPUtoHost_CopyEvent& l_event = this->gpu_to_host_copy_events.get(i);
				BufferGPU& l_source_buffer = this->gpu_buffers.get(l_event.source_buffer);
				BufferHost& l_target_buffer = this->host_buffers.get(l_event.target_buffer);

#if CONTAINER_MEMORY_TEST
				assert_true(l_source_buffer.size <= l_target_buffer.size);
#endif

				VkBufferCopy l_buffer_copy{};
				l_buffer_copy.size = l_source_buffer.size;
				vkCmdCopyBuffer(this->device.command_buffer.command_buffer,
					l_source_buffer.buffer,
					l_target_buffer.buffer,
					1,
					&l_buffer_copy
				);

			}

			this->gpu_to_host_copy_events.clear();
		}


		this->device.command_buffer.end();
	};

	inline void BufferAllocator::force_command_buffer_execution_sync()
	{
		this->device.command_buffer.submit();
		this->device.command_buffer.wait_for_completion();
	};

	inline void BufferAllocator::clean_garbage_buffers()
	{
		for (loop(i, 0, this->garbage_host_buffers.Size))
		{
			this->free_bufferhost(this->garbage_host_buffers.get(i));
		}

		this->garbage_host_buffers.clear();
	};


}
