#pragma once

struct Semafore
{
    gpu::Semaphore semaphore;

    inline static Semafore allocate(const gpu::LogicalDevice p_device)
    {
        Semafore l_semaphore;
        l_semaphore.semaphore = gpu::semaphore_allocate(p_device);
        return l_semaphore;
    };

    inline void free(const gpu::LogicalDevice p_device)
    {
        gpu::semaphore_destroy(p_device, this->semaphore);
    };
};

struct CommandBuffer
{
    gpu::CommandBuffer command_buffer;
    gpu::Queue queue;
    gpu::LogicalDevice device_used;

#if __DEBUG
    enum class DebugState
    {
        WAITING = 0,
        BEGIN = 1,
        END = 2
    } debug_state;
#endif

    inline static CommandBuffer build_default()
    {
        CommandBuffer l_return;
        l_return.command_buffer = token_build_default<gpu::_CommandBuffer>();
        l_return.queue = token_build_default<gpu::_Queue>();
        l_return.device_used = token_build_default<gpu::_LogicalDevice>();
#if __DEBUG
        l_return.debug_state = DebugState::WAITING;
#endif
        return l_return;
    };

    inline void begin()
    {
#if __DEBUG
        assert_true(this->debug_state == DebugState::WAITING);
        this->debug_state = DebugState::BEGIN;
#endif
        gpu::command_buffer_begin(this->device_used, this->command_buffer);
    };

    inline void end()
    {
#if __DEBUG
        assert_true(this->debug_state == DebugState::BEGIN);
        this->debug_state = DebugState::END;
#endif

        gpu::command_buffer_end(this->device_used, this->command_buffer);
    };

    inline gpu::CommandBufferSubmit submit()
    {
#if __DEBUG
        assert_true(this->debug_state == DebugState::END);
        this->debug_state = DebugState::WAITING;
#endif
        return gpu::command_buffer_submit(this->device_used, this->command_buffer, this->queue);
    };

    inline gpu::CommandBufferSubmit submit_and_notify(const Semafore p_notify_semaphore)
    {
#if __DEBUG
        assert_true(this->debug_state == DebugState::END);
        this->debug_state = DebugState::WAITING;
#endif
        return gpu::command_buffer_submit_and_notify(this->device_used, this->command_buffer, this->queue, p_notify_semaphore.semaphore);
    };

    inline gpu::CommandBufferSubmit submit_after(const Semafore p_semaphore_wait_for, const gpu::CommandBufferSubmit p_command_buffer_wait_for)
    {
#if __DEBUG
        assert_true(this->debug_state == DebugState::END);
        this->debug_state = DebugState::WAITING;
#endif
        return gpu::command_buffer_submit_after(this->device_used, this->command_buffer, this->queue, p_semaphore_wait_for.semaphore, p_command_buffer_wait_for);
    };

    inline gpu::CommandBufferSubmit submit_after_and_notify(const Semafore p_semaphore_wait_for, const gpu::CommandBufferSubmit p_command_buffer_wait_for, const Semafore p_notify)
    {
#if __DEBUG
        assert_true(this->debug_state == DebugState::END);
        this->debug_state = DebugState::WAITING;
#endif
        return gpu::command_buffer_submit_after_and_notify(this->device_used, this->command_buffer, this->queue, p_semaphore_wait_for.semaphore, p_command_buffer_wait_for, p_notify.semaphore);
    };

    inline void wait_for_completion()
    {
        gpu::queue_wait_idle(this->queue);
    };
};

struct CommandPool
{
    gpu::CommandPool pool;

    inline static CommandPool allocate(const gpu::LogicalDevice p_device, const gpu::QueueFamily p_queue_family)
    {
        CommandPool l_pool;
        l_pool.pool = gpu::command_pool_allocate(p_device, p_queue_family);
        return l_pool;
    };

    inline void free(const gpu::LogicalDevice p_device)
    {
        gpu::command_pool_destroy(p_device, this->pool);
    };

    inline CommandBuffer allocate_command_buffer(const gpu::LogicalDevice p_device, const gpu::Queue p_queue)
    {
        CommandBuffer l_command_buffer = CommandBuffer::build_default();
        l_command_buffer.command_buffer = gpu::command_pool_allocate_command_buffer(p_device, this->pool);
        l_command_buffer.queue = p_queue;
        l_command_buffer.device_used = p_device;

        // vkDestroySemaphore()
        // This is to avoid errors if we try to submit the command buffer if there is no previous recording.
        l_command_buffer.begin();
        l_command_buffer.end();
        l_command_buffer.submit();
        l_command_buffer.wait_for_completion();

        return l_command_buffer;
    };

    inline void free_command_buffer(const gpu::LogicalDevice p_device, const CommandBuffer& p_command_buffer)
    {
#if __DEBUG
        assert_true(p_command_buffer.debug_state == CommandBuffer::DebugState::WAITING);
#endif
        //  vkDestroySemaphore(p_device, p_command_buffer.semaphore, NULL);
    };
};
