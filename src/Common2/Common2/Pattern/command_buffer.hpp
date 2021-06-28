#pragma once

namespace pattern
{
namespace cb
{

template <class Command> struct CommandBuffer
{
    using t_CommandVector = Vector<Command>;

    t_CommandVector commands;

    inline static CommandBuffer allocate_default()
    {
        return CommandBuffer{t_CommandVector::allocate(0)};
    };

    inline void free()
    {
        this->commands.free();
    }
};

template <class Command> struct CommandPool
{
    using t_CommandBufferPool = Pool<CommandBuffer<Command>>;
    using t_CommandBufferPool_sToken = typename t_CommandBufferPool::sToken;

    t_CommandBufferPool command_buffers;

    inline static CommandPool allocate_default()
    {
        return CommandPool<Command>{t_CommandBufferPool::allocate(0)};
    };

    inline t_CommandBufferPool_sToken allocate_command_buffer()
    {
        CommandBuffer<Command> l_command_buffer = CommandBuffer<Command>::allocate_default();
        return this->command_buffers.alloc_element(l_command_buffer);
    };

    inline void free_command_buffer(const t_CommandBufferPool_sToken p_token)
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
    using t_Execution_Token = typename NNTree<typename CommandPool<Command>::t_CommandBufferPool_sToken>::sToken;
    t_Execution_Token execute_before;
};

template <class Command> struct CommandBufferExecutionFlow
{
    using t_CommandBufferExecutionTree = NNTree<typename Pool<CommandBuffer<Command>>::sToken>;
    using t_CommandBufferExecutionTree_Resolve = typename t_CommandBufferExecutionTree::Resolve;
    using t_CommandBufferExecutionTree_Element = typename t_CommandBufferExecutionTree::t_Element;
    using t_CommandBufferExecutionTree_sToken = typename t_CommandBufferExecutionTree::sToken;
    using t_CommandBufferExecutionTree_sTokenValue = typename t_CommandBufferExecutionTree::sTokenValue;

    t_CommandBufferExecutionTree command_execution_tree;
    t_CommandBufferExecutionTree_sToken command_execution_tree_root;

    inline static CommandBufferExecutionFlow allocate_default()
    {
        CommandBufferExecutionFlow<Command> l_return;
        l_return.command_execution_tree = t_CommandBufferExecutionTree::allocate_default();
        l_return.command_execution_tree_root = l_return.command_execution_tree.push_root_value(t_CommandBufferExecutionTree::t_Element{(token_t)-1});
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

    inline t_CommandBufferExecutionTree_sToken push_command_buffer(const t_CommandBufferExecutionTree_Element p_command)
    {
        SliceN<t_CommandBufferExecutionTree_sToken, 1> l_parents = {this->command_execution_tree_root};
        return this->command_execution_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    inline t_CommandBufferExecutionTree_sToken push_command_buffer_with_constraint(const t_CommandBufferExecutionTree_Element p_command, const Semaphore<Command> p_semaphore)
    {
        SliceN<t_CommandBufferExecutionTree_sToken, 1> l_parents = {p_semaphore.execute_before};
        return this->command_execution_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    template <class _ForeachFunc> inline void process_command_buffer_tree(CommandPool<Command>& p_command_pool, const _ForeachFunc& p_foreach)
    {
        this->command_execution_tree.traverse_to_bottom_distinct_excluded(this->command_execution_tree.get(token_build<t_CommandBufferExecutionTree_sTokenValue>(0)),
                                                                          [&](const t_CommandBufferExecutionTree_Resolve& p_parent, const t_CommandBufferExecutionTree_Resolve& p_node) {
                                                                              t_CommandBufferExecutionTree_Element l_command_token = *p_node.Element;
                                                                              p_foreach(l_command_token, p_command_pool.command_buffers.get(l_command_token));
                                                                          });
        this->command_execution_tree.remove_node_recursively(this->command_execution_tree_root);
        this->command_execution_tree_root = this->command_execution_tree.push_root_value(t_CommandBufferExecutionTree_Element{(token_t)-1});
    };
};

} // namespace cb
} // namespace pattern