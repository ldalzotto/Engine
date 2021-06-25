#pragma once

#include "GPU_Software/gpu_software.hpp"

void gpu::layer_push_debug_layers(VectorSlice<LayerConstString>& p_layers){
    // nothing
};

gpu::Instance gpu::create_instance(const ApplicationInfo& p_application_info)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)heap_malloc(sizeof(gpu_software::Instance));
    *l_instance = gpu_software::Instance::allocate();
    return token_build<gpu::_Instance>((token_t)l_instance);
};

void gpu::instance_destroy(Instance p_instance)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_instance);
    l_instance->free();
    heap_free((int8*)l_instance);
};

gpu::Debugger gpu::initialize_debug_callback(Instance p_instance)
{
    return token_build_default<gpu::_Debugger>();
};

void gpu::debugger_finalize(Debugger p_debugger, Instance p_instance){
    // nothing
};

gpu::physical_device_pick_Return gpu::physical_device_pick(Instance p_instance)
{
    gpu::physical_device_pick_Return l_return = gpu::physical_device_pick_Return::build_default();
    l_return.physical_device = token_build<gpu::_PhysicalDevice>(token_value(p_instance));

    MemoryHeap l_memory_heap;
    l_memory_heap.size = 0;
    l_memory_heap.type = MemoryTypeFlag::HOST_VISIBLE;
    l_return.physical_memory_properties.memory_heaps = SliceN<MemoryHeap, PhysicalDeviceMemoryProperties::stack_size>{l_memory_heap};
    l_return.physical_memory_properties.memory_heaps_size = 1;

    MemoryType l_device_host_memory_type;
    l_device_host_memory_type.type = MemoryTypeFlag::HOST_VISIBLE;
    l_device_host_memory_type.heap_index = {0};
    l_return.physical_memory_properties.memory_types = SliceN<MemoryType, PhysicalDeviceMemoryProperties::stack_size>{l_device_host_memory_type};
    l_return.physical_memory_properties.memory_types_size = 1;

    return l_return;
};

gpu::PhysicalDeviceMemoryIndex gpu::physical_device_get_memorytype_index(const PhysicalDeviceMemoryProperties& p_memory_properties, const MemoryTypeFlag p_required_memory_type,
                                                                         const MemoryTypeFlag p_memory_type)
{
    return gpu::PhysicalDeviceMemoryIndex{0};
};

gpu::LogicalDevice gpu::logical_device_create(PhysicalDevice p_physical_device, const Slice<LayerConstString>& p_validation_layers, const Slice<GPUExtension>& p_gpu_extensions,
                                              const QueueFamily& p_queue)
{
    return token_build<gpu::_LogicalDevice>(token_value(p_physical_device));
};

void gpu::logical_device_destroy(LogicalDevice p_logical_device){

};

gpu::Queue gpu::logical_device_get_queue(const LogicalDevice p_logical_device, const QueueFamily p_queue_family)
{
    return token_build<gpu::_Queue>(token_value(p_logical_device));
};

gpu::DeviceMemory gpu::allocate_memory(const LogicalDevice p_device, const uimax p_allocation_size, const PhysicalDeviceMemoryIndex p_memory_index)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<Span<int8>> l_memory = l_instance->memory_allocate(p_allocation_size);
    return token_build<gpu::_DeviceMemory>(token_value(l_memory));
};

void gpu::free_memory(const LogicalDevice p_device, const DeviceMemory p_device_memory)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<Span<int8>> l_memory = token_build_from<Span<int8>>(p_device_memory);
    l_instance->memory_free(l_memory);
};

gpu::Semaphore gpu::semaphore_allocate(const LogicalDevice p_device)
{
    return token_build_default<gpu::_Semaphore>();
};

void gpu::semaphore_destroy(const LogicalDevice p_device, const Semaphore p_semaphore){
    // nothing
};

void gpu::queue_wait_idle(Queue p_queue)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_queue);
    l_instance->wait_for_end();
};

