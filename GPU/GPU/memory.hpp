#pragma once


namespace v2
{
	struct HeapPagedGPU
	{
		struct MemoryGPU
		{
			gcmemory_t gpu_memory;
			int8* mapped_memory;

			static MemoryGPU allocate(const gc_t p_transfer_device, const uint32 p_memory_type_index, const uimax p_memory_size);
			void free(const gc_t p_transfer_device);
			void map(const gc_t p_transfer_device, const uimax p_memory_size);
		};

		HeapPaged heap;
		Vector<MemoryGPU> gpu_memories;
		int8 is_memory_mapped;

		static HeapPagedGPU allocate_default(const int8 is_memory_mapped, const gc_t p_transfer_device, const uimax p_memory_chunk_size);
		void free(const gc_t p_transfer_device);

		int8 allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, const uint32 p_memory_type_index, HeapPagedToken* out_chunk);
		void release_element(const HeapPagedToken& p_token);
	};

	struct TransferDeviceHeapToken
	{
		uint32 heap_index;
		HeapPagedToken heap_paged_token;
	};

	/*
		Holds HeapPagedGPU based on the memory_type value.
		Vulkan allows to allocate memory based on their type.
	*/
	struct TransferDeviceHeap
	{
		Span<HeapPagedGPU> gpu_heaps;

		static TransferDeviceHeap allocate_default(const GraphicsCard& p_graphics_card, const gc_t p_transfer_device);
		void free(const gc_t p_transfer_device);

		int8 allocate_element(const GraphicsCard& p_graphics_card, const gc_t p_transfer_device, const VkMemoryRequirements& p_requirements,
			const VkMemoryPropertyFlags p_memory_property_flags, TransferDeviceHeapToken* out_token);
		void release_element(const TransferDeviceHeapToken& p_memory);

		Slice<int8> get_element_as_slice(const TransferDeviceHeapToken& p_token);
		SliceOffset<int8> get_element_gcmemory_and_offset(const TransferDeviceHeapToken& p_token);
	};

	/*
		A TransferDevice is a logical instance of the GraphicsCard.
		It holds the command buffer that will execute all commands related to memory allocation or copy
	*/
	struct TransferDevice
	{
		GraphicsCard graphics_card;
		gc_t device;
		gcqueue_t transfer_queue;

		TransferDeviceHeap heap;
		CommandPool command_pool;
		CommandBuffer command_buffer;

		static TransferDevice allocate(const GPUInstance& p_instance);
		void free();

	};

	struct MappedHostMemory
	{
		Slice<int8> memory;

		static MappedHostMemory build_default();

		void map(TransferDevice& p_transfer_device, const TransferDeviceHeapToken& p_memory);
		void unmap(TransferDevice& p_device);
		void copy_from(const Slice<int8>& p_from);
		int8 is_mapped();
	};

	/*
		A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the CPU.
	*/
	struct BufferHost
	{
		MappedHostMemory memory;
		TransferDeviceHeapToken heap_token;
		VkBuffer buffer;
		uimax size;

		static BufferHost allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const VkBufferUsageFlags p_usage_flags);
		void free(TransferDevice& p_transfer_device);

		void push(const Slice<int8>& p_from);

		Slice<int8>& get_mapped_memory();

	private:
		void bind(TransferDevice& p_transfer_device);
		void map(TransferDevice& p_transfer_device);
		void unmap(TransferDevice& p_transfer_device);
	};

	/*
		A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the GPU.
	*/
	struct BufferGPU
	{
		TransferDeviceHeapToken heap_token;
		VkBuffer buffer;
		uimax size;

		static BufferGPU allocate(TransferDevice& p_transfer_device, const uimax p_size, const VkBufferUsageFlags p_usage_flags);
		void free(TransferDevice& p_transfer_device);

	private:
		void bind(TransferDevice& p_transfer_device);
	};

	struct ImageFormat
	{
		VkImageAspectFlags imageAspect;
		VkImageType imageType;
		VkFormat format;
		v3ui extent;
		uint32 mipLevels;
		uint32 arrayLayers;
		VkSampleCountFlagBits samples;
	};

	struct ImageHost
	{
		MappedHostMemory memory;
		TransferDeviceHeapToken heap_token;
		VkImage image;
		ImageFormat format;

		static ImageHost allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format,
			const VkImageUsageFlags p_image_usage, const VkImageLayout p_initial_layout);
		void free(TransferDevice& p_transfer_device);

		void push(const Slice<int8>& p_from);
		Slice<int8>& get_mapped_memory();

	private:
		void map(TransferDevice& p_transfer_device);
		void unmap(TransferDevice& p_transfer_device);
		void bind(TransferDevice& p_transfer_device);
	};

	struct ImageGPU
	{
		TransferDeviceHeapToken heap_token;
		VkImage image;
		ImageFormat format;

		static ImageGPU allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format,
			const VkImageUsageFlags p_image_usage, const VkImageLayout p_initial_layout);
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
		Pool<ImageHost> host_images;
		Pool<ImageGPU> gpu_images;

		Vector<Token(BufferHost)> garbage_host_buffers;
		Vector<Token(ImageHost)> garbage_host_images;

		struct Buffer_HosttoGPU_CopyEvent
		{
			Token(BufferHost) staging_buffer;
			Token(BufferGPU) target_buffer;
		};

		Vector<Buffer_HosttoGPU_CopyEvent> buffer_host_to_gpu_copy_events;

		struct Buffer_GPUtoHost_CopyEvent
		{
			Token(BufferGPU) source_buffer;
			Token(BufferHost) target_buffer;
		};

		Vector<Buffer_GPUtoHost_CopyEvent> buffer_gpu_to_host_copy_events;

		struct Image_HosttoGPU_CopyEvent
		{
			Token(ImageHost) staging_image;
			Token(ImageGPU) target_image;
		};

		Vector<Image_HosttoGPU_CopyEvent> image_host_to_gpu_copy_events;

		struct ImageGPUtoHost_CopyEvent
		{
			Token(ImageGPU) source_image;
			Token(ImageHost) target_image;
		};

		Vector<ImageGPUtoHost_CopyEvent> image_gpu_to_host_copy_events;

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

		Token(ImageHost) allocate_imagehost(const Slice<int8>& p_value, const ImageFormat& p_image_format, const VkImageUsageFlags p_usage_flags);
		Token(ImageHost) allocate_imagehost_empty(const ImageFormat& p_image_format, const VkImageUsageFlags p_usage_flags);
		void free_imagehost(const Token(ImageHost) p_image_host);
		Slice<int8>& get_imagehost_mapped_memory(const Token(ImageHost) p_image_host);

		Token(ImageGPU) allocate_imagegpu(const ImageFormat& p_image_format, const VkImageUsageFlags p_usage_flags);
		void free_imagegpu(const Token(ImageGPU) p_image_gpu);
		void write_to_imagegpu(const Token(ImageGPU) p_image_gpu, const Slice<int8>& p_value /*, const VkImageLayout p_before_layout, const VkImageLayout p_after_layout*/);
		Token(ImageHost) read_from_imagegpu(const Token(ImageGPU) p_image_gpu);

		void step();
		void force_command_buffer_execution_sync();

	private:
		void clean_garbage_buffers();
	};

};


