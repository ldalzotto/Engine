#pragma once

#ifndef GPU_BACKEND_VULKAN
#define GPU_BACKEND_VULKAN 1
#endif

#ifndef GPU_BACKEND_SOFTWARE
#define GPU_BACKEND_SOFTWARE 0
#endif

#include "Math2/math.hpp"

#include "./backends/interface.hpp"
#include "./_external/gpu_platform_interface.hpp"
#if GPU_BACKEND_VULKAN
#include "./backends/vulkan_backend.hpp"
#elif GPU_BACKEND_SOFTWARE
#include "./backends/software_backend.hpp"
#endif

#include "./command_buffer.hpp"
#include "./instance.hpp"
#include "./memory.hpp"

// TODO -> the TMP_GPU_GRAPHICS_ENABLED preprocessing variable enable or disable all gpu graphics related stuff
// This variable must be removed when graphics will have been abstracted on the gpu_interface
#if GPU_BACKEND_SOFTWARE
#define TMP_GPU_GRAPHICS_ENABLED 0
#else
#define TMP_GPU_GRAPHICS_ENABLED 1
#endif
#if TMP_GPU_GRAPHICS_ENABLED

#include "./graphics.hpp"
#include "./material.hpp"
#include "./graphics_binder.hpp"
#include "./present.hpp"

#endif

struct GPUContext
{
    GPUInstance instance;
    BufferMemory buffer_memory;
#if TMP_GPU_GRAPHICS_ENABLED
    GraphicsAllocator2 graphics_allocator;
#endif
    Semafore buffer_end_semaphore;
    gpu::CommandBufferSubmit buffer_command_submit;
    Semafore graphics_end_semaphore;

    inline static GPUContext allocate(const Slice<GPUExtension>& p_gpu_extensions)
    {
        GPUContext l_context;
        l_context.instance = GPUInstance::allocate(p_gpu_extensions);
        l_context.buffer_memory = BufferMemory::allocate(l_context.instance);
#if TMP_GPU_GRAPHICS_ENABLED
        l_context.graphics_allocator = GraphicsAllocator2::allocate_default(l_context.instance);
#endif
        l_context.buffer_end_semaphore = Semafore::allocate(l_context.instance.logical_device);
        l_context.graphics_end_semaphore = Semafore::allocate(l_context.instance.logical_device);
        return l_context;
    };

    inline void free()
    {
        this->buffer_step_force_execution();

        this->buffer_end_semaphore.free(this->instance.logical_device);
        this->graphics_end_semaphore.free(this->instance.logical_device);
#if TMP_GPU_GRAPHICS_ENABLED
        this->graphics_allocator.free();
#endif
        this->buffer_memory.free();
        this->instance.free();
    };

    inline void buffer_step_submit()
    {
        this->buffer_memory.allocator.device.command_buffer.begin();
        BufferStep::step(this->buffer_memory.allocator, this->buffer_memory.events);
        this->buffer_memory.allocator.device.command_buffer.end();
        this->buffer_command_submit = this->buffer_memory.allocator.device.command_buffer.submit_and_notify(this->buffer_end_semaphore);
    };

    inline void buffer_step_submit_no_graphics_notification()
    {
        this->buffer_memory.allocator.device.command_buffer.begin();
        BufferStep::step(this->buffer_memory.allocator, this->buffer_memory.events);
        this->buffer_memory.allocator.device.command_buffer.end();
        this->buffer_command_submit = this->buffer_memory.allocator.device.command_buffer.submit();
    };

    inline void buffer_step_force_execution()
    {
        this->buffer_step_submit_no_graphics_notification();
        this->buffer_memory.allocator.device.command_buffer.wait_for_completion();
    };

#if TMP_GPU_GRAPHICS_ENABLED
    inline GraphicsBinder build_graphics_binder()
    {
        GraphicsBinder l_binder = GraphicsBinder::build(this->buffer_memory.allocator, this->graphics_allocator);
        l_binder.start();
        return l_binder;
    };

    inline void submit_graphics_binder(GraphicsBinder& p_binder)
    {
        p_binder.end();
        p_binder.submit_after(this->buffer_end_semaphore, this->buffer_command_submit);
    };

    inline void submit_graphics_binder_and_notity_end(GraphicsBinder& p_binder)
    {
        p_binder.end();
        p_binder.submit_after_and_notify(this->buffer_end_semaphore, this->buffer_command_submit, this->graphics_end_semaphore);
    };

    inline void wait_for_completion()
    {
        this->graphics_allocator.graphics_device.command_buffer.wait_for_completion();
        this->buffer_command_submit = token_build_default<gpu::_CommandBufferSubmit>();
    };
#endif
};

// The buffer step can be broken down into multiple parts if for some reason, we want to execute an additional operation on the transfer command buffer.
// If the buffer step only use BufferStep::step then it's cleaner to use the GPUContext functions
// /!\ It is important to note that any additional operations that involve buffer or image allocation must take place before the call to BufferStep because it is him that handle all read/write logic.
struct BufferStepExecutionFlow
{
    inline static void buffer_step_begin(BufferMemory& p_buffer_memory)
    {
        p_buffer_memory.allocator.device.command_buffer.begin();
    };

    inline static void buffer_step_submit(GPUContext& p_gpu_context)
    {
        p_gpu_context.buffer_memory.allocator.device.command_buffer.end();
        p_gpu_context.buffer_memory.allocator.device.command_buffer.submit_and_notify(p_gpu_context.buffer_end_semaphore);
    };

    inline static void buffer_step_submit_no_graphics_notification(GPUContext& p_gpu_context)
    {
        p_gpu_context.buffer_memory.allocator.device.command_buffer.end();
        p_gpu_context.buffer_memory.allocator.device.command_buffer.submit();
    };

    inline static void buffer_step_force_execution(GPUContext& p_gpu_context)
    {
        p_gpu_context.buffer_memory.allocator.device.command_buffer.end();
        p_gpu_context.buffer_memory.allocator.device.command_buffer.submit();
        p_gpu_context.buffer_memory.allocator.device.command_buffer.wait_for_completion();
    };
};