void gpu::command_buffer_begin(LogicalDevice p_device, CommandBuffer p_command_buffer){
    // nothing
};

void gpu::command_buffer_end(LogicalDevice p_device, CommandBuffer p_command_buffer){
    // nothing
};

gpu::CommandBufferSubmit gpu::command_buffer_submit(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    return token_build_from<gpu::_CommandBufferSubmit>(l_instance->command_buffer_submit(token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer)));
};

gpu::CommandBufferSubmit gpu::command_buffer_submit_and_notify(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_notified_semaphore)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    return token_build_from<gpu::_CommandBufferSubmit>(l_instance->command_buffer_submit(token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer)));
};

gpu::CommandBufferSubmit gpu::command_buffer_submit_after(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore,
                                                          CommandBufferSubmit p_after_command_buffer)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);

    return token_build_from<gpu::_CommandBufferSubmit>(l_instance->command_buffer_submit_wait_for(
        token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer),
        pattern::cb::Semaphore<gpu_software::GPUCommand>{token_build_from<NNTree<Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>>::Node>(p_after_command_buffer)}));
};

gpu::CommandBufferSubmit gpu::command_buffer_submit_after_and_notify(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore,
                                                                     CommandBufferSubmit p_after_command_buffer, Semaphore p_notify_semaphore)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    return token_build_from<gpu::_CommandBufferSubmit>(l_instance->command_buffer_submit_wait_for(
        token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer),
        pattern::cb::Semaphore<gpu_software::GPUCommand>{token_build_from<NNTree<Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>>::Node>(p_after_command_buffer)}));
};

void gpu::command_copy_buffer(LogicalDevice p_device, CommandBuffer p_command_buffer, const Buffer p_source, const uimax p_source_size, Buffer p_target, const uimax p_target_size)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>> l_command_buffer_token = token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer);
    pattern::cb::CommandBuffer<gpu_software::GPUCommand>& l_command_buffer = l_instance->command_pool.command_buffers.get(l_command_buffer_token);

    gpu_software::Buffer& l_source_buffer = l_instance->buffers.get(token_build_from<gpu_software::Buffer>(p_source));
    gpu_software::Buffer& l_target_buffer = l_instance->buffers.get(token_build_from<gpu_software::Buffer>(p_target));

    gpu_software::GPUCommand_CopyMemory l_copy_memory;
    l_copy_memory.source = l_source_buffer.binded_memory;
    l_copy_memory.source.Size = p_source_size;
    l_copy_memory.target = l_target_buffer.binded_memory;
    l_copy_memory.target.Size = p_target_size;
    l_command_buffer.commands.push_back_element(gpu_software::GPUCommand::build_copy_memory(l_copy_memory));
};

void gpu::command_copy_buffer_to_image(LogicalDevice p_device, CommandBuffer p_command_buffer, const Buffer p_source, const uimax p_source_size, const Image p_target,
                                       const ImageFormat& p_target_format)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>> l_command_buffer_token = token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer);
    pattern::cb::CommandBuffer<gpu_software::GPUCommand>& l_command_buffer = l_instance->command_pool.command_buffers.get(l_command_buffer_token);

    gpu_software::Buffer& l_source_buffer = l_instance->buffers.get(token_build_from<gpu_software::Buffer>(p_source));
    gpu_software::Image& l_target_image = l_instance->images.get(token_build_from<gpu_software::Image>(p_target));

    gpu_software::GPUCommand_CopyMemory l_copy_memory;
    l_copy_memory.source = l_source_buffer.binded_memory;
    l_copy_memory.source.Size = p_source_size;
    l_copy_memory.target = l_target_image.binded_memory;
    l_copy_memory.target.Size = p_target_format.extent.x * p_target_format.extent.y * p_target_format.extent.z;
    l_command_buffer.commands.push_back_element(gpu_software::GPUCommand::build_copy_memory(l_copy_memory));
};