namespace v2
{

	inline HeapPagedGPU::MemoryGPU HeapPagedGPU::MemoryGPU::allocate(const gc_t p_transfer_device, const uint32 p_memory_type_index, const uimax p_memory_size)
	{
		MemoryGPU l_heap_gpu;

		VkMemoryAllocateInfo l_memoryallocate_info{};
		l_memoryallocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		l_memoryallocate_info.allocationSize = p_memory_size;
		l_memoryallocate_info.memoryTypeIndex = p_memory_type_index;

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

	inline HeapPagedGPU HeapPagedGPU::allocate_default(const int8 is_memory_mapped, const gc_t p_transfer_device, const uimax p_memory_chunk_size)
	{
		return HeapPagedGPU
		{
			HeapPaged::allocate_default(p_memory_chunk_size),
			Vector<MemoryGPU>::allocate(0),
			is_memory_mapped
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

	inline int8 HeapPagedGPU::allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, gc_t p_transfer_device, const uint32 p_memory_type_index, HeapPagedToken* out_chunk)
	{
		HeapPaged::AllocatedElementReturn l_allocated_chunk;
		HeapPaged::AllocationState l_allocation_state = this->heap.allocate_element_norealloc_with_alignment(p_size, p_alignmenent_constaint, &l_allocated_chunk);
		if ((HeapPaged::AllocationState_t)l_allocation_state & (HeapPaged::AllocationState_t)HeapPaged::AllocationState::PAGE_CREATED)
		{
			this->gpu_memories.push_back_element(MemoryGPU::allocate(p_transfer_device, p_memory_type_index, this->heap.PageSize));
			if (this->is_memory_mapped)
			{
				this->gpu_memories.get(l_allocated_chunk.token.PageIndex).map(p_transfer_device, this->heap.PageSize);
			}
		}

		*out_chunk = l_allocated_chunk.token;

		return (HeapPaged::AllocationState_t)l_allocation_state & (HeapPaged::AllocationState_t)HeapPaged::AllocationState::ALLOCATED;
	};

	inline void HeapPagedGPU::release_element(const HeapPagedToken& p_token)
	{
		this->heap.release_element(p_token);
	};

	inline TransferDeviceHeap TransferDeviceHeap::allocate_default(const GraphicsCard& p_graphics_card, const gc_t p_transfer_device)
	{
		TransferDeviceHeap l_heap = TransferDeviceHeap{ Span<HeapPagedGPU>::allocate(p_graphics_card.device_memory_properties.memoryHeapCount) };
		for (loop(i, 0, p_graphics_card.device_memory_properties.memoryHeapCount))
		{
			int8 l_is_memory_mapped = 0;
			for (loop(j, 0, p_graphics_card.device_memory_properties.memoryTypeCount))
			{
				const VkMemoryType* l_memory_type = &p_graphics_card.device_memory_properties.memoryTypes[j];
				if (l_memory_type->heapIndex == i)
				{
					if ((l_memory_type->propertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
						|| (l_memory_type->propertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
						|| (l_memory_type->propertyFlags & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
					{
						l_is_memory_mapped = 1;
						break;
					}
				}

			}

			l_heap.gpu_heaps.get(i) = HeapPagedGPU::allocate_default(l_is_memory_mapped, p_transfer_device, 16000000);

		}
		return l_heap;
	};

	inline void TransferDeviceHeap::free(const gc_t p_transfer_device)
	{
		for (loop(i, 0, this->gpu_heaps.Capacity))
		{
			this->gpu_heaps.get(i).free(p_transfer_device);
		};
		this->gpu_heaps.free();
	};


	inline int8 TransferDeviceHeap::allocate_element(const GraphicsCard& p_graphics_card, const gc_t p_transfer_device, const VkMemoryRequirements& p_requirements,
		const VkMemoryPropertyFlags p_memory_property_flags, TransferDeviceHeapToken* out_token)
	{
		uint32 l_memory_type_index = p_graphics_card.get_memory_type_index(p_requirements, p_memory_property_flags);
		out_token->heap_index = p_graphics_card.device_memory_properties.memoryTypes[l_memory_type_index].heapIndex;
		return this->gpu_heaps.get(out_token->heap_index).allocate_element(p_requirements.size, p_requirements.alignment, p_transfer_device, l_memory_type_index, &out_token->heap_paged_token);
	};

	inline void TransferDeviceHeap::release_element(const TransferDeviceHeapToken& p_memory)
	{
		this->gpu_heaps.get(p_memory.heap_index).release_element(p_memory.heap_paged_token);
	};

	inline Slice<int8> TransferDeviceHeap::get_element_as_slice(const TransferDeviceHeapToken& p_token)
	{
		HeapPagedGPU& l_heap = this->gpu_heaps.get(p_token.heap_index);
		SliceIndex* l_slice_index = l_heap.heap.get_sliceindex_only(p_token.heap_paged_token);
		return Slice<int8>::build_memory_offset_elementnb(
			l_heap.gpu_memories.get(p_token.heap_paged_token.PageIndex).mapped_memory,
			l_slice_index->Begin, l_slice_index->Size
		);
	};

	inline SliceOffset<int8> TransferDeviceHeap::get_element_gcmemory_and_offset(const TransferDeviceHeapToken& p_token)
	{
		HeapPagedGPU& l_heap = this->gpu_heaps.get(p_token.heap_index);
		return SliceOffset<int8>::build_from_sliceindex(
			(int8*)l_heap.gpu_memories.get(p_token.heap_paged_token.PageIndex).gpu_memory,
			*l_heap.heap.get_sliceindex_only(p_token.heap_paged_token)
		);
	};


	inline TransferDevice TransferDevice::allocate(const GPUInstance& p_instance)
	{
		TransferDevice l_transfer_device;
		l_transfer_device.graphics_card = p_instance.graphics_card;

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

		l_transfer_device.heap = TransferDeviceHeap::allocate_default(p_instance.graphics_card, l_transfer_device.device);

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

	inline MappedHostMemory MappedHostMemory::build_default()
	{
		return MappedHostMemory{
			Slice<int8>::build(NULL, 0,0)
		};
	};

	inline void MappedHostMemory::map(TransferDevice& p_transfer_device, const TransferDeviceHeapToken& p_memory)
	{
		if (!this->is_mapped())
		{
			this->memory = p_transfer_device.heap.get_element_as_slice(p_memory);
		}
	};

	inline void MappedHostMemory::unmap(TransferDevice& p_device)
	{
		if (this->is_mapped())
		{
			*this = MappedHostMemory::build_default();
		}
	};

	inline void MappedHostMemory::copy_from(const Slice<int8>& p_from)
	{
		slice_memcpy(this->memory, p_from);
	};

	inline int8 MappedHostMemory::is_mapped()
	{
		return this->memory.Begin != NULL;
	};


	inline BufferHost BufferHost::allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const VkBufferUsageFlags p_usage_flags)
	{
		BufferHost l_buffer_host;

		l_buffer_host.size = p_buffer_size;
		l_buffer_host.memory = MappedHostMemory::build_default();

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = p_usage_flags;
		l_buffercreate_info.size = p_buffer_size;

		vk_handle_result(vkCreateBuffer(p_transfer_device.device, &l_buffercreate_info, NULL, &l_buffer_host.buffer));

		VkMemoryRequirements l_requirements;
		vkGetBufferMemoryRequirements(p_transfer_device.device, l_buffer_host.buffer, &l_requirements);

		p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_requirements, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &l_buffer_host.heap_token);
		// p_transfer_device.heap.allocate_host_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_buffer_host.heap_token);
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

		p_transfer_device.heap.release_element(this->heap_token);
		// p_transfer_device.heap.release_host_write_element(this->heap_token);
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
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		// SliceOffset<int8> l_memory = p_transfer_device.heap.get_host_write_gcmemory_and_offset(this->heap_token);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline void BufferHost::map(TransferDevice& p_transfer_device)
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

		p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_requirements, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &l_buffer_gpu.heap_token);
		// p_transfer_device.heap.allocate_gpu_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_buffer_gpu.heap_token);
		l_buffer_gpu.bind(p_transfer_device);

		return l_buffer_gpu;
	};

	inline void BufferGPU::free(TransferDevice& p_transfer_device)
	{
		p_transfer_device.heap.release_element(this->heap_token);
		// p_transfer_device.heap.release_gpu_write_element(this->heap_token);
		vkDestroyBuffer(p_transfer_device.device, this->buffer, NULL);
		this->buffer = NULL;
	};

	inline void BufferGPU::bind(TransferDevice& p_transfer_device)
	{
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		// SliceOffset<int8> l_memory = p_transfer_device.heap.get_gpu_write_gcmemory_and_offset(this->heap_token);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline ImageHost ImageHost::allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format,
		const VkImageUsageFlags p_image_usage, const VkImageLayout p_initial_layout)
	{
		ImageHost l_image_host;
		l_image_host.memory = MappedHostMemory::build_default();
		l_image_host.format = p_image_format;

		VkImageCreateInfo l_image_create_info{};
		l_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		l_image_create_info.imageType = p_image_format.imageType;
		l_image_create_info.format = p_image_format.format;
		l_image_create_info.extent = VkExtent3D{ p_image_format.extent.x, p_image_format.extent.y, p_image_format.extent.z };
		l_image_create_info.mipLevels = p_image_format.mipLevels;
		l_image_create_info.arrayLayers = p_image_format.arrayLayers;
		l_image_create_info.samples = p_image_format.samples;
		l_image_create_info.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR; //This is mandatory for host readable image
		l_image_create_info.usage = p_image_usage;
		l_image_create_info.initialLayout = p_initial_layout;

		vk_handle_result(vkCreateImage(p_transfer_device.device, &l_image_create_info, NULL, &l_image_host.image));


		VkMemoryRequirements l_requirements;
		vkGetImageMemoryRequirements(p_transfer_device.device, l_image_host.image, &l_requirements);

		p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_requirements,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &l_image_host.heap_token);
		// p_transfer_device.heap.allocate_host_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_image_host.heap_token);
		l_image_host.memory.map(p_transfer_device, l_image_host.heap_token);

		l_image_host.map(p_transfer_device);
		l_image_host.bind(p_transfer_device);

		return l_image_host;
	};

	inline void ImageHost::free(TransferDevice& p_transfer_device)
	{
		if (this->memory.is_mapped())
		{
			this->unmap(p_transfer_device);
		};

		vkDestroyImage(p_transfer_device.device, this->image, NULL);
		p_transfer_device.heap.release_element(this->heap_token);
		// p_transfer_device.heap.release_host_write_element(this->heap_token);
	};

	inline void ImageHost::push(const Slice<int8>& p_from)
	{
		slice_memcpy(this->memory.memory, p_from);
	};

	inline Slice<int8>& ImageHost::get_mapped_memory()
	{
		return this->memory.memory;
	};

	inline void ImageHost::map(TransferDevice& p_transfer_device)
	{
		this->memory.map(p_transfer_device, this->heap_token);
	};

	inline void ImageHost::unmap(TransferDevice& p_transfer_device)
	{
		this->memory.unmap(p_transfer_device);
	};

	inline void ImageHost::bind(TransferDevice& p_transfer_device)
	{
#if RENDER_BOUND_TEST
		assert_true(this->memory.is_mapped());
#endif
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		// SliceOffset<int8> l_memory = p_transfer_device.heap.get_host_write_gcmemory_and_offset(this->heap_token);
		vkBindImageMemory(p_transfer_device.device, this->image, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline ImageGPU ImageGPU::allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format,
		const VkImageUsageFlags p_image_usage, const VkImageLayout p_initial_layout)
	{
		ImageGPU l_image_gpu;
		l_image_gpu.format = p_image_format;

		VkImageCreateInfo l_image_create_info{};
		l_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		l_image_create_info.imageType = p_image_format.imageType;
		l_image_create_info.format = p_image_format.format;
		l_image_create_info.extent = VkExtent3D{ p_image_format.extent.x, p_image_format.extent.y, p_image_format.extent.z };
		l_image_create_info.mipLevels = p_image_format.mipLevels;
		l_image_create_info.arrayLayers = p_image_format.arrayLayers;
		l_image_create_info.samples = p_image_format.samples;
		l_image_create_info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		l_image_create_info.usage = p_image_usage;
		l_image_create_info.initialLayout = p_initial_layout;

		vk_handle_result(vkCreateImage(p_transfer_device.device, &l_image_create_info, NULL, &l_image_gpu.image));

		VkMemoryRequirements l_requirements;
		vkGetImageMemoryRequirements(p_transfer_device.device, l_image_gpu.image, &l_requirements);

		p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_requirements,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &l_image_gpu.heap_token);

		l_image_gpu.bind(p_transfer_device);

		return l_image_gpu;
	};

	inline void ImageGPU::free(TransferDevice& p_transfer_device)
	{
		vkDestroyImage(p_transfer_device.device, this->image, NULL);
		p_transfer_device.heap.release_element(this->heap_token);
	};

	inline	void ImageGPU::bind(TransferDevice& p_transfer_device)
	{
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		vkBindImageMemory(p_transfer_device.device, this->image, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};


	inline BufferAllocator BufferAllocator::allocate_default(const GPUInstance& p_instance)
	{

		return BufferAllocator{
			TransferDevice::allocate(p_instance),
			Pool<BufferHost>::allocate(0),
			Pool<BufferGPU>::allocate(0),
			Pool<ImageHost>::allocate(0),
			Pool<ImageGPU>::allocate(0),
			Vector<Token(BufferHost)>::allocate(0),
			Vector<Token(ImageHost)>::allocate(0),
			Vector<Buffer_HosttoGPU_CopyEvent>::allocate(0),
			Vector<Buffer_GPUtoHost_CopyEvent>::allocate(0),
			Vector<Image_HosttoGPU_CopyEvent>::allocate(0),
			Vector<ImageGPUtoHost_CopyEvent>::allocate(0)
		};
	};

	inline void BufferAllocator::free()
	{
		this->step();
		this->force_command_buffer_execution_sync();

		this->clean_garbage_buffers();

		this->host_buffers.free();
		this->gpu_buffers.free();
		this->host_images.free();
		this->gpu_images.free();
		this->garbage_host_buffers.free();
		this->garbage_host_images.free();
		this->buffer_gpu_to_host_copy_events.free();
		this->buffer_host_to_gpu_copy_events.free();
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
		for (vector_loop_reverse(&this->buffer_gpu_to_host_copy_events, i))
		{
			Buffer_GPUtoHost_CopyEvent& l_event = this->buffer_gpu_to_host_copy_events.get(i);
			if (tk_eq(l_event.source_buffer, p_buffer_gpu))
			{
				this->garbage_host_buffers.push_back_element(l_event.target_buffer);
				this->buffer_gpu_to_host_copy_events.erase_element_at(i);
			}
		}

		for (vector_loop_reverse(&this->buffer_host_to_gpu_copy_events, i))
		{
			Buffer_HosttoGPU_CopyEvent& l_event = this->buffer_host_to_gpu_copy_events.get(i);
			if (tk_eq(l_event.target_buffer, p_buffer_gpu))
			{
				this->garbage_host_buffers.push_back_element(l_event.staging_buffer);
				this->buffer_host_to_gpu_copy_events.erase_element_at(i);
			}
		}

		BufferGPU& l_buffer = this->gpu_buffers.get(p_buffer_gpu);
		l_buffer.free(this->device);
		this->gpu_buffers.release_element(p_buffer_gpu);
	};

	inline void BufferAllocator::write_to_buffergpu(const Token(BufferGPU) p_buffer_gpu, const Slice<int8>& p_value)
	{
		Token(BufferHost) l_staging_buffer = this->allocate_bufferhost(p_value, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		this->buffer_host_to_gpu_copy_events.push_back_element(Buffer_HosttoGPU_CopyEvent{ l_staging_buffer, p_buffer_gpu });
	};

	inline Token(BufferHost) BufferAllocator::read_from_buffergpu(const Token(BufferGPU) p_buffer_gpu)
	{
		BufferGPU& l_buffer_gpu = this->gpu_buffers.get(p_buffer_gpu);
		Token(BufferHost) l_staging_buffer = this->allocate_bufferhost_empty(l_buffer_gpu.size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		this->buffer_gpu_to_host_copy_events.push_back_element(Buffer_GPUtoHost_CopyEvent{ p_buffer_gpu, l_staging_buffer });
		return l_staging_buffer;
	};

	inline Token(ImageHost) BufferAllocator::allocate_imagehost(const Slice<int8>& p_value, const ImageFormat& p_image_format, const VkImageUsageFlags p_usage_flags)
	{
		ImageHost l_image_host = ImageHost::allocate(this->device, p_image_format, p_usage_flags, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED);
		l_image_host.push(p_value);
		return this->host_images.alloc_element(l_image_host);
	};

	inline Token(ImageHost) BufferAllocator::allocate_imagehost_empty(const ImageFormat& p_image_format, const VkImageUsageFlags p_usage_flags)
	{
		return this->host_images.alloc_element(ImageHost::allocate(this->device, p_image_format, p_usage_flags, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED));
	};

	inline void BufferAllocator::free_imagehost(const Token(ImageHost) p_image_host)
	{
		ImageHost& l_image_host = this->host_images.get(p_image_host);
		l_image_host.free(this->device);
		this->host_images.release_element(p_image_host);
	};

	inline Slice<int8>& BufferAllocator::get_imagehost_mapped_memory(const Token(ImageHost) p_image_host)
	{
		return this->host_images.get(p_image_host).get_mapped_memory();
	};

	inline Token(ImageGPU) BufferAllocator::allocate_imagegpu(const ImageFormat& p_image_format, const VkImageUsageFlags p_usage_flags)
	{
		return this->gpu_images.alloc_element(ImageGPU::allocate(this->device, p_image_format, p_usage_flags, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED));
	};

	inline void BufferAllocator::free_imagegpu(const Token(ImageGPU) p_image_gpu)
	{
		for (vector_loop_reverse(&this->image_gpu_to_host_copy_events, i))
		{
			ImageGPUtoHost_CopyEvent& l_event = this->image_gpu_to_host_copy_events.get(i);
			if (tk_eq(l_event.source_image, p_image_gpu))
			{
				this->garbage_host_images.push_back_element(l_event.target_image);
				this->image_gpu_to_host_copy_events.erase_element_at(i);
			}
		}

		for (vector_loop_reverse(&this->image_host_to_gpu_copy_events, i))
		{
			Image_HosttoGPU_CopyEvent& l_event = this->image_host_to_gpu_copy_events.get(i);
			if (tk_eq(l_event.target_image, p_image_gpu))
			{
				this->garbage_host_images.push_back_element(l_event.staging_image);
				this->image_host_to_gpu_copy_events.erase_element_at(i);
			}
		}

		ImageGPU& l_image = this->gpu_images.get(p_image_gpu);
		l_image.free(this->device);
		this->gpu_images.release_element(p_image_gpu);
	};

	inline void BufferAllocator::write_to_imagegpu(const Token(ImageGPU) p_image_gpu, const Slice<int8>& p_value /*, const VkImageLayout p_before_layout, const VkImageLayout p_after_layout*/)
	{
		ImageGPU& l_image_gpu = this->gpu_images.get(p_image_gpu);
		Token(ImageHost) l_stagin_image = this->allocate_imagehost(p_value, l_image_gpu.format, VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		this->image_host_to_gpu_copy_events.push_back_element(Image_HosttoGPU_CopyEvent{ l_stagin_image, p_image_gpu });
	};

	inline Token(ImageHost) BufferAllocator::read_from_imagegpu(const Token(ImageGPU) p_image_gpu)
	{
		ImageGPU& l_buffer_gpu = this->gpu_images.get(p_image_gpu);
		Token(ImageHost) l_staging_buffer = this->allocate_imagehost_empty(l_buffer_gpu.format, VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		this->image_gpu_to_host_copy_events.push_back_element(ImageGPUtoHost_CopyEvent{ p_image_gpu, l_staging_buffer });
		return l_staging_buffer;
	};


	struct TextureLayoutTransitionBarrierConfiguration {
		VkAccessFlags src_access_mask;
		VkAccessFlags dst_access_mask;

		VkPipelineStageFlags src_stage;
		VkPipelineStageFlags dst_stage;

		inline TextureLayoutTransitionBarrierConfiguration() {};
		inline TextureLayoutTransitionBarrierConfiguration(const VkAccessFlags p_src_access_mask, const VkAccessFlags p_dst_access_mask,
			const VkPipelineStageFlags p_src_stage, const VkPipelineStageFlags p_dst_stage)
		{
			this->src_access_mask = p_src_access_mask;
			this->dst_access_mask = p_dst_access_mask;
			this->src_stage = p_src_stage;
			this->dst_stage = p_dst_stage;
		};
	};

	template<VkImageLayout SourceLayout, VkImageLayout TargetLayout>
	struct TransitionBarrierConfigurationBuilder
	{
		inline static TextureLayoutTransitionBarrierConfiguration build();
	};

	template<>
	struct TransitionBarrierConfigurationBuilder<VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL> {

		inline static TextureLayoutTransitionBarrierConfiguration build()
		{
			return TextureLayoutTransitionBarrierConfiguration(
				VkAccessFlags(0), VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT);
		};
	};

	template<>
	struct TransitionBarrierConfigurationBuilder<VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL> {

		inline static TextureLayoutTransitionBarrierConfiguration build()
		{
			return TextureLayoutTransitionBarrierConfiguration(
				VkAccessFlags(0), VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT);
		};
	};


	inline void BufferAllocator::step()
	{
		this->clean_garbage_buffers();

		this->device.command_buffer.begin();

		if (this->buffer_host_to_gpu_copy_events.Size > 0)
		{
			for (loop(i, 0, this->buffer_host_to_gpu_copy_events.Size))
			{
				Buffer_HosttoGPU_CopyEvent& l_event = this->buffer_host_to_gpu_copy_events.get(i);
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

			this->buffer_host_to_gpu_copy_events.clear();
		}

		if (this->buffer_gpu_to_host_copy_events.Size > 0)
		{

			for (loop(i, 0, this->buffer_gpu_to_host_copy_events.Size))
			{
				Buffer_GPUtoHost_CopyEvent& l_event = this->buffer_gpu_to_host_copy_events.get(i);
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

			this->buffer_gpu_to_host_copy_events.clear();
		}

		if (this->image_host_to_gpu_copy_events.Size > 0)
		{
			for (loop(i, 0, this->image_host_to_gpu_copy_events.Size))
			{
				Image_HosttoGPU_CopyEvent& l_event = this->image_host_to_gpu_copy_events.get(i);
				ImageHost& l_source = this->host_images.get(l_event.staging_image);
				ImageGPU& l_target = this->gpu_images.get(l_event.target_image);

				VkImageCopy l_region = {
					VkImageSubresourceLayers{l_source.format.imageAspect, 0,0,l_source.format.arrayLayers},
					VkOffset3D{0,0,0},
					VkImageSubresourceLayers{l_target.format.imageAspect, 0,0,l_target.format.arrayLayers},
					VkOffset3D{0,0,0},
					VkExtent3D{l_target.format.extent.x, l_target.format.extent.y, l_target.format.extent.z}
				};


				{
					VkImageMemoryBarrier l_image_memory_barrier{};
					l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					l_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					l_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.image = l_source.image;
					l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{ l_source.format.imageAspect, 0, l_source.format.arrayLayers, 0, l_source.format.arrayLayers };

					TextureLayoutTransitionBarrierConfiguration l_transition_configuration = TransitionBarrierConfigurationBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL>::build();

					l_image_memory_barrier.srcAccessMask = l_transition_configuration.src_access_mask;
					l_image_memory_barrier.dstAccessMask = l_transition_configuration.dst_access_mask;

					vkCmdPipelineBarrier(this->device.command_buffer.command_buffer, l_transition_configuration.src_stage, l_transition_configuration.dst_stage, 0, 0, NULL, 0, NULL,
						1, &l_image_memory_barrier);
				}
				

				{
					VkImageMemoryBarrier l_image_memory_barrier{};
					l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					l_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					l_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.image = l_target.image;
					l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{ l_target.format.imageAspect, 0, l_target.format.arrayLayers, 0, l_target.format.arrayLayers };

					TextureLayoutTransitionBarrierConfiguration l_transition_configuration = TransitionBarrierConfigurationBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL>::build();

					l_image_memory_barrier.srcAccessMask = l_transition_configuration.src_access_mask;
					l_image_memory_barrier.dstAccessMask = l_transition_configuration.dst_access_mask;

					vkCmdPipelineBarrier(this->device.command_buffer.command_buffer, l_transition_configuration.src_stage, l_transition_configuration.dst_stage, 0, 0, NULL, 0, NULL,
						1, &l_image_memory_barrier);
				}

				vkCmdCopyImage(this->device.command_buffer.command_buffer, l_source.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					l_target.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &l_region);

				this->garbage_host_images.push_back_element(l_event.staging_image);
			}

			this->image_host_to_gpu_copy_events.clear();
		}

		if (this->image_gpu_to_host_copy_events.Size > 0)
		{
			for (loop(i, 0, this->image_gpu_to_host_copy_events.Size))
			{
				ImageGPUtoHost_CopyEvent& l_event = this->image_gpu_to_host_copy_events.get(i);
				ImageGPU& l_source = this->gpu_images.get(l_event.source_image);
				ImageHost& l_target = this->host_images.get(l_event.target_image);

				VkImageCopy l_region = {
					VkImageSubresourceLayers{l_source.format.imageAspect, 0,0,l_source.format.arrayLayers},
					VkOffset3D{0,0,0},
					VkImageSubresourceLayers{l_target.format.imageAspect, 0,0,l_target.format.arrayLayers},
					VkOffset3D{0,0,0}, 
					VkExtent3D{l_target.format.extent.x, l_target.format.extent.y, l_target.format.extent.z}
				};

				{
					VkImageMemoryBarrier l_image_memory_barrier{};
					l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					l_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					l_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.image = l_source.image;
					l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{ l_source.format.imageAspect, 0, l_source.format.arrayLayers, 0, l_source.format.arrayLayers };

					TextureLayoutTransitionBarrierConfiguration l_transition_configuration = TransitionBarrierConfigurationBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL>::build();

					l_image_memory_barrier.srcAccessMask = l_transition_configuration.src_access_mask;
					l_image_memory_barrier.dstAccessMask = l_transition_configuration.dst_access_mask;

					vkCmdPipelineBarrier(this->device.command_buffer.command_buffer, l_transition_configuration.src_stage, l_transition_configuration.dst_stage, 0, 0, NULL, 0, NULL,
						1, &l_image_memory_barrier);
				}


				{
					VkImageMemoryBarrier l_image_memory_barrier{};
					l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					l_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					l_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					l_image_memory_barrier.image = l_target.image;
					l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{ l_target.format.imageAspect, 0, l_target.format.arrayLayers, 0, l_target.format.arrayLayers };

					TextureLayoutTransitionBarrierConfiguration l_transition_configuration = TransitionBarrierConfigurationBuilder<VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL>::build();

					l_image_memory_barrier.srcAccessMask = l_transition_configuration.src_access_mask;
					l_image_memory_barrier.dstAccessMask = l_transition_configuration.dst_access_mask;

					vkCmdPipelineBarrier(this->device.command_buffer.command_buffer, l_transition_configuration.src_stage, l_transition_configuration.dst_stage, 0, 0, NULL, 0, NULL,
						1, &l_image_memory_barrier);
				}

				vkCmdCopyImage(this->device.command_buffer.command_buffer, l_source.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					l_target.image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &l_region);
			}

			this->image_gpu_to_host_copy_events.clear();
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

		for (loop(i, 0, this->garbage_host_images.Size))
		{
			this->free_imagehost(this->garbage_host_images.get(i));
		}

		this->garbage_host_images.clear();
	};


}
