#pragma once

namespace pattern
{
namespace cb
{

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
    Pool<CommandBuffer<Command>> command_buffers;

    inline static CommandPool allocate_default()
    {
        return CommandPool<Command>{Pool<CommandBuffer<Command>>::allocate(0)};
    };

    inline typename Pool<CommandBuffer<Command>>::sToken allocate_command_buffer()
    {
        CommandBuffer<Command> l_command_buffer = CommandBuffer<Command>::allocate_default();
        return this->command_buffers.alloc_element(l_command_buffer);
    };

    inline void free_command_buffer(const typename Pool<CommandBuffer<Command>>::sToken p_token)
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
    typename NNTree<typename Pool<CommandBuffer<Command>>::sToken>::sToken execute_before;
};

// TODO -> type clean
template <class Command> struct CommandBufferExecutionFlow
{
    NNTree<typename Pool<CommandBuffer<Command>>::sToken> command_tree;
    typename NNTree<typename Pool<CommandBuffer<Command>>::sToken>::sToken command_tree_root;

    inline static CommandBufferExecutionFlow allocate_default()
    {
        CommandBufferExecutionFlow<Command> l_return;
        l_return.command_tree = NNTree<typename Pool<CommandBuffer<Command>>::sToken>::allocate_default();
        l_return.command_tree_root = l_return.command_tree.push_root_value(token_build_default<Pool<CommandBuffer<Command>>::sTokenValue>());
        return l_return;
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->command_tree.Nodes.get_free_size() == (this->command_tree.Nodes.get_size() - 1)); // Only the root element is still allocated
        assert_true(!this->command_tree.is_node_free(this->command_tree_root));
#endif
        this->command_tree.free();
    };

    inline typename NNTree<typename Pool<CommandBuffer<Command>>::sToken>::sToken push_command_buffer(const typename Pool<CommandBuffer<Command>>::sToken p_command)
    {
        SliceN<typename NNTree<typename Pool<CommandBuffer<Command>>::sToken>::sToken, 1> l_parents = {this->command_tree_root};
        return this->command_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    inline typename NNTree<typename Pool<CommandBuffer<Command>>::sToken>::sToken push_command_buffer_with_constraint(const typename Pool<CommandBuffer<Command>>::sToken p_command,
                                                                                                      const Semaphore<Command> p_semaphore)
    {
        SliceN<typename NNTree<typename Pool<CommandBuffer<Command>>::sToken>::sToken, 1> l_parents = {p_semaphore.execute_before};
        return this->command_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    template <class _ForeachFunc> inline void process_command_buffer_tree(CommandPool<Command>& p_command_pool, const _ForeachFunc& p_foreach)
    {
        this->command_tree.traverse_to_bottom_distinct_excluded(this->command_tree.get(token_build<NNTree<Pool<CommandBuffer<Command>>::sToken>::sTokenValue>(0)),
                                                                [&](const NNTree<Pool<CommandBuffer<Command>>::sToken>::Resolve& p_parent, const NNTree<Pool<CommandBuffer<Command>>::sToken>::Resolve& p_node) {
																	typename Pool<CommandBuffer<Command>>::sToken l_command_token = *p_node.Element;
                                                                    p_foreach(l_command_token, p_command_pool.command_buffers.get(l_command_token));
                                                                });
        this->command_tree.remove_node_recursively(this->command_tree_root);
        this->command_tree_root = this->command_tree.push_root_value(token_build_default<Pool<CommandBuffer<Command>>::sTokenValue>());
    };
};

} // namespace cb
} // namespace pattern