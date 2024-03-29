#pragma once

/*
    Allocates huge chunk of GPU memory and cut slice into it based on vulkan constraint.
*/
struct HeapPagedGPU
{
    struct MemoryGPU
    {
        gpu::DeviceMemory gpu_memory;
        int8* mapped_memory;

        inline static MemoryGPU allocate(const gpu::LogicalDevice p_transfer_device, const gpu::PhysicalDeviceMemoryIndex p_memory_type_index, const uimax p_memory_size)
        {
            MemoryGPU l_heap_gpu;
            l_heap_gpu.gpu_memory = gpu::allocate_memory(p_transfer_device, p_memory_size, p_memory_type_index);
            l_heap_gpu.mapped_memory = NULL;
            return l_heap_gpu;
        };

        inline void free(const gpu::LogicalDevice p_transfer_device)
        {
            gpu::free_memory(p_transfer_device, this->gpu_memory);
        };

        inline void map(const gpu::LogicalDevice p_transfer_device, const uimax p_memory_size)
        {
            this->mapped_memory = gpu::map_memory(p_transfer_device, this->gpu_memory, 0, p_memory_size);
        };
    };

    HeapPaged heap;
    Vector<MemoryGPU> gpu_memories;
    int8 is_memory_mapped;

    inline static HeapPagedGPU allocate_default(const int8 is_memory_mapped, const gpu::LogicalDevice p_transfer_device, const uimax p_memory_chunk_size)
    {
        return HeapPagedGPU{HeapPaged::allocate_default(p_memory_chunk_size), Vector<MemoryGPU>::allocate(0), is_memory_mapped};
    };

    inline void free(const gpu::LogicalDevice p_transfer_device)
    {
        for (loop(i, 0, this->gpu_memories.Size))
        {
            this->gpu_memories.get(i).free(p_transfer_device);
        }
        this->gpu_memories.free();
        this->heap.free();
    };

    inline int8 allocate_element(const uimax p_size, const uimax p_modulo_offset, gpu::LogicalDevice p_transfer_device, const gpu::PhysicalDeviceMemoryIndex p_memory_type_index,
                                 HeapPagedToken* out_chunk)
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

    inline void release_element(const HeapPagedToken& p_token)
    {
        this->heap.release_element(p_token);
    };
};

struct TransferDeviceHeapToken
{
    gpu::HeapIndex heap_index;
    HeapPagedToken heap_paged_token;
};

/*
    Holds HeapPagedGPU based on the memory_type value.
    Vulkan allows to allocate memory based on their type.
*/
struct TransferDeviceHeap
{
    Span<HeapPagedGPU> gpu_heaps;

