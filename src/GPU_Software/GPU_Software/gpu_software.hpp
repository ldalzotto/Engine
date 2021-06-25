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

    inline static GPUCommand build_copy_memory(const GPUCommand_CopyMemory& p_copy_memory)
    {
        GPUCommand l_command;
        l_command.type = Type::COPY_MEMORY;
        l_command.value.copy_memory = p_copy_memory;
        return l_command;
    };
};

struct GPUCommand_Utils
{
    inline static void process_commands(Vector<GPUCommand>& p_commands)
    {
        for (loop(i, 0, p_commands.Size))
        {
            process_single_command(p_commands.get(i));
        }
    };

    inline static void process_single_command(GPUCommand& p_command)
    {
        switch (p_command.type)
        {
        case GPUCommand::Type::COPY_MEMORY:
        {
            p_command.value.copy_memory.target.copy_memory(p_command.value.copy_memory.source);
        }
        break;
        default:
            abort();
        }
    };
};

struct Buffer
{
    uimax size;
    Slice<int8> binded_memory;

    inline static Buffer build_default()
    {
        Buffer l_buffer;
        l_buffer.size = 0;
        l_buffer.binded_memory = Slice<int8>::build_default();
        return l_buffer;
    };
};

struct Image
{
    struct Format
    {
        uimax x, y, z;
    } format;

    Slice<int8> binded_memory;

    inline static Image build_default()
    {
        Image l_image;
        l_image.format = {0, 0, 0};
        l_image.binded_memory = Slice<int8>::build_default();
        return l_image;
    };

    inline static Image build_format(const Format& p_format)
    {
        Image l_image;
        l_image.format = p_format;
        l_image.binded_memory = Slice<int8>::build_default();
        return l_image;
    };
};

struct Instance
{
    Pool<Span<int8>> memory_chunks;
    Pool<Buffer> buffers;
    Pool<Image> images;
    pattern::cb::CommandPool<GPUCommand> command_pool;
    pattern::cb::CommandBufferExecutionFlow<GPUCommand> command_execution;

    inline static Instance allocate()
    {
        Instance l_instance;
        l_instance.memory_chunks = Pool<Span<int8>>::allocate(0);
        l_instance.buffers = Pool<Buffer>::allocate(0);
        l_instance.images = Pool<Image>::allocate(0);
        l_instance.command_pool = pattern::cb::CommandPool<GPUCommand>::allocate_default();
        return l_instance;
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->memory_chunks.has_allocated_elements());
        assert_true(!this->buffers.has_allocated_elements());
        assert_true(!this->images.has_allocated_elements());
#endif
        this->memory_chunks.free();
        this->buffers.free();
        this->images.free();
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

    inline void wait_for_end()
    {
        this->command_execution.process_command_buffer_tree(this->command_pool, [&](const Token<pattern::cb::CommandBuffer<GPUCommand>>, pattern::cb::CommandBuffer<GPUCommand>& p_command_buffer) {
            GPUCommand_Utils::process_commands(p_command_buffer.commands);
            p_command_buffer.commands.clear();
        });
    };
};

}; // namespace gpu_software