#pragma once

#include "GPU_Software/gpu_software.hpp"

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

gpu::Queue gpu::logical_device_get_queue(const LogicalDevice p_logical_device, const QueueFamily p_queue_family){
    // TODO ?
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

void gpu::command_buffer_begin(LogicalDevice p_device, CommandBuffer p_command_buffer){
    // nothing
};

void gpu::command_buffer_end(LogicalDevice p_device, CommandBuffer p_command_buffer){
    // nothing
};

gpu::CommandBufferSubmit gpu::command_buffer_submit(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    l_instance->command_buffer_submit(token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer));
};

gpu::CommandBufferSubmit gpu::command_buffer_submit_and_notify(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_notified_semaphore)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    l_instance->command_buffer_submit(token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer));
};

gpu::CommandBufferSubmit gpu::command_buffer_submit_after(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore,
                                                          CommandBufferSubmit p_after_command_buffer)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);

    l_instance->command_buffer_submit_wait_for(
        token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer),
        pattern::cb::Semaphore<gpu_software::GPUCommand>{token_build_from<NNTree<Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>>::Node>(p_after_command_buffer)});
};

gpu::CommandBufferSubmit gpu::command_buffer_submit_after_and_notify(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore,
                                                                     CommandBufferSubmit p_after_command_buffer, Semaphore p_notify_semaphore)
{
    gpu_software::Instance* l_instance = (gpu_software::Instance*)token_value(p_device);
    l_instance->command_buffer_submit_wait_for(
        token_build_from<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>(p_command_buffer),
        pattern::cb::Semaphore<gpu_software::GPUCommand>{token_build_from<NNTree<Token<pattern::cb::CommandBuffer<gpu_software::GPUCommand>>>::Node>(p_after_command_buffer)});
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