    inline static TransferDeviceHeap allocate_default(const GraphicsCard& p_graphics_card, const gpu::LogicalDevice p_transfer_device)
    {
        TransferDeviceHeap l_heap;
        l_heap.gpu_heaps = Span<HeapPagedGPU>::allocate(p_graphics_card.device_memory_properties.memory_heaps_size);
        for (loop(i, 0, p_graphics_card.device_memory_properties.memory_heaps_size))
        {
            int8 l_is_memory_mapped = 0;
            for (loop(j, 0, p_graphics_card.device_memory_properties.memory_types_size))
            {
                const gpu::MemoryType& l_memory_type = p_graphics_card.device_memory_properties.memory_types.get(j);
                if (l_memory_type.heap_index.index == i)
                {
                    if (((gpu::MemoryTypeFlag_t)l_memory_type.type & (gpu::MemoryTypeFlag_t)gpu::MemoryTypeFlag::HOST_VISIBLE) ||
                        ((gpu::MemoryTypeFlag_t)l_memory_type.type & (gpu::MemoryTypeFlag_t)gpu::MemoryTypeFlag::HOST_COHERENT) ||
                        ((gpu::MemoryTypeFlag_t)l_memory_type.type & (gpu::MemoryTypeFlag_t)gpu::MemoryTypeFlag::HOST_VISIBLE))
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

    inline void free(const gpu::LogicalDevice p_transfer_device)
    {
        for (loop(i, 0, this->gpu_heaps.Capacity))
        {
            this->gpu_heaps.get(i).free(p_transfer_device);
        };
        this->gpu_heaps.free();
    };

    inline int8 allocate_element(const GraphicsCard& p_graphics_card, const gpu::LogicalDevice p_transfer_device, const gpu::MemoryRequirements& p_requirements,
                                 const gpu::MemoryTypeFlag p_memory_type, TransferDeviceHeapToken* out_token)
    {
        // We make sure that memory have an allocated heap size of max(p_requirements.size, p_requirements.alignment)
        uimax l_aligned_size = p_requirements.size;
        if (l_aligned_size < p_requirements.alignment)
        {
            l_aligned_size = p_requirements.alignment;
        };

        gpu::PhysicalDeviceMemoryIndex l_memory_type_index = p_graphics_card.get_memory_type_index(p_requirements.memory_type, p_memory_type);
        out_token->heap_index = p_graphics_card.device_memory_properties.memory_types.get(l_memory_type_index.index).heap_index;
        return this->gpu_heaps.get(out_token->heap_index.index).allocate_element(l_aligned_size, p_requirements.alignment, p_transfer_device, l_memory_type_index, &out_token->heap_paged_token);
    };

    inline void release_element(const TransferDeviceHeapToken& p_memory)
    {
        this->gpu_heaps.get(p_memory.heap_index.index).release_element(p_memory.heap_paged_token);
    };

    inline Slice<int8> get_element_as_slice(const TransferDeviceHeapToken& p_token)
    {
        HeapPagedGPU& l_heap = this->gpu_heaps.get(p_token.heap_index.index);
        SliceIndex* l_slice_index = l_heap.heap.get_sliceindex_only(p_token.heap_paged_token);
        return Slice<int8>::build_memory_offset_elementnb(l_heap.gpu_memories.get(p_token.heap_paged_token.PageIndex).mapped_memory, l_slice_index->Begin, l_slice_index->Size);
    };

    inline SliceOffset<int8> get_element_gcmemory_and_offset(const TransferDeviceHeapToken& p_token)
    {
        HeapPagedGPU& l_heap = this->gpu_heaps.get(p_token.heap_index.index);
        return SliceOffset<int8>::build_from_sliceindex((int8*)token_value(l_heap.gpu_memories.get(p_token.heap_paged_token.PageIndex).gpu_memory), *l_heap.heap.get_sliceindex_only(p_token.heap_paged_token));
    };
};

/*
    A TransferDevice is a logical instance of the GraphicsCard.
    It holds the command buffer that will execute all commands related to memory allocation or copy
*/
struct TransferDevice
{
    GraphicsCard graphics_card;
    gpu::LogicalDevice device;
    gpu::Queue transfer_queue;

    TransferDeviceHeap heap;
    CommandPool command_pool;
    CommandBuffer command_buffer;

    inline static TransferDevice allocate(const GPUInstance& p_instance)
    {
        TransferDevice l_transfer_device;
        l_transfer_device.graphics_card = p_instance.graphics_card;
        l_transfer_device.device = p_instance.logical_device;

        l_transfer_device.transfer_queue = gpu::logical_device_get_queue(l_transfer_device.device, p_instance.graphics_card.transfer_queue_family);

        l_transfer_device.heap = TransferDeviceHeap::allocate_default(p_instance.graphics_card, l_transfer_device.device);

        l_transfer_device.command_pool = CommandPool::allocate(l_transfer_device.device, p_instance.graphics_card.transfer_queue_family);
        l_transfer_device.command_buffer = l_transfer_device.command_pool.allocate_command_buffer(l_transfer_device.device, l_transfer_device.transfer_queue);

        return l_transfer_device;
    };

    inline void free()
    {
        this->heap.free(this->device);
        this->command_pool.free_command_buffer(this->device, this->command_buffer);
        this->command_pool.free(this->device);
    };
};

enum class BufferIndexType
{
    UINT16 = 0,
    UINT32 = 1
};

struct MappedHostMemory
{
    Slice<int8> memory;

    inline static MappedHostMemory build_default()
    {
        return MappedHostMemory{Slice<int8>::build_begin_end(NULL, 0, 0)};
    };

    inline void map(TransferDevice& p_transfer_device, const TransferDeviceHeapToken& p_memory)
    {
        if (!this->is_mapped())
        {
            this->memory = p_transfer_device.heap.get_element_as_slice(p_memory);
        }
    };

    inline void unmap(TransferDevice& p_device)
    {
        if (this->is_mapped())
        {
            *this = MappedHostMemory::build_default();
        }
    };

    inline void copy_from(const Slice<int8>& p_from)
    {
        this->memory.copy_memory(p_from);
    };

    inline int8 is_mapped()
    {
        return this->memory.Begin != NULL;
    };
};

template <class _Buffer> struct iBuffer
{
    const _Buffer& buffer;

    inline gpu::Buffer get_buffer() const
    {
        return this->buffer.buffer;
    };

    inline uimax get_size() const
    {
        return this->buffer.size;
    }
};

/*
    A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the CPU.
*/
struct BufferHost
{
    MappedHostMemory memory;
    TransferDeviceHeapToken heap_token;
    gpu::Buffer buffer;
    uimax size;

    inline static BufferHost allocate(TransferDevice& p_transfer_device, const uimax p_buffer_size, const BufferUsageFlag p_usage_flags)
    {
        BufferHost l_buffer_host;

        l_buffer_host.size = p_buffer_size;
        l_buffer_host.memory = MappedHostMemory::build_default();

        l_buffer_host.buffer = gpu::buffer_allocate(p_transfer_device.device, p_buffer_size, p_usage_flags);

        gpu::MemoryRequirements l_memory_requirements = gpu::buffer_get_memory_requirements(p_transfer_device.device, l_buffer_host.buffer);
        p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_memory_requirements, gpu::MemoryTypeFlag::HOST_VISIBLE, &l_buffer_host.heap_token);
        l_buffer_host.memory.map(p_transfer_device, l_buffer_host.heap_token);
        l_buffer_host.bind(p_transfer_device);

        return l_buffer_host;
    };

    inline void free(TransferDevice& p_transfer_device)
    {
        if (this->memory.is_mapped())
        {
            this->unmap(p_transfer_device);
        }

        p_transfer_device.heap.release_element(this->heap_token);
        gpu::buffer_destroy(p_transfer_device.device, this->buffer);
        this->buffer = token_build_default<gpu::_Buffer>();
    };

    inline void push(const Slice<int8>& p_from)
    {
        this->memory.copy_from(p_from);
    };

    // /!\ WARNING - The mapped memory slice is the slice of the WHOLE allocated GPU memory.
    // Depending on GPU device, effectively allocated GPU memory may be higher that the initial requested Size.
    // Thus, casting on the returned slice may cause crash.
    // To have a slice that is representative of the asked allocation size, use get_mapped_effective_memory
    inline Slice<int8>& get_mapped_memory()
    {
        return this->memory.memory;
    };

    inline Slice<int8> get_mapped_effective_memory()
    {
        Slice<int8> l_return = this->memory.memory;
        l_return.Size = this->size;
        return l_return;
    };

  private:
    inline void bind(TransferDevice& p_transfer_device)
    {
#if __DEBUG
        assert_true(this->memory.is_mapped());
#endif
        SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
        gpu::buffer_bind_memory(p_transfer_device.device, this->buffer, token_build<gpu::_DeviceMemory>((token_t)l_memory.Memory), l_memory.Offset);
    };

    inline void map(TransferDevice& p_transfer_device)
    {
        this->memory.map(p_transfer_device, this->heap_token);
    };

    inline void unmap(TransferDevice& p_transfer_device)
    {
        this->memory.unmap(p_transfer_device);
    };
};

/*
    A BufferHost is a pointer to a HeapPaged chunk that have been specified to be visible by the GPU.
*/
struct BufferGPU
{
    TransferDeviceHeapToken heap_token;
    gpu::Buffer buffer;
    uimax size;

    inline static BufferGPU allocate(TransferDevice& p_transfer_device, const uimax p_size, const BufferUsageFlag p_usage_flags)
    {
        BufferGPU l_buffer_gpu;

        l_buffer_gpu.size = p_size;

        l_buffer_gpu.buffer = gpu::buffer_allocate(p_transfer_device.device, p_size, p_usage_flags);
        gpu::MemoryRequirements l_gpu_requirements = gpu::buffer_get_memory_requirements(p_transfer_device.device, l_buffer_gpu.buffer);

        p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_gpu_requirements, gpu::MemoryTypeFlag::DEVICE_LOCAL, &l_buffer_gpu.heap_token);
        l_buffer_gpu.bind(p_transfer_device);

        return l_buffer_gpu;
    };

    inline void free(TransferDevice& p_transfer_device)
    {
        p_transfer_device.heap.release_element(this->heap_token);
        gpu::buffer_destroy(p_transfer_device.device, this->buffer);
        this->buffer = token_build_default<gpu::_Buffer>();
    };

  private:
    inline void bind(TransferDevice& p_transfer_device)
    {
        SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
        gpu::buffer_bind_memory(p_transfer_device.device, this->buffer, token_build<gpu::_DeviceMemory>((token_t)l_memory.Memory), l_memory.Offset);
    };
};

template <class _Image> struct iImage
{
    _Image& image;

    inline const gpu::Image get_image() const
    {
        return this->image.image;
    };

    inline const ImageFormat& get_image_format() const
    {
        return this->image.format;
    };
};

struct ImageHost
{
    MappedHostMemory memory;
    TransferDeviceHeapToken heap_token;
    gpu::Image image;
    ImageFormat format;

    inline static ImageHost allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format)
    {
        ImageHost l_image_host;
        l_image_host.memory = MappedHostMemory::build_default();
        l_image_host.format = p_image_format;
        // l_image_host.target_image_layout = ImageUsageUtils::get_target_imagelayout_from_usage(l_image_host.format.imageUsage);

        // gpu::ImageTilingFlag::LINEAR is mandatory for host readable image
        l_image_host.image = gpu::image_allocate(p_transfer_device.device, p_image_format, gpu::ImageTilingFlag::LINEAR);

        gpu::MemoryRequirements l_gpu_requirements = gpu::image_get_memory_requirements(p_transfer_device.device, l_image_host.image);

        p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_gpu_requirements, gpu::MemoryTypeFlag::HOST_VISIBLE, &l_image_host.heap_token);
        // p_transfer_device.heap.allocate_host_write_element(p_transfer_device.device, l_requirements.size, l_requirements.alignment, &l_image_host.heap_token);
        l_image_host.memory.map(p_transfer_device, l_image_host.heap_token);

        l_image_host.map(p_transfer_device);
        l_image_host.bind(p_transfer_device);

        return l_image_host;
    };

