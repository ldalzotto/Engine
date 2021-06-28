#pragma once

namespace pattern
{
namespace cb
{

template <class Command> struct CommandBuffer
{
    using tCommandVector = Vector<Command>;

    tCommandVector commands;

    inline static CommandBuffer allocate_default()
    {
        return CommandBuffer{tCommandVector::allocate(0)};
    };

    inline void free()
    {
        this->commands.free();
    }
};

template <class Command> struct CommandPool
{
    using tCommandBufferPool = Pool<CommandBuffer<Command>>;
    using sToken = typename tCommandBufferPool::sToken;

    tCommandBufferPool command_buffers;

    inline static CommandPool allocate_default()
    {
        return CommandPool<Command>{tCommandBufferPool::allocate(0)};
    };

    inline sToken allocate_command_buffer()
    {
        CommandBuffer<Command> l_command_buffer = CommandBuffer<Command>::allocate_default();
        return this->command_buffers.alloc_element(l_command_buffer);
    };

    inline void free_command_buffer(const sToken p_token)
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
    using tExecutionToken = typename NNTree<typename CommandPool<Command>::sToken>::sToken;
    tExecutionToken execute_before;
};

template <class Command> struct CommandBufferExecutionFlow
{
    using tCommandBufferTree = NNTree<typename Pool<CommandBuffer<Command>>::sToken>;
    using tCommandBufferTreeResolve = typename tCommandBufferTree::Resolve;
    using tCommandBufferTreeElement = typename tCommandBufferTree::tElement;
    using tCommandBufferTreeToken = typename tCommandBufferTree::sToken;
    using tCommandBufferTreeTokenValue = typename tCommandBufferTree::sTokenValue;

    tCommandBufferTree command_tree;
    tCommandBufferTreeToken command_tree_root;

    inline static CommandBufferExecutionFlow allocate_default()
    {
        CommandBufferExecutionFlow<Command> l_return;
        l_return.command_tree = tCommandBufferTree::allocate_default();
        l_return.command_tree_root = l_return.command_tree.push_root_value(tCommandBufferTree::tElement{(token_t)-1});
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

    inline tCommandBufferTreeToken push_command_buffer(const tCommandBufferTreeElement p_command)
    {
        SliceN<tCommandBufferTreeToken, 1> l_parents = {this->command_tree_root};
        return this->command_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    inline tCommandBufferTreeToken push_command_buffer_with_constraint(const tCommandBufferTreeElement p_command, const Semaphore<Command> p_semaphore)
    {
        SliceN<tCommandBufferTreeToken, 1> l_parents = {p_semaphore.execute_before};
        return this->command_tree.push_value(p_command, slice_from_slicen(&l_parents));
    };

    template <class _ForeachFunc> inline void process_command_buffer_tree(CommandPool<Command>& p_command_pool, const _ForeachFunc& p_foreach)
    {
        this->command_tree.traverse_to_bottom_distinct_excluded(this->command_tree.get(token_build<tCommandBufferTreeTokenValue>(0)),
                                                                [&](const tCommandBufferTreeResolve& p_parent, const tCommandBufferTreeResolve& p_node) {
                                                                    tCommandBufferTreeElement l_command_token = *p_node.Element;
                                                                    p_foreach(l_command_token, p_command_pool.command_buffers.get(l_command_token));
                                                                });
        this->command_tree.remove_node_recursively(this->command_tree_root);
        this->command_tree_root = this->command_tree.push_root_value(tCommandBufferTreeElement{(token_t)-1});
    };
};

} // namespace cb
} // namespace pattern