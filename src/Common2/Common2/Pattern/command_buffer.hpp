#pragma once

namespace pattern
{
namespace cb
{

template <class Command> struct CommandBuffer;

namespace CommandBuffer_Types
{
template <class Command> using CommandBuffer_Pool = Pool<CommandBuffer<Command>>;
template <class Command> using CommandBuffer_Token = typename CommandBuffer_Pool<Command>::sToken;
template <class Command> using CommandBufferExecution_Tree = NNTree<CommandBuffer_Token<Command>>;
template <class Command> using CommandBufferExecution_Token = typename CommandBufferExecution_Tree<Command>::sToken;
}; // namespace CommandBuffer_Types

#define CommandBufferTypes_forward(CommandType)                                                                                                                                                        \
    using CommandBuffer_Pool = pattern::cb::CommandBuffer_Types::CommandBuffer_Pool<CommandType>;                                                                                                      \
    using CommandBuffer_Token = pattern::cb::CommandBuffer_Types::CommandBuffer_Token<CommandType>;                                                                                                    \
    using CommandBufferExecution_Tree = pattern::cb::CommandBuffer_Types::CommandBufferExecution_Tree<CommandType>;                                                                                    \
    using CommandBufferExecution_Token = pattern::cb::CommandBuffer_Types::CommandBufferExecution_Token<CommandType>;

template <class Command> struct CommandBuffer
{
    Vector<Command> commands;

    inline static CommandBuffer allocate_default()
    {
        return CommandBuffer{Vector<Command>::allocate(0)};
    };

    inline void free()
    {
        this->commands.free();
    }
};

template <class Command> struct CommandPool
{
    CommandBufferTypes_forward(Command);

    CommandBuffer_Pool command_buffers;

    inline static CommandPool allocate_default()
    {
        return CommandPool<Command>{CommandBuffer_Pool::allocate(0)};
    };

    inline CommandBuffer_Token allocate_command_buffer()
    {
        CommandBuffer<Command> l_command_buffer = CommandBuffer<Command>::allocate_default();
        return this->command_buffers.alloc_element(l_command_buffer);
    };

    inline void free_command_buffer(const CommandBuffer_Token p_token)
    {
        this->command_buffers.get(p_token).free();
        this->command_buffers.release_element(p_token);
    };

    inline void free()
    {
#if __DEBUG
        assert_true(!this->command_buffers.has_allocated_elements());
#endif
        this->command_buffers.free();
    };
};

template <class Command> struct Semaphore
{
    CommandBufferTypes_forward(Command);

    CommandBufferExecution_Token execute_before;
};

template <class Command> struct CommandBufferExecutionFlow
{
    CommandBufferTypes_forward(Command);

    CommandBufferExecution_Tree command_execution_tree;
    CommandBufferExecution_Token command_execution_tree_root;

    inline static CommandBufferExecutionFlow allocate_default()
    {
        CommandBufferExecutionFlow<Command> l_return;
        l_return.command_execution_tree = CommandBufferExecution_Tree::allocate_default();
        l_return.command_execution_tree_root = l_return.command_execution_tree.push_root_value(CommandBuffer_Token::build_default());
        return l_return;
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->command_execution_tree.Nodes.get_free_size() == (this->command_execution_tree.Nodes.get_size() - 1)); // Only the root element is still allocated
        assert_true(!this->command_execution_tree.is_node_free(this->command_execution_tree_root));
#endif
        this->command_execution_tree.free();
    };

    inline CommandBufferExecution_Token push_command_buffer(const CommandBuffer_Token p_command)
    {
        SliceN<CommandBufferExecution_Token, 1> l_parents = {this->command_execution_tree_root};
        return this->command_execution_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    inline CommandBufferExecution_Token push_command_buffer_with_constraint(const CommandBuffer_Token p_command, const Semaphore<Command> p_semaphore)
    {
        SliceN<CommandBufferExecution_Token, 1> l_parents = {p_semaphore.execute_before};
        return this->command_execution_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    template <class _ForeachFunc> inline void process_command_buffer_tree(CommandPool<Command>& p_command_pool, const _ForeachFunc& p_foreach)
    {
        this->command_execution_tree.traverse_to_bottom_distinct_excluded(this->command_execution_tree.get(CommandBufferExecution_Token::build(0)),
                                                                          [&](const CommandBufferExecution_Tree::Resolve& p_parent, const CommandBufferExecution_Tree::Resolve& p_node) {
                                                                              CommandBuffer_Token l_command_token = *p_node.Element;
                                                                              p_foreach(l_command_token, p_command_pool.command_buffers.get(l_command_token));
                                                                          });
        this->command_execution_tree.remove_node_recursively(this->command_execution_tree_root);
        this->command_execution_tree_root = this->command_execution_tree.push_root_value(CommandBuffer_Token::build_default());
    };
};

} // namespace cb
} // namespace pattern