    inline void free(TransferDevice& p_transfer_device)
    {
        if (this->memory.is_mapped())
        {
            this->unmap(p_transfer_device);
        };

        gpu::image_destroy(p_transfer_device.device, this->image);
        p_transfer_device.heap.release_element(this->heap_token);
        // p_transfer_device.heap.release_host_write_element(this->heap_token);
    };

    inline void push(const Slice<int8>& p_from)
    {
        this->memory.memory.copy_memory(p_from);
    };

    inline Slice<int8>& get_mapped_memory()
    {
        return this->memory.memory;
    };

  private:
    inline void map(TransferDevice& p_transfer_device)
    {
        this->memory.map(p_transfer_device, this->heap_token);
    };

    inline void unmap(TransferDevice& p_transfer_device)
    {
        this->memory.unmap(p_transfer_device);
    };

    inline void bind(TransferDevice& p_transfer_device)
    {
#if __DEBUG
        assert_true(this->memory.is_mapped());
#endif
        SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
        // SliceOffset<int8> l_memory = p_transfer_device.heap.get_host_write_gcmemory_and_offset(this->heap_token);
        gpu::image_bind_memory(p_transfer_device.device, this->image, token_build<gpu::_DeviceMemory>((token_t)l_memory.Memory), l_memory.Offset);
    };
};

struct ImageGPU
{
    TransferDeviceHeapToken heap_token;
    gpu::Image image;
    ImageFormat format;
    uimax size;

    inline static ImageGPU allocate(TransferDevice& p_transfer_device, const ImageFormat& p_image_format)
    {
        ImageGPU l_image_gpu;
        l_image_gpu.format = p_image_format;
        // l_image_gpu.target_image_layout = ImageUsageUtils::get_target_imagelayout_from_usage(l_image_gpu.format.imageUsage);

        l_image_gpu.image = gpu::image_allocate(p_transfer_device.device, p_image_format, gpu::ImageTilingFlag::OPTIMAL);

        gpu::MemoryRequirements l_gpu_requirements = gpu::image_get_memory_requirements(p_transfer_device.device, l_image_gpu.image);
        l_image_gpu.size = l_gpu_requirements.size;

        p_transfer_device.heap.allocate_element(p_transfer_device.graphics_card, p_transfer_device.device, l_gpu_requirements, gpu::MemoryTypeFlag::DEVICE_LOCAL, &l_image_gpu.heap_token);

        l_image_gpu.bind(p_transfer_device);

        return l_image_gpu;
    };

    inline void free(TransferDevice& p_transfer_device)
    {
        gpu::image_destroy(p_transfer_device.device, this->image);
        p_transfer_device.heap.release_element(this->heap_token);
    };

  private:
    inline void bind(TransferDevice& p_transfer_device)
    {
        SliceOffset<int8> l_memory = p_transfer_device.heap.get_element_gcmemory_and_offset(this->heap_token);
        gpu::image_bind_memory(p_transfer_device.device, this->image, token_build<gpu::_DeviceMemory>((token_t)l_memory.Memory), l_memory.Offset);
    };
};

struct ImageLayoutTransitionBarrierConfiguration
{
    GPUAccessFlag src_access_mask;
    GPUPipelineStageFlag src_stage;

    GPUAccessFlag dst_access_mask;
    GPUPipelineStageFlag dst_stage;
};

namespace ImageLayoutTransitionBarriers_const
{
static uint8 ImageUsageCount = 6;
};

struct ImageLayoutTransitionBarriers
{
    Span<ImageLayoutTransitionBarrierConfiguration> barriers;

