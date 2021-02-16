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
		Vector <MemoryGPU> gpu_memories;
		int8 is_memory_mapped;

		static HeapPagedGPU allocate_default(const int8 is_memory_mapped, const gc_t p_transfer_device, const uimax p_memory_chunk_size);

		void free(const gc_t p_transfer_device);

		int8 allocate_element(const uimax p_size, const uimax p_modulo_offset, gc_t p_transfer_device, const uint32 p_memory_type_index, HeapPagedToken* out_chunk);

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


	enum class BufferUsageFlag
	{
		TRANSFER_READ = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		TRANSFER_WRITE = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		UNIFORM = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VERTEX = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		INDEX = VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	};

	enum class BufferIndexType
	{
		UINT16 = VkIndexType::VK_INDEX_TYPE_UINT16,
		UINT32 = VkIndexType::VK_INDEX_TYPE_UINT32
	};

	typedef VkFlags BufferUsageFlags;

	struct MappedHostMemory
	{
		Slice<int8> memory;

		static MappedHostMemory build_default();

		void map(TransferDevice& p_transfer_device, const TransferDeviceHeapToken& p_memory);

		void unmap(TransferDevice& p_device);

		void copy_from(const Slice<int8>& p_from);

		int8 is_mapped();
	};

#define ShadowBuffer_t(Prefix) ShadowBuffer_##Prefix
#define ShadowBuffer_member_buffer buffer
#define ShadowBuffer_member_size size

	/*
		A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the CPU.
	*/
	struct BufferHost
	{
		MappedHostMemory memory;
		TransferDeviceHeapToken heap_token;
		VkBuffer ShadowBuffer_member_buffer;
		uimax ShadowBuffer_member_size;

		static BufferHost allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const BufferUsageFlag p_usage_flags);

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
		VkBuffer ShadowBuffer_member_buffer;
		uimax ShadowBuffer_member_size;

		static BufferGPU allocate(TransferDevice& p_transfer_device, const uimax p_size, const BufferUsageFlag p_usage_flags);

		void free(TransferDevice& p_transfer_device);

	private:
		void bind(TransferDevice& p_transfer_device);
	};

	enum class ImageUsageFlag
	{
		UNDEFINED = 0,
		TRANSFER_READ = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		TRANSFER_WRITE = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		SHADER_COLOR_ATTACHMENT = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		SHADER_DEPTH_ATTACHMENT = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		SHADER_TEXTURE_PARAMETER = VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT
	};

	typedef uint8 ImageUsageFlags;

	struct ImageFormat
	{
		VkImageAspectFlags imageAspect;
		VkImageType imageType;
		ImageUsageFlag imageUsage;
		VkFormat format;
		v3ui extent;
		uint32 mipLevels;
		uint32 arrayLayers;
		VkSampleCountFlagBits samples;

		static ImageFormat build_color_2d(const v3ui& p_extend, const ImageUsageFlag p_usage);

		static ImageFormat build_depth_2d(const v3ui& p_extend, const ImageUsageFlag p_usage);
	};

