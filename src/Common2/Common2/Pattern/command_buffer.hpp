#pragma once

namespace pattern
{
namespace cb
{

struct Command
{
};

struct CommandBuffer
{
    Vector<Command> commands;
};

struct CommandPool
{
    Pool<CommandBuffer> command_buffers;
};

} // namespace cb
} // namespace pattern