    inline static ImageLayoutTransitionBarriers allocate()
    {
        const ImageLayoutTransitionBarrierConfiguration undefined_to_transfert_dst =
            ImageLayoutTransitionBarrierConfiguration{GPUAccessFlag::UNKNOWN, GPUPipelineStageFlag::TOP_OF_PIPE, GPUAccessFlag::TRANSFER_WRITE, GPUPipelineStageFlag::TRANSFER};
        const ImageLayoutTransitionBarrierConfiguration undefined_to_transfert_src =
            ImageLayoutTransitionBarrierConfiguration{GPUAccessFlag::UNKNOWN, GPUPipelineStageFlag::TOP_OF_PIPE, GPUAccessFlag::TRANSFER_READ, GPUPipelineStageFlag::TRANSFER};
        const ImageLayoutTransitionBarrierConfiguration transfer_src_to_shader_readonly =
            ImageLayoutTransitionBarrierConfiguration{GPUAccessFlag::TRANSFER_READ, GPUPipelineStageFlag::TRANSFER, GPUAccessFlag::SHADER_READ, GPUPipelineStageFlag::FRAGMENT_SHADER};
        const ImageLayoutTransitionBarrierConfiguration transfer_dst_to_shader_readonly =
            ImageLayoutTransitionBarrierConfiguration{GPUAccessFlag::TRANSFER_WRITE, GPUPipelineStageFlag::TRANSFER, GPUAccessFlag::SHADER_READ, GPUPipelineStageFlag::FRAGMENT_SHADER};
        const ImageLayoutTransitionBarrierConfiguration undefined_to_shader_readonly =
            ImageLayoutTransitionBarrierConfiguration{GPUAccessFlag::UNKNOWN, GPUPipelineStageFlag::TOP_OF_PIPE, GPUAccessFlag::SHADER_READ, GPUPipelineStageFlag::FRAGMENT_SHADER};
        const ImageLayoutTransitionBarrierConfiguration colorattachement_to_shader_readonly = ImageLayoutTransitionBarrierConfiguration{
            GPUAccessFlag::COLOR_ATTACHMENT_WRITE, GPUPipelineStageFlag::COLOR_ATTACHMENT_OUTPUT, GPUAccessFlag::SHADER_READ, GPUPipelineStageFlag::FRAGMENT_SHADER};
        const ImageLayoutTransitionBarrierConfiguration shader_readonly_to_colorattachement = ImageLayoutTransitionBarrierConfiguration{
            GPUAccessFlag::SHADER_READ, GPUPipelineStageFlag::FRAGMENT_SHADER, GPUAccessFlag::COLOR_ATTACHMENT_WRITE, GPUPipelineStageFlag::COLOR_ATTACHMENT_OUTPUT};

        ImageLayoutTransitionBarriers l_barriers = ImageLayoutTransitionBarriers{
            Span<ImageLayoutTransitionBarrierConfiguration>::callocate(ImageLayoutTransitionBarriers_const::ImageUsageCount * ImageLayoutTransitionBarriers_const::ImageUsageCount)};

        // Undefined to *
        l_barriers.barriers.get(1) = undefined_to_transfert_src;
        l_barriers.barriers.get(2) = undefined_to_transfert_dst;
        l_barriers.barriers.get(3) = undefined_to_shader_readonly;
        l_barriers.barriers.get(4) = undefined_to_shader_readonly;
        l_barriers.barriers.get(5) = undefined_to_shader_readonly;

        // Transfer_src to *
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 1) = undefined_to_transfert_src;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 2) = undefined_to_transfert_dst;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 3) = transfer_src_to_shader_readonly;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 4) = transfer_src_to_shader_readonly;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 1) + 5) = transfer_src_to_shader_readonly;

        // Transfer_dst to *
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 1) = undefined_to_transfert_dst;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 2) = undefined_to_transfert_src;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 3) = transfer_dst_to_shader_readonly;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 4) = transfer_dst_to_shader_readonly;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 2) + 5) = transfer_dst_to_shader_readonly;

        // color_attachment to *
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 3) + 1) = undefined_to_transfert_src;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 3) + 2) = undefined_to_transfert_dst;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 3) + 5) = colorattachement_to_shader_readonly;

        // depth_attachment to *
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 4) + 1) = undefined_to_transfert_src;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 4) + 2) = undefined_to_transfert_dst;

        // shader_readonly to *
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 5) + 1) = undefined_to_transfert_src;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 5) + 2) = undefined_to_transfert_dst;
        l_barriers.barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * 5) + 3) = shader_readonly_to_colorattachement;

        return l_barriers;
    };

    inline ImageLayoutTransitionBarrierConfiguration get_barrier(const ImageUsageFlag p_left, const ImageUsageFlag p_right) const
    {
        return this->barriers.get(((ImageLayoutTransitionBarriers_const::ImageUsageCount - 1) * this->get_index_from_imageusage(p_left)) + this->get_index_from_imageusage(p_right));
    };

    inline static ImageLayoutFlag get_imagelayout_from_imageusage(const ImageUsageFlag p_flag)
    {
        // /!\ The order matter !!!
        // Because the ImageUsageFlag can be a combinaison of multiple values,  SHADER_COLOR_ATTACHMENT, SHADER_DEPTH_ATTACHMENT and SHADER_TEXTURE_PARAMETER are exclusive but can be combined
        // with either TRANSFER_READ or TRANSFER_WRITE.

        if ((p_flag & ImageUsageFlag::SHADER_COLOR_ATTACHMENT) != ImageUsageFlag::UNKNOWN)
        {
            return ImageLayoutFlag::COLOR_ATTACHMENT;
        }
        else if ((p_flag & ImageUsageFlag::SHADER_DEPTH_ATTACHMENT) != ImageUsageFlag::UNKNOWN)
        {
            return ImageLayoutFlag::DEPTH_STENCIL_ATTACHMENT;
        }
        else if ((p_flag & ImageUsageFlag::SHADER_TEXTURE_PARAMETER) != ImageUsageFlag::UNKNOWN)
        {
            return ImageLayoutFlag::SHADER_READ_ONLY;
        }

        else if ((p_flag & ImageUsageFlag::TRANSFER_READ) != ImageUsageFlag::UNKNOWN)
        {
            return ImageLayoutFlag::TRANSFER_SRC;
        }
        else if ((p_flag & ImageUsageFlag::TRANSFER_WRITE) != ImageUsageFlag::UNKNOWN)
        {
            return ImageLayoutFlag::TRANSFER_DST;
        }

        return ImageLayoutFlag::UNKNOWN;
    };

    inline void free()
    {
        this->barriers.free();
    };

  private:
    inline uint8 get_index_from_imageusage(const ImageUsageFlag p_flag) const
    {
        if ((p_flag & ImageUsageFlag::SHADER_COLOR_ATTACHMENT) != ImageUsageFlag::UNKNOWN)
        {
            return 3;
        }
        else if ((p_flag & ImageUsageFlag::SHADER_DEPTH_ATTACHMENT) != ImageUsageFlag::UNKNOWN)
        {
            return 4;
        }
        else if ((p_flag & ImageUsageFlag::SHADER_TEXTURE_PARAMETER) != ImageUsageFlag::UNKNOWN)
        {
            return 5;
        }

        /*
          This is very importtant to have the Transfer conditions after the other because a usage can be (SOMETHING | TRANSFER).
          In that case, we want to return the index linked to "SOMETHING" usage.
        */
        else if ((p_flag & ImageUsageFlag::TRANSFER_READ) != ImageUsageFlag::UNKNOWN)
        {
            return 1;
        }
        else if ((p_flag & ImageUsageFlag::TRANSFER_WRITE) != ImageUsageFlag::UNKNOWN)
        {
            return 2;
        }

        return 0;
    };
};

/*
    The BufferAllocator allocates and holds token of vulkan buffer objects.
*/
struct BufferAllocator
{
    TransferDevice device;
    ImageLayoutTransitionBarriers image_layout_barriers;

    Pool<BufferHost> host_buffers;
    Pool<BufferGPU> gpu_buffers;
    Pool<ImageHost> host_images;
    Pool<ImageGPU> gpu_images;

