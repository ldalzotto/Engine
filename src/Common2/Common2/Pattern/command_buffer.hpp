#pragma once

namespace pattern
{
namespace cb
{

template <class Command> struct CommandBuffer
{
    Vector<Command> commands;
};

template <class Command> struct CommandPool
{
    Pool<CommandBuffer<Command>> command_buffers;
};

struct CommandUtils
{
    template <class Command, class _ForeachFunc> inline static void process_command_tree(NNTree<Command>& p_commands, const _ForeachFunc& p_foreach)
    {
        // TODO -> having an algorithm that iterates from the root and gives
        // This is wrong
        p_commands.traverse_to_bottom_distinct(p_commands.get(token_build_default<NNTree<Command>::Node>(p_foreach)));
    };
};

// NNTree<Token<CommandBuffer>> command_buffers;
// -> this is the execution tree

} // namespace cb
} // namespace pattern