#pragma once

#include "Common2/Common2.hpp"

namespace gpu_software
{

struct GPUCommand_CopyMemory
{
    Slice<int8> source;
    Slice<int8> target;
};

struct GPUCommand
{
    using Type_t = int8;
    enum class Type : Type_t
    {
        UNKNOWN = 0,
        COPY_MEMORY = 1
    } type;

    union
    {
        GPUCommand_CopyMemory copy_memory;
    } value;
};

struct Instance
{
    Pool<Span<int8>> memory_chunks;
    pattern::cb::CommandPool<GPUCommand> command_pool;
    pattern::cb::CommandBufferExecutionFlow<GPUCommand> command_execution;

    inline static Instance allocate()
    {
        Instance l_instance;
        l_instance.memory_chunks = Pool<Span<int8>>::allocate(0);
        l_instance.command_pool = pattern::cb::CommandPool<GPUCommand>::allocate_default();
        return l_instance;
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->memory_chunks.has_allocated_elements());
#endif
        this->memory_chunks.free();
        this->command_pool.free();
    };

    inline Token<Span<int8>> memory_allocate(const uimax p_size)
    {
        return this->memory_chunks.alloc_element(Span<int8>::allocate(p_size));
    };

    inline void memory_free(const Token<Span<int8>> p_memory)
    {
        this->memory_chunks.get(p_memory).free();
        this->memory_chunks.release_element(p_memory);
    };

    inline void command_buffer_submit(const Token<pattern::cb::CommandBuffer<GPUCommand>> p_command_buffer)
    {
        this->command_execution.push_command_buffer(p_command_buffer);
    };

    inline void command_buffer_submit_wait_for(const Token<pattern::cb::CommandBuffer<GPUCommand>> p_command_buffer, const pattern::cb::Semaphore<GPUCommand> p_wait_for)
    {
        this->command_execution.push_command_buffer_with_constraint(p_command_buffer, p_wait_for);
    };
};

}; // namespace gpu_software