    inline static BufferAllocator allocate_default(const GPUInstance& p_instance)
    {
        return BufferAllocator{TransferDevice::allocate(p_instance), ImageLayoutTransitionBarriers::allocate(),
                               Pool<BufferHost>::allocate(0),        Pool<BufferGPU>::allocate(0),
                               Pool<ImageHost>::allocate(0),         Pool<ImageGPU>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->host_buffers.has_allocated_elements());
        assert_true(!this->gpu_buffers.has_allocated_elements());
        assert_true(!this->host_images.has_allocated_elements());
        assert_true(!this->gpu_images.has_allocated_elements());
#endif

        this->image_layout_barriers.free();

        this->host_buffers.free();
        this->gpu_buffers.free();
        this->host_images.free();
        this->gpu_images.free();
        this->device.free();
    };

    inline Token<BufferHost> allocate_bufferhost(const Slice<int8>& p_value, const BufferUsageFlag p_usage_flags)
    {
        BufferHost l_buffer = BufferHost::allocate(this->device, p_value.Size, p_usage_flags);
        l_buffer.push(p_value);
        return this->host_buffers.alloc_element(l_buffer);
    };

    inline Token<BufferHost> allocate_bufferhost_empty(const uimax p_size, const BufferUsageFlag p_usage_flags)
    {
        return this->host_buffers.alloc_element(BufferHost::allocate(this->device, p_size, p_usage_flags));
    };

    inline void free_bufferhost(const Token<BufferHost> p_buffer_host)
    {
        BufferHost& l_buffer = this->host_buffers.get(p_buffer_host);
        l_buffer.free(this->device);
        this->host_buffers.release_element(p_buffer_host);
    };

    inline Token<BufferGPU> allocate_buffergpu(const uimax p_size, const BufferUsageFlag p_usage_flags)
    {
        return this->gpu_buffers.alloc_element(BufferGPU::allocate(this->device, p_size, p_usage_flags));
    };

    inline void free_buffergpu(const Token<BufferGPU> p_buffer_gpu)
    {
        BufferGPU& l_buffer = this->gpu_buffers.get(p_buffer_gpu);
        l_buffer.free(this->device);
        this->gpu_buffers.release_element(p_buffer_gpu);
    };

    inline Token<ImageHost> allocate_imagehost(const Slice<int8>& p_value, const ImageFormat& p_image_format)
    {
        ImageHost l_image_host = ImageHost::allocate(this->device, p_image_format);
        l_image_host.push(p_value);
        return this->host_images.alloc_element(l_image_host);
    };

    inline Token<ImageHost> allocate_imagehost_empty(const ImageFormat& p_image_format)
    {
        return this->host_images.alloc_element(ImageHost::allocate(this->device, p_image_format));
    };

    inline void free_imagehost(const Token<ImageHost> p_image_host)
    {
        ImageHost& l_image_host = this->host_images.get(p_image_host);
        l_image_host.free(this->device);
        this->host_images.release_element(p_image_host);
    };

    inline Token<ImageGPU> allocate_imagegpu(const ImageFormat& p_image_format)
    {
        return this->gpu_images.alloc_element(ImageGPU::allocate(this->device, p_image_format));
    };

    inline void free_imagegpu(const Token<ImageGPU> p_image_gpu)
    {
        ImageGPU& l_image = this->gpu_images.get(p_image_gpu);
        l_image.free(this->device);
        this->gpu_images.release_element(p_image_gpu);
    };
};

/*
    The BufferEvents stores all GPU operations that are deferred to be executed on a command buffer.
    This includes :
        1/ Image host and gpu allocation (because they need image layout transition)
        2/ Writing buffer host to gpu
        3/ Writing buffer gpu to host

*/
struct BufferEvents
{
    Vector<Token<BufferHost>> garbage_host_buffers;

    struct WriteBufferHostToBufferGPU
    {
        Token<BufferHost> source_buffer;
        int8 source_buffer_dispose;
        Token<BufferGPU> target_buffer;
    };

    Vector<WriteBufferHostToBufferGPU> write_buffer_host_to_buffer_gpu_events;

    struct WriteBufferGPUToBufferHost
    {
        Token<BufferGPU> source_buffer;
        Token<BufferHost> target_buffer;
    };

    Vector<WriteBufferGPUToBufferHost> write_buffer_gpu_to_buffer_host_events;

    struct AllocatedImageHost
    {
        Token<ImageHost> image;
    };

    Vector<AllocatedImageHost> image_host_allocate_events;

    struct AllocatedImageGPU
    {
        Token<ImageGPU> image;
    };

    Vector<AllocatedImageGPU> image_gpu_allocate_events;

    struct WriteBufferHostToImageGPU
    {
        Token<BufferHost> source_buffer;
        int8 source_buffer_dispose;
        Token<ImageGPU> target_image;
    };

    Vector<WriteBufferHostToImageGPU> write_buffer_host_to_image_gpu_events;

    struct WriteImageGPUToBufferHost
    {
        Token<ImageGPU> source_image;
        Token<BufferHost> target_buffer;
    };

    Vector<WriteImageGPUToBufferHost> write_image_gpu_to_buffer_host_events;

    inline static BufferEvents allocate()
    {
        return BufferEvents{Vector<Token<BufferHost>>::allocate(0),        Vector<WriteBufferHostToBufferGPU>::allocate(0), Vector<WriteBufferGPUToBufferHost>::allocate(0),
                            Vector<AllocatedImageHost>::allocate(0),       Vector<AllocatedImageGPU>::allocate(0),          Vector<WriteBufferHostToImageGPU>::allocate(0),
                            Vector<WriteImageGPUToBufferHost>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->garbage_host_buffers.empty());
        assert_true(this->write_buffer_gpu_to_buffer_host_events.empty());
        assert_true(this->write_buffer_host_to_buffer_gpu_events.empty());
        assert_true(this->image_host_allocate_events.empty());
        assert_true(this->image_gpu_allocate_events.empty());
        assert_true(this->write_buffer_host_to_image_gpu_events.empty());
        assert_true(this->write_image_gpu_to_buffer_host_events.empty());
#endif

        this->garbage_host_buffers.free();
        this->write_buffer_gpu_to_buffer_host_events.free();
        this->write_buffer_host_to_buffer_gpu_events.free();
        this->image_host_allocate_events.free();
        this->image_gpu_allocate_events.free();
        this->write_buffer_host_to_image_gpu_events.free();
        this->write_image_gpu_to_buffer_host_events.free();
    };