void gpu::command_copy_image_to_buffer(LogicalDevice p_device, CommandBuffer p_command_buffer, const Image p_source, const ImageFormat& p_source_format, const Buffer p_target)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>> l_command_buffer_token = token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer);
    pattern::cb::CommandBuffer<gpu_software::GPUCommand>& l_command_buffer = l_instance->command_pool.command_buffers.get(l_command_buffer_token);

    gpu_software::Image& l_source_image = l_instance->images.get(token_build_from<gpu_software::Image>(p_source));
    gpu_software::Buffer& l_target_buffer = l_instance->buffers.get(token_build_from<gpu_software::Buffer>(p_target));

    gpu_software::GPUCommand_CopyMemory l_copy_memory;
    l_copy_memory.source = l_source_image.binded_memory;
    l_copy_memory.source.Size = p_source_format.extent.x * p_source_format.extent.y * p_source_format.extent.z;
    l_copy_memory.target = l_target_buffer.binded_memory;
    l_copy_memory.target.Size = l_copy_memory.source.Size;
    l_command_buffer.commands.push_back_element(gpu_software::GPUCommand::build_copy_memory(l_copy_memory));
};

void gpu::command_copy_image(LogicalDevice p_device, CommandBuffer p_command_buffer, const Image p_source, const ImageFormat& p_source_format, const Image p_target, const ImageFormat& p_target_format)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>> l_command_buffer_token = token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer);
    pattern::cb::CommandBuffer<gpu_software::GPUCommand>& l_command_buffer = l_instance->command_pool.command_buffers.get(l_command_buffer_token);

    gpu_software::Image& l_source_image = l_instance->images.get(token_build_from<gpu_software::Image>(p_source));
    gpu_software::Image& l_target_image = l_instance->images.get(token_build_from<gpu_software::Image>(p_target));

    gpu_software::GPUCommand_CopyMemory l_copy_memory;
    l_copy_memory.source = l_source_image.binded_memory;
    l_copy_memory.source.Size = p_source_format.extent.x * p_source_format.extent.y * p_source_format.extent.z;
    l_copy_memory.target = l_target_image.binded_memory;
    l_copy_memory.target.Size = p_target_format.extent.x * p_target_format.extent.y * p_target_format.extent.z;
    l_command_buffer.commands.push_back_element(gpu_software::GPUCommand::build_copy_memory(l_copy_memory));
};

void gpu::command_image_layout_transition(LogicalDevice p_device, CommandBuffer p_command_buffer, const Image p_image, const ImageFormat& p_image_format, const GPUPipelineStageFlag p_source_stage,
                                          const ImageLayoutFlag p_source_layout, const GPUAccessFlag p_source_access, const GPUPipelineStageFlag p_target_stage, const ImageLayoutFlag p_target_layout,
                                          const GPUAccessFlag p_target_access){
    // nothign
};

int8* gpu::map_memory(const LogicalDevice p_device, const DeviceMemory p_device_memory, const uimax p_offset, const uimax p_size)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    Token<Span<int8>> l_memory = token_build_from<Span<int8>>(p_device_memory);
    Slice<int8> l_mapped_slice = l_instance->memory_chunks.get(l_memory).slice.slide_rv(p_offset);
#if __DEBUG
    assert_true(l_mapped_slice.get_size() >= p_size);
#endif
    return l_mapped_slice.Begin;
};

gpu::CommandPool gpu::command_pool_allocate(const LogicalDevice p_logical_device, const QueueFamily p_queue_family)
{
    return token_build_from<gpu::_CommandPool>(p_logical_device);
};

void gpu::command_pool_destroy(const LogicalDevice p_logical_device, CommandPool p_pool){
    // nothing
};

gpu::CommandBuffer gpu::command_pool_allocate_command_buffer(const LogicalDevice p_logical_device, CommandPool p_command_pool)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>> l_command_buffer = l_instance->command_pool.allocate_command_buffer();
    return token_build_from<gpu::_CommandBuffer>(l_command_buffer);
};