#define ShadowImage(Prefix) ShadowImage_##Prefix
#define ShadowImage_member_image image
#define ShadowImage_member_format format

	struct ImageHost
	{
		MappedHostMemory memory;
		TransferDeviceHeapToken heap_token;
		VkImage ShadowImage_member_image;
		ImageFormat ShadowImage_member_format;

		static ImageHost allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format, const VkImageLayout p_initial_layout);

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
		VkImage ShadowImage_member_image;
		ImageFormat ShadowImage_member_format;
		uimax size;

		static ImageGPU allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format, const VkImageLayout p_initial_layout);

		void free(TransferDevice& p_transfer_device);

	private:
		void bind(TransferDevice& p_transfer_device);
	};

	struct ImageLayoutTransitionBarrierConfiguration
	{
		VkAccessFlags src_access_mask;
		VkPipelineStageFlags src_stage;

		VkAccessFlags dst_access_mask;
		VkPipelineStageFlags dst_stage;
	};

	namespace ImageLayoutTransitionBarriers_const
	{
		static uint8 ImageUsageCount = 6;

	};

	struct ImageLayoutTransitionBarriers
	{
		Span<ImageLayoutTransitionBarrierConfiguration> barriers;

		inline static ImageLayoutTransitionBarriers allocate();

		inline ImageLayoutTransitionBarrierConfiguration get_barrier(const ImageUsageFlag p_left, const ImageUsageFlag p_right) const;

		inline static VkImageLayout get_imagelayout_from_imageusage(const ImageUsageFlag p_flag);

		inline void free();

	private:

		inline uint8 get_index_from_imageusage(const ImageUsageFlag p_flag) const;
	};

	/*
		The BufferAllocator is the front layer that register and execute all allocation, copy of buffers.
	*/
	struct BufferAllocator
	{
		TransferDevice device;
		ImageLayoutTransitionBarriers image_layout_barriers;

		Pool <BufferHost> host_buffers;
		Pool <BufferGPU> gpu_buffers;
		Pool <ImageHost> host_images;
		Pool <ImageGPU> gpu_images;

		Vector <Token(BufferHost)> garbage_host_buffers;
		Vector <Token(ImageHost)> garbage_host_images;

		struct Buffer_HosttoGPU_CopyEvent
		{
			Token(BufferHost) staging_buffer;
			Token(BufferGPU) target_buffer;
		};

		Vector <Buffer_HosttoGPU_CopyEvent> buffer_host_to_gpu_copy_events;

		struct Buffer_GPUtoHost_CopyEvent
		{
			Token(BufferGPU) source_buffer;
			Token(BufferHost) target_buffer;
		};

		Vector <Buffer_GPUtoHost_CopyEvent> buffer_gpu_to_host_copy_events;

		struct ImageHost_AllocatedEvent
		{
			Token(ImageHost) image;
		};

		Vector <ImageHost_AllocatedEvent> image_host_allocate_events;

		struct ImageGPU_AllocatedEvent
		{
			Token(ImageGPU) image;
		};

		Vector <ImageGPU_AllocatedEvent> image_gpu_allocate_events;

		struct Image_HosttoGPU_CopyEvent
		{
			Token(ImageHost) staging_image;
			Token(ImageGPU) target_image;
		};

		Vector <Image_HosttoGPU_CopyEvent> image_host_to_gpu_copy_events;

		struct ImageGPUtoHost_CopyEvent
		{
			Token(ImageGPU) source_image;
			Token(ImageHost) target_image;
		};

		Vector <ImageGPUtoHost_CopyEvent> image_gpu_to_host_copy_events;

		struct BufferHosttoImageGPU_CopyEvent
		{
			Token(BufferHost) source_buffer;
			Token(ImageGPU) target_image;
		};

		Vector <BufferHosttoImageGPU_CopyEvent> buffer_host_to_image_gpu_cpy_event;

		struct ImageGPU_to_BufferHost_CopyEvent
		{
			Token(ImageGPU) source_image;
			Token(BufferHost) target_buffer;
		};

		Vector <ImageGPU_to_BufferHost_CopyEvent> image_gpu_to_buffer_host_copy_events;

		static BufferAllocator allocate_default(const GPUInstance& p_instance);

		void free();

		Token(BufferHost) allocate_bufferhost(const Slice<int8>& p_value, const BufferUsageFlag p_usage_flags);

		Token(BufferHost) allocate_bufferhost_empty(const uimax p_size, const BufferUsageFlag p_usage_flags);

		void free_bufferhost(const Token(BufferHost) p_buffer_host);

		BufferHost& get_bufferhost(const Token(BufferHost) p_buffer_host);

		Token(BufferGPU) allocate_buffergpu(const uimax p_size, const BufferUsageFlag p_usage_flags);

		void free_buffergpu(const Token(BufferGPU) p_buffer_gpu);

		void write_to_buffergpu(const Token(BufferGPU) p_buffer_gpu, const Slice<int8>& p_value);

		BufferGPU& get_buffergpu(const Token(BufferGPU) p_buffer_gpu);

		Token(BufferHost) read_from_buffergpu(const Token(BufferGPU) p_buffer_gpu_token, const BufferGPU& p_buffer_gpu);

		Token(ImageHost) allocate_imagehost(const Slice<int8>& p_value, const ImageFormat& p_image_format);

		Token(ImageHost) allocate_imagehost_empty(const ImageFormat& p_image_format);

		void free_imagehost(const Token(ImageHost) p_image_host);

		ImageHost& get_imagehost(const Token(ImageHost) p_image_host);

		Token(ImageGPU) allocate_imagegpu(const ImageFormat& p_image_format);

		void free_imagegpu(const Token(ImageGPU) p_image_gpu);

		ImageGPU& get_imagegpu(const Token(ImageGPU) p_image_gpu);

		void write_to_imagegpu(const Token(ImageGPU) p_image_gpu_token, const ImageGPU& p_image_gpu, const Slice<int8>& p_value);

		Token(ImageHost) read_from_imagegpu(const Token(ImageGPU) p_image_gpu_token, const ImageGPU& p_image_gpu);

		Token(BufferHost) read_from_imagegpu_to_buffer(const Token(ImageGPU) p_image_gpu_token, const ImageGPU& p_image_gpu);

		void step();

	private:
		void clean_garbage_buffers();
	};

	struct BufferCommandUtils
	{
		static void cmd_copy_buffer_host_to_gpu(const CommandBuffer& p_command_buffer, const BufferHost& p_host, const BufferGPU& p_gpu);

		static void cmd_copy_buffer_gpu_to_host(const CommandBuffer& p_command_buffer, const BufferGPU& p_gpu, const BufferHost& p_host);

		template<class ShadowImage(Source), class ShadowImage(Target)>
		static void cmd_copy_shadowimage_to_shadowimage(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ShadowImage(Source)& p_source, const ShadowImage(Target)& p_target);

		static void cmd_copy_buffer_host_to_image_gpu(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const BufferHost& p_host, const ImageGPU& p_gpu);

		static void cmd_copy_image_gpu_to_buffer_host(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ImageGPU& p_gpu, const BufferHost& p_host);

		static void cmd_image_layout_transition(const CommandBuffer& p_command_buffer, const VkImage p_image, const ImageFormat& p_image_format,
				const VkImageLayout p_source_image_layout, const VkImageLayout p_target_image_layout, const ImageLayoutTransitionBarrierConfiguration& p_lyaout_transition_configuration);

		template<class ShadowImage(_)>
		static void cmd_image_layout_transition_v2(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ShadowImage(_)& p_image,
				const ImageUsageFlag p_source_image_usage, const ImageUsageFlag p_target_image_usage);

	private:
		static void cmd_copy_buffer(const CommandBuffer& p_command_buffer, const VkBuffer p_source_buffer, const uimax p_source_size, const VkBuffer p_target_buffer, const uimax p_target_size);

		static void cmd_revert_image_layout_from_transfer_dst(const CommandBuffer& p_command_buffer, const VkImage p_image, const ImageFormat& p_format);

		static void cmd_revert_image_layout_from_transfer_src(const CommandBuffer& p_command_buffer, const VkImage p_image, const ImageFormat& p_format);
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

	inline int8 HeapPagedGPU::allocate_element(const uimax p_size, const uimax p_modulo_offset, gc_t p_transfer_device, const uint32 p_memory_type_index, HeapPagedToken* out_chunk)
	{
		HeapPaged::AllocatedElementReturn l_allocated_chunk;
		HeapPaged::AllocationState l_allocation_state = this->heap.allocate_element_norealloc_with_modulo_offset(p_size, p_modulo_offset, &l_allocated_chunk);
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
		//We make sure that memory have an allocated heap size of max(p_requirements.size, p_requirements.alignment)
		uimax l_aligned_size = p_requirements.size;
		if (l_aligned_size < p_requirements.alignment)
		{
			l_aligned_size = p_requirements.alignment;
		};

		uint32 l_memory_type_index = p_graphics_card.get_memory_type_index(p_requirements, p_memory_property_flags);
		out_token->heap_index = p_graphics_card.device_memory_properties.memoryTypes[l_memory_type_index].heapIndex;
		return this->gpu_heaps.get(out_token->heap_index).allocate_element(l_aligned_size, p_requirements.alignment, p_transfer_device, l_memory_type_index, &out_token->heap_paged_token);
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
		l_transfer_device.device = p_instance.logical_device;

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
		this->command_pool.free_command_buffer(this->device, this->command_buffer);
		this->command_pool.free(this->device);
	};

	inline MappedHostMemory MappedHostMemory::build_default()
	{
		return MappedHostMemory{
				Slice<int8>::build(NULL, 0, 0)
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


	inline BufferHost BufferHost::allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const BufferUsageFlag p_usage_flags)
	{
		BufferHost l_buffer_host;

		l_buffer_host.size = p_buffer_size;
		l_buffer_host.memory = MappedHostMemory::build_default();

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = (VkBufferUsageFlags)p_usage_flags;
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
#if GPU_BOUND_TEST
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


	inline BufferGPU BufferGPU::allocate(TransferDevice& p_transfer_device, const uimax p_size, const BufferUsageFlag p_usage_flags)
	{
		BufferGPU l_buffer_gpu;

		l_buffer_gpu.ShadowBuffer_member_size = p_size;

		VkBufferCreateInfo l_buffercreate_info{};
		l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffercreate_info.usage = (VkBufferUsageFlags)p_usage_flags;
		l_buffercreate_info.size = p_size;

		vk_handle_result(vkCreateBuffer(p_transfer_device.device, &l_buffercreate_info, NULL, &l_buffer_gpu.buffer));

		VkMemoryRequirements l_requirements;
		vkGetBufferMemoryRequirements(p_transfer_device.device, l_buffer_gpu.ShadowBuffer_member_buffer, &l_requirements);

		p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_requirements, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &l_buffer_gpu.heap_token);
		// p_transfer_device.heap.allocate_gpu_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_buffer_gpu.heap_token);
		l_buffer_gpu.bind(p_transfer_device);

		return l_buffer_gpu;
	};

	inline void BufferGPU::free(TransferDevice& p_transfer_device)
	{
		p_transfer_device.heap.release_element(this->heap_token);
		// p_transfer_device.heap.release_gpu_write_element(this->heap_token);
		vkDestroyBuffer(p_transfer_device.device, this->ShadowBuffer_member_buffer, NULL);
		this->ShadowBuffer_member_buffer = NULL;
	};

	inline void BufferGPU::bind(TransferDevice& p_transfer_device)
	{
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		// SliceOffset<int8> l_memory = p_transfer_device.heap.get_gpu_write_gcmemory_and_offset(this->heap_token);
		vkBindBufferMemory(p_transfer_device.device, this->buffer, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline ImageFormat ImageFormat::build_color_2d(const v3ui& p_extend, const ImageUsageFlag p_usage)
	{
		ImageFormat l_color_imageformat;
		l_color_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		l_color_imageformat.arrayLayers = 1;
		l_color_imageformat.format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
		l_color_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		l_color_imageformat.mipLevels = 1;
		l_color_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		l_color_imageformat.extent = p_extend;
		l_color_imageformat.imageUsage = p_usage;
		return l_color_imageformat;
	};

	inline ImageFormat ImageFormat::build_depth_2d(const v3ui& p_extend, const ImageUsageFlag p_usage)
	{
		ImageFormat l_depth_imageformat;
		l_depth_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
		l_depth_imageformat.arrayLayers = 1;
		l_depth_imageformat.format = VkFormat::VK_FORMAT_D16_UNORM;
		l_depth_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		l_depth_imageformat.mipLevels = 1;
		l_depth_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		l_depth_imageformat.extent = p_extend;
		l_depth_imageformat.imageUsage = p_usage;
		return l_depth_imageformat;
	};

	inline ImageHost ImageHost::allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format, const VkImageLayout p_initial_layout)
	{
		ImageHost l_image_host;
		l_image_host.memory = MappedHostMemory::build_default();
		l_image_host.format = p_image_format;
		// l_image_host.target_image_layout = ImageUsageUtils::get_target_imagelayout_from_usage(l_image_host.format.imageUsage);

		VkImageCreateInfo l_image_create_info{};
		l_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		l_image_create_info.imageType = p_image_format.imageType;
		l_image_create_info.format = p_image_format.format;
		l_image_create_info.extent = VkExtent3D{ (uint32_t)p_image_format.extent.x, (uint32_t)p_image_format.extent.y, (uint32_t)p_image_format.extent.z };
		l_image_create_info.mipLevels = p_image_format.mipLevels;
		l_image_create_info.arrayLayers = p_image_format.arrayLayers;
		l_image_create_info.samples = p_image_format.samples;
		l_image_create_info.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR; //This is mandatory for host readable image
		l_image_create_info.usage = (VkImageUsageFlags)p_image_format.imageUsage;
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
#if GPU_BOUND_TEST
		assert_true(this->memory.is_mapped());
#endif
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		// SliceOffset<int8> l_memory = p_transfer_device.heap.get_host_write_gcmemory_and_offset(this->heap_token);
		vkBindImageMemory(p_transfer_device.device, this->image, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};

	inline ImageGPU ImageGPU::allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format, const VkImageLayout p_initial_layout)
	{
		ImageGPU l_image_gpu;
		l_image_gpu.format = p_image_format;
		// l_image_gpu.target_image_layout = ImageUsageUtils::get_target_imagelayout_from_usage(l_image_gpu.format.imageUsage);

		VkImageCreateInfo l_image_create_info{};
		l_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		l_image_create_info.imageType = p_image_format.imageType;
		l_image_create_info.format = p_image_format.format;
		l_image_create_info.extent = VkExtent3D{ (uint32_t)p_image_format.extent.x, (uint32_t)p_image_format.extent.y, (uint32_t)p_image_format.extent.z };
		l_image_create_info.mipLevels = p_image_format.mipLevels;
		l_image_create_info.arrayLayers = p_image_format.arrayLayers;
		l_image_create_info.samples = p_image_format.samples;
		l_image_create_info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		l_image_create_info.usage = (VkImageUsageFlags)p_image_format.imageUsage;
		l_image_create_info.initialLayout = p_initial_layout;

		vk_handle_result(vkCreateImage(p_transfer_device.device, &l_image_create_info, NULL, &l_image_gpu.image));

		VkMemoryRequirements l_requirements;
		vkGetImageMemoryRequirements(p_transfer_device.device, l_image_gpu.image, &l_requirements);

		l_image_gpu.size = l_requirements.size;

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

	inline void ImageGPU::bind(TransferDevice& p_transfer_device)
	{
		SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
		vkBindImageMemory(p_transfer_device.device, this->image, (VkDeviceMemory)l_memory.Memory, l_memory.Offset);
	};


	inline ImageLayoutTransitionBarriers ImageLayoutTransitionBarriers::allocate()
	{
		const ImageLayoutTransitionBarrierConfiguration undefined_to_transfert_dst = ImageLayoutTransitionBarrierConfiguration
				{
						VkAccessFlags(0), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT
				};
		const ImageLayoutTransitionBarrierConfiguration undefined_to_transfert_src = ImageLayoutTransitionBarrierConfiguration{
				VkAccessFlags(0), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT
		};
		const ImageLayoutTransitionBarrierConfiguration transfer_src_to_shader_readonly = ImageLayoutTransitionBarrierConfiguration{
				VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		};
		const ImageLayoutTransitionBarrierConfiguration transfer_dst_to_shader_readonly = ImageLayoutTransitionBarrierConfiguration{
				VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		};
		const ImageLayoutTransitionBarrierConfiguration undefined_to_shader_readonly = ImageLayoutTransitionBarrierConfiguration{
				VkAccessFlags(0), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		};

		ImageLayoutTransitionBarriers l_barriers = ImageLayoutTransitionBarriers{
				Span<ImageLayoutTransitionBarrierConfiguration>::callocate(ImageLayoutTransitionBarriers_const::ImageUsageCount * ImageLayoutTransitionBarriers_const::ImageUsageCount) };
		l_barriers.barriers.get(1) = undefined_to_transfert_src;
		l_barriers.barriers.get(2) = undefined_to_transfert_dst;
		l_barriers.barriers.get(3) = undefined_to_shader_readonly;
		l_barriers.barriers.get(4) = undefined_to_shader_readonly;
		l_barriers.barriers.get(5) = undefined_to_shader_readonly;

		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 1) = undefined_to_transfert_src;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 2) = undefined_to_transfert_dst;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 3) = transfer_src_to_shader_readonly;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 4) = transfer_src_to_shader_readonly;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 5) = transfer_src_to_shader_readonly;

		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 1) = undefined_to_transfert_dst;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 2) = undefined_to_transfert_src;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 3) = transfer_dst_to_shader_readonly;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 4) = transfer_dst_to_shader_readonly;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 5) = transfer_dst_to_shader_readonly;

		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 3) + 1) = undefined_to_transfert_src;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 3) + 2) = undefined_to_transfert_dst;

		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 4) + 1) = undefined_to_transfert_src;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 4) + 2) = undefined_to_transfert_dst;

		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 5) + 1) = undefined_to_transfert_src;
		l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 5) + 2) = undefined_to_transfert_dst;

		return l_barriers;
	};

	inline ImageLayoutTransitionBarrierConfiguration ImageLayoutTransitionBarriers::get_barrier(const ImageUsageFlag p_left, const ImageUsageFlag p_right) const
	{
		return this->barriers.get(
				((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * this->get_index_from_imageusage(p_left))
				+
				this->get_index_from_imageusage(p_right)
		);
	};

	inline VkImageLayout ImageLayoutTransitionBarriers::get_imagelayout_from_imageusage(const ImageUsageFlag p_flag)
	{
		if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT)
		{
			return VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)
		{
			return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}
		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER)
		{
			return VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::TRANSFER_READ)
		{
			return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE)
		{
			return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}

		return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	};

	inline void ImageLayoutTransitionBarriers::free()
	{
		this->barriers.free();
	};

	inline uint8 ImageLayoutTransitionBarriers::get_index_from_imageusage(const ImageUsageFlag p_flag) const
	{
		if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT)
		{
			return 3;
		}
		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)
		{
			return 4;
		}
		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER)
		{
			return 5;
		}

		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::TRANSFER_READ)
		{
			return 1;
		}
		else if ((ImageUsageFlags)p_flag & (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE)
		{
			return 2;
		}

		return 0;
	};

	inline BufferAllocator BufferAllocator::allocate_default(const GPUInstance& p_instance)
	{

		return BufferAllocator{
				TransferDevice::allocate(p_instance),
				ImageLayoutTransitionBarriers::allocate(),
				Pool<BufferHost>::allocate(0),
				Pool<BufferGPU>::allocate(0),
				Pool<ImageHost>::allocate(0),
				Pool<ImageGPU>::allocate(0),
				Vector<Token(BufferHost) >::allocate(0),
				Vector<Token(ImageHost) >::allocate(0),
				Vector<Buffer_HosttoGPU_CopyEvent>::allocate(0),
				Vector<Buffer_GPUtoHost_CopyEvent>::allocate(0),
				Vector<ImageHost_AllocatedEvent>::allocate(0),
				Vector<ImageGPU_AllocatedEvent>::allocate(0),
				Vector<Image_HosttoGPU_CopyEvent>::allocate(0),
				Vector<ImageGPUtoHost_CopyEvent>::allocate(0),
				Vector<BufferHosttoImageGPU_CopyEvent>::allocate(0),
				Vector<ImageGPU_to_BufferHost_CopyEvent>::allocate(0)
		};
	};

	inline void BufferAllocator::free()
	{
		this->step();
		this->device.command_buffer.force_sync_execution();

		this->clean_garbage_buffers();

#if GPU_BOUND_TEST
		assert_true(!this->host_buffers.has_allocated_elements());
		assert_true(!this->gpu_buffers.has_allocated_elements());
		assert_true(!this->host_images.has_allocated_elements());
		assert_true(!this->gpu_images.has_allocated_elements());
		assert_true(this->garbage_host_buffers.empty());
		assert_true(this->garbage_host_images.empty());
		assert_true(this->buffer_gpu_to_host_copy_events.empty());
		assert_true(this->buffer_host_to_gpu_copy_events.empty());
		assert_true(this->image_host_allocate_events.empty());
		assert_true(this->image_gpu_allocate_events.empty());
		assert_true(this->image_gpu_to_host_copy_events.empty());
		assert_true(this->image_host_to_gpu_copy_events.empty());
		assert_true(this->buffer_host_to_image_gpu_cpy_event.empty());
		assert_true(this->image_gpu_to_buffer_host_copy_events.empty());
#endif

		this->image_layout_barriers.free();

		this->host_buffers.free();
		this->gpu_buffers.free();
		this->host_images.free();
		this->gpu_images.free();
		this->garbage_host_buffers.free();
		this->garbage_host_images.free();
		this->buffer_gpu_to_host_copy_events.free();
		this->buffer_host_to_gpu_copy_events.free();
		this->image_host_allocate_events.free();
		this->image_gpu_allocate_events.free();
		this->image_gpu_to_host_copy_events.free();
		this->image_host_to_gpu_copy_events.free();
		this->buffer_host_to_image_gpu_cpy_event.free();
		this->image_gpu_to_buffer_host_copy_events.free();
		this->device.free();
	};

	inline Token(BufferHost) BufferAllocator::allocate_bufferhost(const Slice<int8>& p_value, const BufferUsageFlag p_usage_flags)
	{
		BufferHost l_buffer = BufferHost::allocate(this->device, p_value.Size, p_usage_flags);
		l_buffer.push(p_value);
		return this->host_buffers.alloc_element(l_buffer);
	};

	inline Token(BufferHost) BufferAllocator::allocate_bufferhost_empty(const uimax p_size, const BufferUsageFlag p_usage_flags)
	{
		return this->host_buffers.alloc_element(BufferHost::allocate(this->device, p_size, p_usage_flags));
	};

	inline void BufferAllocator::free_bufferhost(const Token(BufferHost) p_buffer_host)
	{
		BufferHost& l_buffer = this->host_buffers.get(p_buffer_host);
		l_buffer.free(this->device);
		this->host_buffers.release_element(p_buffer_host);
	};

	inline BufferHost& BufferAllocator::get_bufferhost(const Token(BufferHost) p_buffer_host)
	{
		return this->host_buffers.get(p_buffer_host);
	};

	inline Token(BufferGPU) BufferAllocator::allocate_buffergpu(const uimax p_size, const BufferUsageFlag p_usage_flags)
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
		Token(BufferHost) l_staging_buffer = this->allocate_bufferhost(p_value, BufferUsageFlag::TRANSFER_READ);
		this->buffer_host_to_gpu_copy_events.push_back_element(Buffer_HosttoGPU_CopyEvent{ l_staging_buffer, p_buffer_gpu });
	};

	inline BufferGPU& BufferAllocator::get_buffergpu(const Token(BufferGPU) p_buffer_gpu)
	{
		return this->gpu_buffers.get(p_buffer_gpu);
	};

	inline Token(BufferHost) BufferAllocator::read_from_buffergpu(const Token(BufferGPU) p_buffer_gpu_token, const BufferGPU& p_buffer_gpu)
	{
		Token(BufferHost) l_staging_buffer = this->allocate_bufferhost_empty(p_buffer_gpu.size, BufferUsageFlag::TRANSFER_WRITE);
		this->buffer_gpu_to_host_copy_events.push_back_element(Buffer_GPUtoHost_CopyEvent{ p_buffer_gpu_token, l_staging_buffer });
		return l_staging_buffer;
	};

	inline Token(ImageHost) BufferAllocator::allocate_imagehost(const Slice<int8>& p_value, const ImageFormat& p_image_format)
	{
		ImageHost l_image_host = ImageHost::allocate(this->device, p_image_format, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED);
		l_image_host.push(p_value);
		return this->host_images.alloc_element(l_image_host);
	};

	inline Token(ImageHost) BufferAllocator::allocate_imagehost_empty(const ImageFormat& p_image_format)
	{
		Token(ImageHost) l_allocated_image = this->host_images.alloc_element(ImageHost::allocate(this->device, p_image_format, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED));
		this->image_host_allocate_events.push_back_element(ImageHost_AllocatedEvent{ l_allocated_image });
		return l_allocated_image;
	};

	inline void BufferAllocator::free_imagehost(const Token(ImageHost) p_image_host)
	{
		for (vector_loop_reverse(&this->image_host_allocate_events, i))
		{
			ImageHost_AllocatedEvent& l_event = this->image_host_allocate_events.get(i);
			if (tk_eq(l_event.image, p_image_host))
			{
				this->image_host_allocate_events.erase_element_at(i);
			}
		}


		ImageHost& l_image_host = this->host_images.get(p_image_host);
		l_image_host.free(this->device);
		this->host_images.release_element(p_image_host);
	};

	inline ImageHost& BufferAllocator::get_imagehost(const Token(ImageHost) p_image_host)
	{
		return this->host_images.get(p_image_host);
	};

	inline Token(ImageGPU) BufferAllocator::allocate_imagegpu(const ImageFormat& p_image_format)
	{
		Token(ImageGPU) l_allocated_image = this->gpu_images.alloc_element(ImageGPU::allocate(this->device, p_image_format, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED));
		this->image_gpu_allocate_events.push_back_element(ImageGPU_AllocatedEvent{ l_allocated_image });
		return l_allocated_image;
	};

	inline ImageGPU& BufferAllocator::get_imagegpu(const Token(ImageGPU) p_image_gpu)
	{
		return this->gpu_images.get(p_image_gpu);
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

		for (vector_loop_reverse(&this->buffer_host_to_image_gpu_cpy_event, i))
		{
			BufferHosttoImageGPU_CopyEvent& l_event = this->buffer_host_to_image_gpu_cpy_event.get(i);
			if (tk_eq(l_event.target_image, p_image_gpu))
			{
				this->garbage_host_buffers.push_back_element(l_event.source_buffer);
				this->buffer_host_to_image_gpu_cpy_event.erase_element_at(i);
			}
		}

		for (vector_loop_reverse(&this->image_gpu_to_buffer_host_copy_events, i))
		{
			ImageGPU_to_BufferHost_CopyEvent& l_event = this->image_gpu_to_buffer_host_copy_events.get(i);
			if (tk_eq(l_event.source_image, p_image_gpu))
			{
				this->garbage_host_buffers.push_back_element(l_event.target_buffer);
				this->image_gpu_to_buffer_host_copy_events.erase_element_at(i);
			}
		}

		for (vector_loop_reverse(&this->image_gpu_allocate_events, i))
		{
			ImageGPU_AllocatedEvent& l_event = this->image_gpu_allocate_events.get(i);
			if (tk_eq(l_event.image, p_image_gpu))
			{
				this->image_gpu_allocate_events.erase_element_at(i);
			}
		}

		ImageGPU& l_image = this->gpu_images.get(p_image_gpu);
		l_image.free(this->device);
		this->gpu_images.release_element(p_image_gpu);
	};

	inline void BufferAllocator::write_to_imagegpu(const Token(ImageGPU) p_image_gpu_token, const ImageGPU& p_image_gpu, const Slice<int8>& p_value)
	{
		/*
		ImageFormat l_stagin_image_format = p_image_gpu.format;
		l_stagin_image_format.imageUsage = ImageUsageFlag::TRANSFER_READ;
		Token(ImageHost) l_stagin_image = this->allocate_imagehost(p_value, l_stagin_image_format);
		this->image_host_to_gpu_copy_events.push_back_element(Image_HosttoGPU_CopyEvent{ l_stagin_image, p_image_gpu_token });
		*/
		Token(BufferHost) l_stagin_buffer = this->allocate_bufferhost(p_value, BufferUsageFlag::TRANSFER_READ);
		this->buffer_host_to_image_gpu_cpy_event.push_back_element(BufferHosttoImageGPU_CopyEvent{ l_stagin_buffer, p_image_gpu_token });
	};

	inline Token(ImageHost) BufferAllocator::read_from_imagegpu(const Token(ImageGPU) p_image_gpu_token, const ImageGPU& p_image_gpu)
	{
		ImageFormat l_stagin_image_format = p_image_gpu.format;
		l_stagin_image_format.imageUsage = ImageUsageFlag::TRANSFER_WRITE;
		Token(ImageHost) l_staging_buffer = this->allocate_imagehost_empty(l_stagin_image_format);
		this->image_gpu_to_host_copy_events.push_back_element(ImageGPUtoHost_CopyEvent{ p_image_gpu_token, l_staging_buffer });
		return l_staging_buffer;
	};

	inline Token(BufferHost) BufferAllocator::read_from_imagegpu_to_buffer(const Token(ImageGPU) p_image_gpu_token, const ImageGPU& p_image_gpu)
	{
		Token(BufferHost) l_stagin_buffer = this->allocate_bufferhost_empty(p_image_gpu.size, BufferUsageFlag::TRANSFER_WRITE);
		this->image_gpu_to_buffer_host_copy_events.push_back_element(ImageGPU_to_BufferHost_CopyEvent{ p_image_gpu_token, l_stagin_buffer });
		return l_stagin_buffer;
	};

	inline void BufferAllocator::step()
	{
		this->clean_garbage_buffers();

		this->device.command_buffer.begin();

		if (this->image_host_allocate_events.Size > 0)
		{
			for (loop(i, 0, this->image_host_allocate_events.Size))
			{
				Token(ImageHost) l_image_token = this->image_host_allocate_events.get(i).image;
				ImageHost& l_image = this->host_images.get(l_image_token);
				BufferCommandUtils::cmd_image_layout_transition_v2(this->device.command_buffer, this->image_layout_barriers,
						l_image,
						ImageUsageFlag::UNDEFINED,
						l_image.format.imageUsage);
			}
			this->image_host_allocate_events.clear();
		}

		if (this->image_gpu_allocate_events.Size > 0)
		{
			for (loop(i, 0, this->image_gpu_allocate_events.Size))
			{
				Token(ImageGPU) l_image_token = this->image_gpu_allocate_events.get(i).image;
				ImageGPU& l_image = this->gpu_images.get(l_image_token);
				BufferCommandUtils::cmd_image_layout_transition_v2(this->device.command_buffer, this->image_layout_barriers,
						l_image,
						ImageUsageFlag::UNDEFINED,
						l_image.format.imageUsage);
			}
			this->image_gpu_allocate_events.clear();
		}

		if (this->buffer_host_to_image_gpu_cpy_event.Size > 0)
		{
			for (loop(i, 0, this->buffer_host_to_image_gpu_cpy_event.Size))
			{
				BufferHosttoImageGPU_CopyEvent& l_event = this->buffer_host_to_image_gpu_cpy_event.get(i);
				BufferCommandUtils::cmd_copy_buffer_host_to_image_gpu(
						this->device.command_buffer, this->image_layout_barriers,
						this->host_buffers.get(l_event.source_buffer),
						this->gpu_images.get(l_event.target_image)
				);
				this->garbage_host_buffers.push_back_element(l_event.source_buffer);
			}
			this->buffer_host_to_image_gpu_cpy_event.clear();
		}

		if (this->image_gpu_to_buffer_host_copy_events.Size > 0)
		{
			for (loop(i, 0, this->image_gpu_to_buffer_host_copy_events.Size))
			{
				ImageGPU_to_BufferHost_CopyEvent& l_event = this->image_gpu_to_buffer_host_copy_events.get(i);
				BufferCommandUtils::cmd_copy_image_gpu_to_buffer_host(
						this->device.command_buffer, this->image_layout_barriers,
						this->gpu_images.get(l_event.source_image),
						this->host_buffers.get(l_event.target_buffer)
				);
			}
			this->image_gpu_to_buffer_host_copy_events.clear();
		}

		if (this->buffer_host_to_gpu_copy_events.Size > 0)
		{
			for (loop(i, 0, this->buffer_host_to_gpu_copy_events.Size))
			{
				Buffer_HosttoGPU_CopyEvent& l_event = this->buffer_host_to_gpu_copy_events.get(i);
				BufferCommandUtils::cmd_copy_buffer_host_to_gpu(this->device.command_buffer,
						this->host_buffers.get(l_event.staging_buffer),
						this->gpu_buffers.get(l_event.target_buffer));
				this->garbage_host_buffers.push_back_element(l_event.staging_buffer);
			}

			this->buffer_host_to_gpu_copy_events.clear();
		}

		if (this->buffer_gpu_to_host_copy_events.Size > 0)
		{

			for (loop(i, 0, this->buffer_gpu_to_host_copy_events.Size))
			{
				Buffer_GPUtoHost_CopyEvent& l_event = this->buffer_gpu_to_host_copy_events.get(i);
				BufferCommandUtils::cmd_copy_buffer_gpu_to_host(this->device.command_buffer,
						this->gpu_buffers.get(l_event.source_buffer),
						this->host_buffers.get(l_event.target_buffer));
			}

			this->buffer_gpu_to_host_copy_events.clear();
		}

		if (this->image_host_to_gpu_copy_events.Size > 0)
		{
			for (loop(i, 0, this->image_host_to_gpu_copy_events.Size))
			{
				Image_HosttoGPU_CopyEvent& l_event = this->image_host_to_gpu_copy_events.get(i);
				BufferCommandUtils::cmd_copy_shadowimage_to_shadowimage(this->device.command_buffer, this->image_layout_barriers,
						this->host_images.get(l_event.staging_image),
						this->gpu_images.get(l_event.target_image));
				this->garbage_host_images.push_back_element(l_event.staging_image);
			}

			this->image_host_to_gpu_copy_events.clear();
		}

		if (this->image_gpu_to_host_copy_events.Size > 0)
		{
			for (loop(i, 0, this->image_gpu_to_host_copy_events.Size))
			{
				ImageGPUtoHost_CopyEvent& l_event = this->image_gpu_to_host_copy_events.get(i);
				BufferCommandUtils::cmd_copy_shadowimage_to_shadowimage(this->device.command_buffer, this->image_layout_barriers,
						this->gpu_images.get(l_event.source_image),
						this->host_images.get(l_event.target_image));
			}

			this->image_gpu_to_host_copy_events.clear();
		}

		this->device.command_buffer.end();
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


	inline void BufferCommandUtils::cmd_copy_buffer_host_to_gpu(const CommandBuffer& p_command_buffer, const BufferHost& p_host, const BufferGPU& p_gpu)
	{
		BufferCommandUtils::cmd_copy_buffer(p_command_buffer, p_host.buffer, p_host.size, p_gpu.buffer, p_gpu.size);
	};

	inline void BufferCommandUtils::cmd_copy_buffer_gpu_to_host(const CommandBuffer& p_command_buffer, const BufferGPU& p_gpu, const BufferHost& p_host)
	{
		BufferCommandUtils::cmd_copy_buffer(p_command_buffer, p_gpu.buffer, p_gpu.size, p_host.buffer, p_host.size);
	};


	template<class ShadowImage(Source), class ShadowImage(Target)>
	inline void BufferCommandUtils::cmd_copy_shadowimage_to_shadowimage(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ShadowImage(Source)& p_source, const ShadowImage(Target)& p_target)
	{

#if CONTAINER_MEMORY_TEST
		assert_true(p_source.ShadowImage_member_format.extent == p_target.ShadowImage_member_format.extent);
#endif

		{
			BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_source, p_source.ShadowImage_member_format.imageUsage, ImageUsageFlag::TRANSFER_READ);
			BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_target, p_target.ShadowImage_member_format.imageUsage, ImageUsageFlag::TRANSFER_WRITE);
		}

		VkImageCopy l_region = {
				VkImageSubresourceLayers{ p_source.ShadowImage_member_format.imageAspect, 0, 0, (uint32_t)p_source.ShadowImage_member_format.arrayLayers },
				VkOffset3D{ 0, 0, 0 },
				VkImageSubresourceLayers{ p_target.ShadowImage_member_format.imageAspect, 0, 0, (uint32_t)p_target.ShadowImage_member_format.arrayLayers },
				VkOffset3D{ 0, 0, 0 },
				VkExtent3D{ (uint32_t)p_source.ShadowImage_member_format.extent.x, (uint32_t)p_source.ShadowImage_member_format.extent.y, (uint32_t)p_source.ShadowImage_member_format.extent.z }
		};

		vkCmdCopyImage(p_command_buffer.command_buffer, p_source.ShadowImage_member_image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				p_target.ShadowImage_member_image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &l_region);

		{
			BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_source, ImageUsageFlag::TRANSFER_READ, p_source.ShadowImage_member_format.imageUsage);
			BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_target, ImageUsageFlag::TRANSFER_WRITE, p_target.ShadowImage_member_format.imageUsage);
		}
	};

	inline void BufferCommandUtils::cmd_copy_buffer_host_to_image_gpu(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const BufferHost& p_host, const ImageGPU& p_gpu)
	{
#if CONTAINER_MEMORY_TEST
		assert_true(p_host.size <= p_gpu.size);
#endif

		BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_gpu, p_gpu.ShadowImage_member_format.imageUsage, ImageUsageFlag::TRANSFER_WRITE);

		VkBufferImageCopy l_buffer_image_copy{};
		l_buffer_image_copy.imageSubresource = VkImageSubresourceLayers{
				p_gpu.format.imageAspect,
				0,
				0,
				(uint32_t)p_gpu.format.arrayLayers };
		l_buffer_image_copy.imageExtent = VkExtent3D{ (uint32_t)p_gpu.format.extent.x, (uint32_t)p_gpu.format.extent.y, (uint32_t)p_gpu.format.extent.z };

		vkCmdCopyBufferToImage(p_command_buffer.command_buffer, p_host.buffer, p_gpu.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &l_buffer_image_copy);

		BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_gpu, ImageUsageFlag::TRANSFER_WRITE, p_gpu.format.imageUsage);
	};

	inline void BufferCommandUtils::cmd_copy_image_gpu_to_buffer_host(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ImageGPU& p_gpu, const BufferHost& p_host)
	{

#if CONTAINER_MEMORY_TEST
		assert_true(p_gpu.size <= p_host.size);
#endif

		BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_gpu, p_gpu.format.imageUsage, ImageUsageFlag::TRANSFER_READ);

		VkBufferImageCopy l_buffer_image_copy{};
		l_buffer_image_copy.imageSubresource = VkImageSubresourceLayers{
				p_gpu.format.imageAspect,
				0,
				0,
				(uint32_t)p_gpu.format.arrayLayers };
		l_buffer_image_copy.imageExtent = VkExtent3D{ (uint32_t)p_gpu.format.extent.x, (uint32_t)p_gpu.format.extent.y, (uint32_t)p_gpu.format.extent.z };

		vkCmdCopyImageToBuffer(p_command_buffer.command_buffer, p_gpu.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, p_host.buffer, 1, &l_buffer_image_copy);

		BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_gpu, ImageUsageFlag::TRANSFER_READ, p_gpu.format.imageUsage);
	};


	inline void BufferCommandUtils::cmd_image_layout_transition(const CommandBuffer& p_command_buffer, const VkImage p_image, const ImageFormat& p_image_format,
			const VkImageLayout p_source_image_layout, const VkImageLayout p_target_image_layout, const ImageLayoutTransitionBarrierConfiguration& p_lyaout_transition_configuration)
	{
		VkImageMemoryBarrier l_image_memory_barrier{};
		l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		l_image_memory_barrier.oldLayout = p_source_image_layout;
		l_image_memory_barrier.newLayout = p_target_image_layout;
		l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		l_image_memory_barrier.image = p_image;
		l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{
				p_image_format.imageAspect,
				0,
				(uint32_t)p_image_format.arrayLayers,
				0,
				(uint32_t)p_image_format.arrayLayers };

		l_image_memory_barrier.srcAccessMask = p_lyaout_transition_configuration.src_access_mask;
		l_image_memory_barrier.dstAccessMask = p_lyaout_transition_configuration.dst_access_mask;

		vkCmdPipelineBarrier(p_command_buffer.command_buffer, p_lyaout_transition_configuration.src_stage, p_lyaout_transition_configuration.dst_stage, 0, 0, NULL, 0, NULL,
				1, &l_image_memory_barrier);
	};


	template<class ShadowImage(_)>
	inline void BufferCommandUtils::cmd_image_layout_transition_v2(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ShadowImage(_)& p_image,
			const ImageUsageFlag p_source_image_usage, const ImageUsageFlag p_target_image_usage)
	{
		VkImageMemoryBarrier l_image_memory_barrier{};
		l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		l_image_memory_barrier.oldLayout = ImageLayoutTransitionBarriers::get_imagelayout_from_imageusage(p_source_image_usage);
		l_image_memory_barrier.newLayout = ImageLayoutTransitionBarriers::get_imagelayout_from_imageusage(p_target_image_usage);
		l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		l_image_memory_barrier.image = p_image.ShadowImage_member_image;

		l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{
				p_image.ShadowImage_member_format.imageAspect,
				0,
				(uint32_t)p_image.ShadowImage_member_format.arrayLayers,
				0,
				(uint32_t)p_image.ShadowImage_member_format.arrayLayers };

		ImageLayoutTransitionBarrierConfiguration l_layout_barrier = p_barriers.get_barrier(p_source_image_usage, p_target_image_usage);

		l_image_memory_barrier.srcAccessMask = l_layout_barrier.src_access_mask;
		l_image_memory_barrier.dstAccessMask = l_layout_barrier.dst_access_mask;

		vkCmdPipelineBarrier(p_command_buffer.command_buffer, l_layout_barrier.src_stage, l_layout_barrier.dst_stage, 0, 0, NULL, 0, NULL,
				1, &l_image_memory_barrier);
	};

	inline void BufferCommandUtils::cmd_copy_buffer(const CommandBuffer& p_command_buffer, const VkBuffer p_source_buffer, const uimax p_source_size, const VkBuffer p_target_buffer, const uimax p_target_size)
	{
#if CONTAINER_MEMORY_TEST
		assert_true(p_source_size <= p_target_size);
#endif

		VkBufferCopy l_buffer_copy{};
		l_buffer_copy.size = p_source_size;
		vkCmdCopyBuffer(p_command_buffer.command_buffer,
				p_source_buffer,
				p_target_buffer,
				1,
				&l_buffer_copy
		);
	};


}