    inline void remove_buffer_host_references(const Token<BufferHost> p_buffer_host)
    {
        for (vector_loop_reverse(&this->write_buffer_host_to_buffer_gpu_events, i))
        {
            WriteBufferHostToBufferGPU& l_event = this->write_buffer_host_to_buffer_gpu_events.get(i);
            if (token_equals(l_event.source_buffer, p_buffer_host))
            {
                // The source buffer is already supposed to be disposed
                this->write_buffer_host_to_buffer_gpu_events.erase_element_at_always(i);
            }
        }
        for (vector_loop_reverse(&this->write_buffer_gpu_to_buffer_host_events, i))
        {
            WriteBufferGPUToBufferHost& l_event = this->write_buffer_gpu_to_buffer_host_events.get(i);
            if (token_equals(l_event.target_buffer, p_buffer_host))
            {
                this->write_buffer_gpu_to_buffer_host_events.erase_element_at_always(i);
            }
        }
        for (vector_loop_reverse(&this->write_buffer_host_to_image_gpu_events, i))
        {
            WriteBufferHostToImageGPU& l_event = this->write_buffer_host_to_image_gpu_events.get(i);
            if (token_equals(l_event.source_buffer, p_buffer_host))
            {
                // The source buffer is already supposed to be disposed
                this->write_buffer_host_to_image_gpu_events.erase_element_at_always(i);
            }
        }
        for (vector_loop_reverse(&this->write_image_gpu_to_buffer_host_events, i))
        {
            WriteImageGPUToBufferHost& l_event = this->write_image_gpu_to_buffer_host_events.get(i);
            if (token_equals(l_event.target_buffer, p_buffer_host))
            {
                this->write_image_gpu_to_buffer_host_events.erase_element_at_always(i);
            }
        }
    };

    inline void remove_buffer_gpu_references(const Token<BufferGPU> p_buffer_gpu)
    {
        for (vector_loop_reverse(&this->write_buffer_gpu_to_buffer_host_events, i))
        {
            WriteBufferGPUToBufferHost& l_event = this->write_buffer_gpu_to_buffer_host_events.get(i);
            if (token_equals(l_event.source_buffer, p_buffer_gpu))
            {
                this->write_buffer_gpu_to_buffer_host_events.erase_element_at_always(i);
            }
        }

        for (vector_loop_reverse(&this->write_buffer_host_to_buffer_gpu_events, i))
        {
            WriteBufferHostToBufferGPU& l_event = this->write_buffer_host_to_buffer_gpu_events.get(i);
            if (token_equals(l_event.target_buffer, p_buffer_gpu))
            {
                if (l_event.source_buffer_dispose)
                {
                    this->garbage_host_buffers.push_back_element(l_event.source_buffer);
                }
                this->write_buffer_host_to_buffer_gpu_events.erase_element_at_always(i);
            }
        }
    };

    inline void remove_image_host_references(const Token<ImageHost> p_image_host)
    {
        for (vector_loop_reverse(&this->image_host_allocate_events, i))
        {
            AllocatedImageHost& l_event = this->image_host_allocate_events.get(i);
            if (token_equals(l_event.image, p_image_host))
            {
                this->image_host_allocate_events.erase_element_at_always(i);
            }
        }
    };

    inline void remove_image_gpu_references(const Token<ImageGPU> p_image_gpu)
    {
        for (vector_loop_reverse(&this->write_buffer_host_to_image_gpu_events, i))
        {
            WriteBufferHostToImageGPU& l_event = this->write_buffer_host_to_image_gpu_events.get(i);
            if (token_equals(l_event.target_image, p_image_gpu))
            {
                if (l_event.source_buffer_dispose)
                {
                    this->garbage_host_buffers.push_back_element(l_event.source_buffer);
                }
                this->write_buffer_host_to_image_gpu_events.erase_element_at_always(i);
            }
        }

        for (vector_loop_reverse(&this->write_image_gpu_to_buffer_host_events, i))
        {
            WriteImageGPUToBufferHost& l_event = this->write_image_gpu_to_buffer_host_events.get(i);
            if (token_equals(l_event.source_image, p_image_gpu))
            {
                this->write_image_gpu_to_buffer_host_events.erase_element_at_always(i);
            }
        }

        for (vector_loop_reverse(&this->image_gpu_allocate_events, i))
        {
            AllocatedImageGPU& l_event = this->image_gpu_allocate_events.get(i);
            if (token_equals(l_event.image, p_image_gpu))
            {
                this->image_gpu_allocate_events.erase_element_at_always(i);
            }
        }
    };
};

struct BufferCommandUtils
{
    inline static void cmd_copy_buffer_host_to_gpu(const CommandBuffer& p_command_buffer, const BufferHost& p_host, const BufferGPU& p_gpu)
    {
        BufferCommandUtils::cmd_copy_buffer(p_command_buffer, p_host.buffer, p_host.size, p_gpu.buffer, p_gpu.size);
    };

    inline static void cmd_copy_buffer_gpu_to_host(const CommandBuffer& p_command_buffer, const BufferGPU& p_gpu, const BufferHost& p_host)
    {
        BufferCommandUtils::cmd_copy_buffer(p_command_buffer, p_gpu.buffer, p_gpu.size, p_host.buffer, p_host.size);
    };

    template <class SourceImage, class TargetImage>
    inline static void cmd_copy_shadowimage_to_shadowimage(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const iImage<SourceImage> p_source,
                                                           const iImage<TargetImage> p_target)
    {

#if __DEBUG
        assert_true(p_source.get_image_format().extent == ShadowImage_c_get_format(&p_target).extent);
#endif

        {
            BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_source, p_source.get_image_format().imageUsage, ImageUsageFlag::TRANSFER_READ);
            BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_target, p_target.get_image().imageUsage, ImageUsageFlag::TRANSFER_WRITE);
        }

        gpu::command_copy_image(p_command_buffer.command_buffer, p_source.get_image(), p_source.get_image_format(), p_target.get_image(), p_target.get_image_format());

        {
            BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_source, ImageUsageFlag::TRANSFER_READ, p_source.get_image_format().imageUsage);
            BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, p_target, ImageUsageFlag::TRANSFER_WRITE, p_target.get_image_format().imageUsage);
        }
    };

    inline static void cmd_copy_buffer_host_to_image_gpu(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const BufferHost& p_host, const ImageGPU& p_gpu)
    {
#if __DEBUG
        assert_true(p_host.size <= p_gpu.size);
#endif

        BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, iImage<const ImageGPU>{p_gpu}, p_gpu.format.imageUsage, ImageUsageFlag::TRANSFER_WRITE);
        gpu::command_copy_buffer_to_image(p_command_buffer.device_used, p_command_buffer.command_buffer, p_host.buffer, p_host.size, p_gpu.image, p_gpu.format);
        BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, iImage<const ImageGPU>{p_gpu}, ImageUsageFlag::TRANSFER_WRITE, p_gpu.format.imageUsage);
    };

    inline static void cmd_copy_image_gpu_to_buffer_host(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const ImageGPU& p_gpu, const BufferHost& p_host)
    {
#if __DEBUG
        assert_true(p_gpu.size <= p_host.size);
#endif

        BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, iImage<const ImageGPU>{p_gpu}, p_gpu.format.imageUsage, ImageUsageFlag::TRANSFER_READ);
        gpu::command_copy_image_to_buffer(p_command_buffer.device_used, p_command_buffer.command_buffer, p_gpu.image, p_gpu.format, p_host.buffer);
        BufferCommandUtils::cmd_image_layout_transition_v2(p_command_buffer, p_barriers, iImage<const ImageGPU>{p_gpu}, ImageUsageFlag::TRANSFER_READ, p_gpu.format.imageUsage);
    };

    template <class _Image>
    inline static void cmd_image_layout_transition_v2(const CommandBuffer& p_command_buffer, const ImageLayoutTransitionBarriers& p_barriers, const iImage<_Image> p_image,
                                                      const ImageUsageFlag p_source_image_usage, const ImageUsageFlag p_target_image_usage)
    {
        ImageLayoutTransitionBarrierConfiguration l_layout_barrier = p_barriers.get_barrier(p_source_image_usage, p_target_image_usage);
        gpu::command_image_layout_transition(p_command_buffer.device_used, p_command_buffer.command_buffer, p_image.get_image(), p_image.get_image_format(), l_layout_barrier.src_stage,
                                             ImageLayoutTransitionBarriers::get_imagelayout_from_imageusage(p_source_image_usage), l_layout_barrier.src_access_mask, l_layout_barrier.dst_stage,
                                             ImageLayoutTransitionBarriers::get_imagelayout_from_imageusage(p_target_image_usage), l_layout_barrier.dst_access_mask);
    };

  private:
    inline static void cmd_copy_buffer(const CommandBuffer& p_command_buffer, const gpu::Buffer p_source_buffer, const uimax p_source_size, const gpu::Buffer p_target_buffer,
                                       const uimax p_target_size)
    {
#if __DEBUG
        assert_true(p_source_size <= p_target_size);
#endif
        gpu::command_copy_buffer(p_command_buffer.device_used, p_command_buffer.command_buffer, p_source_buffer, p_source_size, p_target_buffer, p_target_size);
    };
};