void gpu::command_pool_destroy_command_buffer(const LogicalDevice p_logical_device, CommandPool p_command_pool, CommandBuffer p_command_buffer)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>> l_command_buffer = token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer);
    l_instance->command_pool.free_command_buffer(l_command_buffer);
};

gpu::Buffer gpu::buffer_allocate(const LogicalDevice p_logical_device, const uimax p_size, const BufferUsageFlag p_usage_flag)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    gpu_software::Buffer l_buffer = gpu_software::Buffer::build_default();
    l_buffer.size = p_size;
    return token_build_from<gpu::_Buffer>(l_instance->buffers.alloc_element(l_buffer));
};

void gpu::buffer_destroy(const LogicalDevice p_logical_device, const Buffer p_buffer)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Buffer> l_buffer = token_build_from<gpu_software::Buffer>(p_buffer);
    l_instance->buffers.release_element(l_buffer);
};

gpu::MemoryRequirements gpu::buffer_get_memory_requirements(const LogicalDevice p_logical_device, const Buffer p_buffer)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Buffer> l_buffer = token_build_from<gpu_software::Buffer>(p_buffer);
    MemoryRequirements l_requirements;
    l_requirements.size = l_instance->buffers.get(l_buffer).size;
    l_requirements.alignment = 1;
    l_requirements.memory_type = MemoryTypeFlag::HOST_VISIBLE;
    return l_requirements;
};

void gpu::buffer_bind_memory(const LogicalDevice p_logical_device, const Buffer p_buffer, const DeviceMemory p_memory, const uimax p_offset)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Buffer> l_buffer_token = token_build_from<gpu_software::Buffer>(p_buffer);
    gpu_software::Buffer& l_buffer = l_instance->buffers.get(l_buffer_token);
    Token<Span<int8>> l_memory = token_build_from<Span<int8>>(p_memory);
    l_buffer.binded_memory = l_instance->memory_chunks.get(l_memory).slice.slide_rv(p_offset);
    l_buffer.binded_memory.Size = l_buffer.size;
};

gpu::Image gpu::image_allocate(const LogicalDevice p_logical_device, const ImageFormat& p_image_format, const ImageTilingFlag p_image_tiling)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Image> l_image =
        l_instance->images.alloc_element(gpu_software::Image::build_format(gpu_software::Image::Format{p_image_format.extent.x, p_image_format.extent.y, p_image_format.extent.z}));
    return token_build_from<gpu::_Image>(l_image);
};

void gpu::image_destroy(const LogicalDevice p_logical_device, const Image p_image)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Image> l_image = token_build_from<gpu_software::Image>(p_image);
    l_instance->images.release_element(l_image);
};

gpu::MemoryRequirements gpu::image_get_memory_requirements(const LogicalDevice p_logical_device, const Image p_image)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Image> l_image_token = token_build_from<gpu_software::Image>(p_image);
    gpu_software::Image& l_image = l_instance->images.get(l_image_token);
    gpu::MemoryRequirements l_requirements;
    l_requirements.size = l_image.format.x * l_image.format.y * l_image.format.z;
    l_requirements.alignment = 1;
    l_requirements.memory_type = MemoryTypeFlag::HOST_VISIBLE;
    return l_requirements;
};

void gpu::image_bind_memory(const LogicalDevice p_logical_device, const Image p_image, const DeviceMemory p_memory, const uimax p_offset)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_logical_device);
    Token<gpu_software::Image> l_image_token = token_build_from<gpu_software::Image>(p_image);
    Token<Span<int8>> l_memory = token_build_from<Span<int8>>(p_memory);
    gpu_software::Image& l_image = l_instance->images.get(l_image_token);
    l_image.binded_memory = l_instance->memory_chunks.get(l_memory).slice.slide_rv(p_offset);
    l_image.binded_memory.Size = l_image.format.x * l_image.format.y * l_image.format.z;
};