struct BufferStep
{
    // /!\ We make the assumption that the command buffer in the BufferAllocator has been already opened
    inline static void step(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events)
    {
#if __DEBUG
        assert_true(p_buffer_allocator.device.command_buffer.debug_state == CommandBuffer::DebugState::BEGIN);
#endif

        clean_garbage_buffers(p_buffer_allocator, p_buffer_events);

        /*
            /!\ It is very important that image host and gpu allocation events are processed before image copy operations because further events assume that the image layout of the image is the
           targetted one.
        */
        /*
         ImageLayout transition from unknown state to desired imageUsage.
        */
        if (p_buffer_events.image_host_allocate_events.Size > 0)
        {
            for (loop(i, 0, p_buffer_events.image_host_allocate_events.Size))
            {
                Token<ImageHost> l_image_token = p_buffer_events.image_host_allocate_events.get(i).image;
                ImageHost& l_image = p_buffer_allocator.host_images.get(l_image_token);
                BufferCommandUtils::cmd_image_layout_transition_v2(p_buffer_allocator.device.command_buffer, p_buffer_allocator.image_layout_barriers, iImage<ImageHost>{l_image},
                                                                   ImageUsageFlag::UNKNOWN, l_image.format.imageUsage);
            }
            p_buffer_events.image_host_allocate_events.clear();
        }

        /*
          ImageLayout transition from unknown state to desired imageUsage.
         */
        if (p_buffer_events.image_gpu_allocate_events.Size > 0)
        {
            for (loop(i, 0, p_buffer_events.image_gpu_allocate_events.Size))
            {
                Token<ImageGPU> l_image_token = p_buffer_events.image_gpu_allocate_events.get(i).image;
                ImageGPU& l_image = p_buffer_allocator.gpu_images.get(l_image_token);
                BufferCommandUtils::cmd_image_layout_transition_v2(p_buffer_allocator.device.command_buffer, p_buffer_allocator.image_layout_barriers, iImage<ImageGPU>{l_image},
                                                                   ImageUsageFlag::UNKNOWN, l_image.format.imageUsage);
            }
            p_buffer_events.image_gpu_allocate_events.clear();
        }

        /*
            ImageLayout transition from desired imageUsage to transfert_dst.
            Copy command.
            ImageLayout transition from transfert_dst to desired imageUsage.
        */
        if (p_buffer_events.write_buffer_host_to_image_gpu_events.Size > 0)
        {
            for (loop(i, 0, p_buffer_events.write_buffer_host_to_image_gpu_events.Size))
            {
                auto& l_event = p_buffer_events.write_buffer_host_to_image_gpu_events.get(i);
                BufferCommandUtils::cmd_copy_buffer_host_to_image_gpu(p_buffer_allocator.device.command_buffer, p_buffer_allocator.image_layout_barriers,
                                                                      p_buffer_allocator.host_buffers.get(l_event.source_buffer), p_buffer_allocator.gpu_images.get(l_event.target_image));

                if (l_event.source_buffer_dispose)
                {
                    p_buffer_events.garbage_host_buffers.push_back_element(l_event.source_buffer);
                }
            }
            p_buffer_events.write_buffer_host_to_image_gpu_events.clear();
        }

        /*
            ImageLayout transition from desired imageUsage to transfert_src.
            Copy command.
            ImageLayout transition from transfert_src to desired imageUsage.
        */
        if (p_buffer_events.write_image_gpu_to_buffer_host_events.Size > 0)
        {
            for (loop(i, 0, p_buffer_events.write_image_gpu_to_buffer_host_events.Size))
            {
                auto& l_event = p_buffer_events.write_image_gpu_to_buffer_host_events.get(i);
                BufferCommandUtils::cmd_copy_image_gpu_to_buffer_host(p_buffer_allocator.device.command_buffer, p_buffer_allocator.image_layout_barriers,
                                                                      p_buffer_allocator.gpu_images.get(l_event.source_image), p_buffer_allocator.host_buffers.get(l_event.target_buffer));
            }
            p_buffer_events.write_image_gpu_to_buffer_host_events.clear();
        }

        if (p_buffer_events.write_buffer_host_to_buffer_gpu_events.Size > 0)
        {
            for (loop(i, 0, p_buffer_events.write_buffer_host_to_buffer_gpu_events.Size))
            {
                auto& l_event = p_buffer_events.write_buffer_host_to_buffer_gpu_events.get(i);
                BufferCommandUtils::cmd_copy_buffer_host_to_gpu(p_buffer_allocator.device.command_buffer, p_buffer_allocator.host_buffers.get(l_event.source_buffer),
                                                                p_buffer_allocator.gpu_buffers.get(l_event.target_buffer));

                if (l_event.source_buffer_dispose)
                {
                    p_buffer_events.garbage_host_buffers.push_back_element(l_event.source_buffer);
                }
            }

            p_buffer_events.write_buffer_host_to_buffer_gpu_events.clear();
        }

        if (p_buffer_events.write_buffer_gpu_to_buffer_host_events.Size > 0)
        {

            for (loop(i, 0, p_buffer_events.write_buffer_gpu_to_buffer_host_events.Size))
            {
                auto& l_event = p_buffer_events.write_buffer_gpu_to_buffer_host_events.get(i);
                BufferCommandUtils::cmd_copy_buffer_gpu_to_host(p_buffer_allocator.device.command_buffer, p_buffer_allocator.gpu_buffers.get(l_event.source_buffer),
                                                                p_buffer_allocator.host_buffers.get(l_event.target_buffer));
            }

            p_buffer_events.write_buffer_gpu_to_buffer_host_events.clear();
        }
    };

    inline static void clean_garbage_buffers(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events)
    {
        for (loop(i, 0, p_buffer_events.garbage_host_buffers.Size))
        {
            p_buffer_allocator.free_bufferhost(p_buffer_events.garbage_host_buffers.get(i));
        }

        p_buffer_events.garbage_host_buffers.clear();
    };
};

struct BufferMemory
{
    BufferAllocator allocator;
    BufferEvents events;

    inline static BufferMemory allocate(const GPUInstance& p_instance)
    {
        return BufferMemory{BufferAllocator::allocate_default(p_instance), BufferEvents::allocate()};
    };

    inline void free()
    {
        BufferStep::clean_garbage_buffers(this->allocator, this->events);

        this->allocator.free();
        this->events.free();
    };
};

struct BufferAllocatorComposition
{
    inline static void free_buffer_host_and_remove_event_references(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<BufferHost> p_buffer_host)
    {
        p_buffer_events.remove_buffer_host_references(p_buffer_host);
        p_buffer_allocator.free_bufferhost(p_buffer_host);
    };

    inline static void free_buffer_gpu_and_remove_event_references(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<BufferGPU> p_buffer_gpu)
    {
        p_buffer_events.remove_buffer_gpu_references(p_buffer_gpu);
        p_buffer_allocator.free_buffergpu(p_buffer_gpu);
    };

    inline static Token<ImageHost> allocate_imagehost_and_push_creation_event(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Slice<int8>& p_value,
                                                                              const ImageFormat& p_image_format)
    {
        Token<ImageHost> l_image_host = p_buffer_allocator.allocate_imagehost(p_value, p_image_format);
        p_buffer_events.image_host_allocate_events.push_back_element(BufferEvents::AllocatedImageHost{l_image_host});
        return l_image_host;
    };

    inline static Token<ImageHost> allocate_imagehost_empty_and_push_creation_event(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const ImageFormat& p_image_format)
    {
        Token<ImageHost> l_image_host = p_buffer_allocator.allocate_imagehost_empty(p_image_format);
        p_buffer_events.image_host_allocate_events.push_back_element(BufferEvents::AllocatedImageHost{l_image_host});
        return l_image_host;
    };

    inline static void free_image_host_and_remove_event_references(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<ImageHost> p_image_host)
    {
        p_buffer_events.remove_image_host_references(p_image_host);
        p_buffer_allocator.free_imagehost(p_image_host);
    };

    inline static Token<ImageGPU> allocate_imagegpu_and_push_creation_event(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const ImageFormat& p_image_format)
    {
        Token<ImageGPU> l_image_gpu = p_buffer_allocator.allocate_imagegpu(p_image_format);
        p_buffer_events.image_gpu_allocate_events.push_back_element(BufferEvents::AllocatedImageGPU{l_image_gpu});
        return l_image_gpu;
    };

    inline static void free_image_gpu_and_remove_event_references(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<ImageGPU> p_image_gpu)
    {
        p_buffer_events.remove_image_gpu_references(p_image_gpu);
        p_buffer_allocator.free_imagegpu(p_image_gpu);
    };
};

struct BufferReadWrite
{
    inline static void write_to_buffergpu(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<BufferGPU> p_buffer_gpu, const Slice<int8>& p_value)
    {
        Token<BufferHost> l_staging_buffer = p_buffer_allocator.allocate_bufferhost(p_value, BufferUsageFlag::TRANSFER_READ);
        p_buffer_events.write_buffer_host_to_buffer_gpu_events.push_back_element(BufferEvents::WriteBufferHostToBufferGPU{l_staging_buffer, 1, p_buffer_gpu});
    };

        // TODO -> we want this to be the default behavior
#if 0
    inline static void write_to_buffergpu_no_allocation(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<BufferGPU> p_buffer_gpu,
                                                        const Token<BufferHost> p_buffer_source)
    {
        p_buffer_events.write_buffer_host_to_buffer_gpu_events.push_back_element(BufferEvents::WriteBufferHostToBufferGPU{p_buffer_source, 0, p_buffer_gpu});
    };
#endif

    inline static Token<BufferHost> read_from_buffergpu(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<BufferGPU> p_buffer_gpu_token, const BufferGPU& p_buffer_gpu)
    {
        Token<BufferHost> l_staging_buffer = p_buffer_allocator.allocate_bufferhost_empty(p_buffer_gpu.size, BufferUsageFlag::TRANSFER_WRITE);
        p_buffer_events.write_buffer_gpu_to_buffer_host_events.push_back_element(BufferEvents::WriteBufferGPUToBufferHost{p_buffer_gpu_token, l_staging_buffer});
        return l_staging_buffer;
    };

    inline static void write_to_imagegpu(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<ImageGPU> p_image_gpu_token, const ImageGPU& p_image_gpu,
                                         const Slice<int8>& p_value)
    {
        Token<BufferHost> l_stagin_buffer = p_buffer_allocator.allocate_bufferhost(p_value, BufferUsageFlag::TRANSFER_READ);
        p_buffer_events.write_buffer_host_to_image_gpu_events.push_back_element(BufferEvents::WriteBufferHostToImageGPU{l_stagin_buffer, 1, p_image_gpu_token});
    };

    inline static Token<BufferHost> read_from_imagegpu_to_buffer(BufferAllocator& p_buffer_allocator, BufferEvents& p_buffer_events, const Token<ImageGPU> p_image_gpu_token,
                                                                 const ImageGPU& p_image_gpu)
    {
        Token<BufferHost> l_stagin_buffer = p_buffer_allocator.allocate_bufferhost_empty(p_image_gpu.size, BufferUsageFlag::TRANSFER_WRITE);
        p_buffer_events.write_image_gpu_to_buffer_host_events.push_back_element(BufferEvents::WriteImageGPUToBufferHost{p_image_gpu_token, l_stagin_buffer});
        return l_stagin_buffer;
    };
};
