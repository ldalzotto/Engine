#pragma once

/*
    A NTree is a hierarchy of objects with ( Parent 1 <----> N Child ) relationship.
*/
template <class ElementType> struct NTree
{
    struct Node;

    using sToken = Token<Node>;

    using tMemoryPool = Pool<ElementType>;
    using tMemoryPoolTokenValue = typename tMemoryPool::sTokenValue;

    using tIndicesPool = Pool<Node>;
    using tIndicesPoolToken = typename tIndicesPool::sToken;
    using tIndicesPoolTokenValue = typename tIndicesPool::sTokenValue;

    using tRangesPoolOfVector = PoolOfVector<sToken>;
    using tRangesPoolOfVectorToken = typename tRangesPoolOfVector::sToken;

    struct Node
    {
        sToken index;
        sToken parent;

        tRangesPoolOfVectorToken childs;

        inline static Node build(const sToken p_index, const sToken p_parent, const tRangesPoolOfVectorToken p_childs)
        {
            return Node{p_index, p_parent, p_childs};
        };

        inline static Node build_index_childs(const sToken p_index, const tRangesPoolOfVectorToken p_childs)
        {
            return Node{p_index, token_build_default<Node>(), p_childs};
        };
    };

    tMemoryPool Memory;
    tIndicesPool Indices;
    tRangesPoolOfVector Indices_childs;

    struct Resolve
    {
        ElementType* Element;
        Node* Node;

        inline static Resolve build(ElementType* p_element, struct Node* p_node)
        {
            return Resolve{p_element, p_node};
        };

        inline int8 has_parent() const
        {
            return token_value(this->Node->parent) != (token_t)-1;
        };
    };

    inline static NTree<ElementType> allocate_default()
    {
        return NTree<ElementType>{tMemoryPool::allocate(0), tIndicesPool::allocate(0), tRangesPoolOfVector::allocate_default()};
    };

    inline void free()
    {
        this->Memory.free();
        this->Indices.free();
        this->Indices_childs.free();
    };

    inline Resolve get(const sToken p_token)
    {
        return Resolve::build(&this->Memory.get(token_build_from<tMemoryPoolTokenValue>(p_token)), &this->Indices.get(token_build_from<tIndicesPoolTokenValue>(p_token)));
    };

    inline ElementType& get_value(const sToken p_token)
    {
        return this->Memory.get(token_build_from<tMemoryPoolTokenValue>(p_token));
    };

    inline Slice<sToken> get_childs(const tRangesPoolOfVectorToken p_child_token)
    {
        return this->Indices_childs.get_vector(p_child_token);
    };

    inline Slice<sToken> get_childs_from_node(const sToken p_node)
    {
        return this->get_childs(this->get(p_node).Node->childs);
    };

    inline int8 add_child_silent(const Resolve& p_parent, const Resolve& p_new_child)
    {
        if (!token_equals(p_parent.Node->index, p_new_child.Node->index))
        {
            this->add_child_unsafe(p_parent, p_new_child);
            return 1;
        }
        return 0;
    };

    inline int8 add_child_silent(const sToken p_parent, const sToken p_new_child)
    {
        Resolve l_new_child_value = this->get(p_new_child);
        return this->add_child_silent(this->get(p_parent), l_new_child_value);
    };

    inline void add_child(const Resolve& p_parent, const Resolve& p_new_child)
    {
#if __DEBUG
        // The node child cannot be the node parent
        if (token_equals(p_parent.Node->index, p_new_child.Node->index))
        {
            abort();
        }
#endif

        this->add_child_unsafe(p_parent, p_new_child);
    };

    inline void add_child(const sToken p_parent, const sToken p_new_child)
    {
        this->add_child(this->get(p_parent), this->get(p_new_child));
    }

    inline sToken push_root_value(const ElementType& p_element)
    {
#if __DEBUG
        assert_true(this->Memory.get_size() == 0);
#endif
        sToken l_node;
        tRangesPoolOfVectorToken l_childs;
        this->allocate_root_node(p_element, &l_node, &l_childs);
        return l_node;
    };

    inline sToken push_value(const ElementType& p_element, const sToken p_parent)
    {
        sToken l_node;
        tRangesPoolOfVectorToken l_childs;
        this->allocate_node(p_parent, p_element, &l_node, &l_childs);
        return l_node;
    };

    template <class ForEachFunc> inline void traverse3(const sToken p_current_node, const ForEachFunc& p_foreach_func)
    {
        Resolve l_node = this->get(p_current_node);
        p_foreach_func(l_node);
        Slice<sToken> l_childs = this->get_childs(l_node.Node->childs);
        for (uimax i = 0; i < l_childs.Size; i++)
        {
            this->traverse3(l_childs.get(i), p_foreach_func);
        };
    };

    template <class ForEachFunc> inline void traverse3_excluded(const sToken p_current_node, const ForEachFunc& p_foreach_func)
    {
        Resolve l_node = this->get(p_current_node);
        Slice<sToken> l_childs = this->get_childs(l_node.Node->childs);
        for (uimax i = 0; i < l_childs.Size; i++)
        {
            this->traverse3(l_childs.get(i), p_foreach_func);
        };
    };

    inline void get_nodes(const sToken p_start_node_included, Vector<Resolve>* in_out_nodes)
    {
        this->traverse3(p_start_node_included, [in_out_nodes](const Resolve& p_node) {
            in_out_nodes->push_back_element(p_node);
        });
    };

    inline void remove_node_recursively(const sToken p_node)
    {
        Vector<Resolve> l_involved_nodes = Vector<Resolve>::allocate(0);
        this->get_nodes(p_node, &l_involved_nodes);

        Slice<Resolve> l_involved_nodes_slice = l_involved_nodes.to_slice();
        this->remove_nodes_and_detach(l_involved_nodes_slice);

        l_involved_nodes.free();
    };

    inline void remove_nodes(const Slice<Resolve>& p_removed_nodes)
    {
        for (loop(i, 0, p_removed_nodes.Size))
        {
            const Resolve& l_removed_node = p_removed_nodes.get(i);
            this->Memory.release_element(token_build_from<tMemoryPoolTokenValue>(l_removed_node.Node->index));
            this->Indices.release_element(token_build_from<tIndicesPoolTokenValue>(l_removed_node.Node->index));
            this->Indices_childs.release_vector(l_removed_node.Node->childs);
        }
    };

    inline void remove_nodes_and_detach(Slice<Resolve>& p_removed_nodes)
    {
        this->detach_from_tree(p_removed_nodes.get(0));
        this->remove_nodes(p_removed_nodes);
    };

    inline void make_node_orphan(Resolve& p_node)
    {
        this->detach_from_tree(p_node);
    };

  private:
    inline void allocate_node(const sToken p_parent, const ElementType& p_element, sToken* out_created_node, tRangesPoolOfVectorToken* out_created_childs)
    {
        *out_created_node = token_build_from<Node>(this->Memory.alloc_element(p_element));
        *out_created_childs = this->Indices_childs.alloc_vector();
        tIndicesPoolToken l_created_index = this->Indices.alloc_element(Node::build(*out_created_node, p_parent, *out_created_childs));

#if __DEBUG
        assert_true(token_equals(l_created_index, *out_created_node));
#endif

        this->Indices_childs.element_push_back_element(this->get(p_parent).Node->childs, token_build_from<Node>(l_created_index));
    };

    inline void allocate_root_node(const ElementType& p_element, sToken* out_created_node, tRangesPoolOfVectorToken* out_created_childs)
    {
        *out_created_node = token_build_from<Node>(this->Memory.alloc_element(p_element));
        *out_created_childs = this->Indices_childs.alloc_vector();
        tIndicesPoolToken l_created_index = this->Indices.alloc_element(Node::build_index_childs(*out_created_node, *out_created_childs));

#if __DEBUG
        assert_true(token_equals(l_created_index, *out_created_node));
#endif
    };

    inline void detach_from_tree(const Resolve& p_node)
    {
        if (p_node.has_parent())
        {
            Resolve l_parent = this->get(p_node.Node->parent);
            Slice<sToken> l_parent_childs = this->Indices_childs.get_vector(l_parent.Node->childs);
            for (loop(i, 0, l_parent_childs.Size))
            {
                if (token_equals(l_parent_childs.get(i), p_node.Node->index))
                {
                    this->Indices_childs.element_erase_element_at_always(l_parent.Node->childs, i);
                    break;
                }
            }
        }
        p_node.Node->parent = token_build_default<Node>();
    };

    inline void add_child_unsafe(const Resolve& p_parent, const Resolve& p_new_child)
    {
        this->detach_from_tree(p_new_child);

        p_new_child.Node->parent = p_parent.Node->index;
        this->Indices_childs.element_push_back_element(p_parent.Node->childs, p_new_child.Node->index);
    };
};

/*
    A NNTree is a hierarchy of objects with ( Parent N <----> N Child ) relationship.
 */
template <class ElementType> struct NNTree
{
    struct Node;

    using sTokenValue = typename Pool<Node>::sTokenValue;
    using sToken = typename Pool<Node>::sToken;

    using tElement = ElementType;
    using tMemoryPool = Pool<tElement>;
    using tNodesPool = Pool<Node>;
    using tRangesPoolOfVector = PoolOfVector<sToken>;

    struct Node
    {
        typename tRangesPoolOfVector::sToken parents;
        typename tRangesPoolOfVector::sToken childs;
    };

    tMemoryPool Memory;
    tNodesPool Nodes;
    tRangesPoolOfVector Ranges;

    struct Resolve
    {
        sToken node_token;
        ElementType* Element;
        Node* Node;

        inline static Resolve build_default()
        {
            Resolve l_return;
            l_return.node_token = token_build_default<sTokenValue>();
            l_return.Node = NULL;
            l_return.Element = NULL;
            return l_return;
        };
    };

    inline static NNTree<ElementType> allocate_default()
    {
        NNTree<ElementType> l_return;
        l_return.Memory = tMemoryPool::allocate(0);
        l_return.Nodes = tNodesPool::allocate(0);
        l_return.Ranges = tRangesPoolOfVector::allocate_default();
        return l_return;
    };

    inline void free()
    {
        this->Memory.free();
        this->Nodes.free();
        this->Ranges.free();
    };

    inline int8 has_allocated_elements()
    {
        return this->Memory.has_allocated_elements() && this->Nodes.has_allocated_elements() && this->Ranges.has_allocated_elements();
    };

    inline int8 is_node_free(const sToken p_node)
    {
        return this->Memory.is_element_free(token_build_from<tMemoryPool::sTokenValue>(p_node)) && this->Nodes.is_element_free(p_node);
    };

    inline sToken push_root_value(const ElementType& p_element)
    {
        return this->allocate_node(p_element, Slice<sToken>::build_default());
    };

    inline sToken push_value(const ElementType& p_element, const Slice<sToken>& p_parents)
    {
        return this->allocate_node(p_element, p_parents);
    };

    inline Resolve get(const sToken p_element)
    {
        Resolve l_return;
        l_return.node_token = p_element;
        l_return.Node = &this->Nodes.get(p_element);
        l_return.Element = &this->Memory.get(token_build_from<tMemoryPool::sTokenValue>(p_element));
        return l_return;
    };

    inline Slice<sToken> get_childs(const Resolve& p_node)
    {
        return this->Ranges.get_element_as_iVector(p_node.Node->childs).to_slice();
    };

    inline Slice<sToken> get_parents(const Resolve& p_node)
    {
        return this->Ranges.get_element_as_iVector(p_node.Node->parents).to_slice();
    };

    inline void add_child(const sToken p_parent, const sToken p_child)
    {
#if __DEBUG
        assert_true(!token_equals(p_parent, p_child));
        // Child/Parent validation is done in add_child_to, add_parent_to functions
#endif
        this->add_child_to(this->get(p_parent), p_child, this->get(p_child));
    };

    inline void remove_node_recursively(const sToken p_node)
    {
        // (parent) - (child) relationship
        // /!\ We use Token<> instead of Resolve because we will execute node removal fo every connection, and the the memory can be reallocated or moved.
        struct NodeConnection
        {
            sToken parent;
            sToken child;
        };

        Vector<NodeConnection> l_connections_that_will_be_removed = Vector<NodeConnection>::allocate(0);

        this->traverse_to_bottom_excluded(this->get(p_node), [&](const Resolve& p_parent, const Resolve& p_child_node) {
            NodeConnection l_pair;
            l_pair.parent = p_parent.node_token;
            l_pair.child = p_child_node.node_token;
            l_connections_that_will_be_removed.push_back_element(l_pair);
        });

        for (loop(i, 0, l_connections_that_will_be_removed.Size))
        {
            NodeConnection& l_pair = l_connections_that_will_be_removed.get(i);
            Resolve l_child = this->get(l_pair.child);
            Resolve l_parent = this->get(l_pair.parent);
            this->remove_link_bidirectional(l_child, l_parent);
        }

        // We remove nodes on a second path because there is a possibility that a parent is removed while still having childs that needs to be removed.
        // This is due to the fact that we iterate the tree from top to bottom.
        for (loop(i, 0, l_connections_that_will_be_removed.Size))
        {
            NodeConnection& l_pair = l_connections_that_will_be_removed.get(i);
            Resolve l_child = this->get(l_pair.child);
            if (this->get_parents(l_child).Size == 0)
            {
                this->remove_node(l_child);
            }
        }

        Resolve l_node = this->get(p_node);
        Slice<sToken> l_node_parents = this->get_parents(l_node);
        for (loop(i, 0, l_node_parents.Size))
        {
            this->remove_link_bidirectional(l_node, this->get(l_node_parents.get(i)));
        }
        this->remove_node(l_node);
        l_connections_that_will_be_removed.free();
    };

    // TODO -> adding a way to abort the traversal ?
    template <class _ForeachFunc> inline void traverse_to_bottom(const Resolve& p_start_node_included, const _ForeachFunc& p_foreach)
    {
        p_foreach(Resolve::build_default(), p_start_node_included);
        this->traverse_to_bottom_excluded(p_start_node_included, p_foreach);
    };

    template <class _ForeachFunc> inline void traverse_to_bottom_excluded(const Resolve& p_start_node_excluded, const _ForeachFunc& p_foreach)
    {
        Slice<sToken> l_childs = this->get_childs(p_start_node_excluded);
        for (loop(i, 0, l_childs.Size))
        {
            Resolve l_node = this->get(l_childs.get(i));
            p_foreach(p_start_node_excluded, l_node);
            this->traverse_to_bottom_excluded(l_node, p_foreach);
        }
    };

    /*
        Perform a traversal while ensuring that all nodes are executed only once.
    */
    template <class _ForeachFunc> inline void traverse_to_bottom_distinct(const Resolve& p_start_node_included, const _ForeachFunc& p_foreach)
    {
        Span<int8> l_execution_states = Span<int8>::allocate(this->Nodes.get_size());
        l_execution_states.slice.zero();

        this->traverse_to_bottom(p_start_node_included, [&](const Resolve& p_parent, const Resolve& p_node) {
            int8& l_exectuion_state = l_execution_states.get(token_value(p_node.node_token));
            if (!l_exectuion_state)
            {
                l_exectuion_state = 1;
                p_foreach(p_parent, p_node);
            }
        });

        l_execution_states.free();
    };

    /*
        Perform a traversal while ensuring that all nodes are executed only once.
    */
    template <class _ForeachFunc> inline void traverse_to_bottom_distinct_excluded(const Resolve& p_start_node_excluded, const _ForeachFunc& p_foreach)
    {
        Span<int8> l_execution_states = Span<int8>::allocate(this->Nodes.get_size());
        l_execution_states.slice.zero();

        this->traverse_to_bottom_excluded(p_start_node_excluded, [&](const Resolve& p_parent, const Resolve& p_node) {
            int8& l_exectuion_state = l_execution_states.get(token_value(p_node.node_token));
            if (!l_exectuion_state)
            {
                l_exectuion_state = 1;
                p_foreach(p_parent, p_node);
            }
        });

        l_execution_states.free();
    };

  private:
    inline sToken allocate_node(const ElementType& p_element, const Slice<sToken>& p_parents)
    {
        sToken l_element = token_build_from<sTokenValue>(this->Memory.alloc_element(p_element));
        Node l_node;
        l_node.childs = this->Ranges.alloc_vector();
        l_node.parents = this->Ranges.alloc_vector_with_values(Slice<sToken>::build_memory_elementnb((sToken*)p_parents.Begin, p_parents.Size));
        this->Nodes.alloc_element(l_node);

        Resolve l_inserted_node_resolve = this->get(l_element);
        for (loop(i, 0, p_parents.Size))
        {
            this->add_child_to(this->get(p_parents.get(i)), l_element, l_inserted_node_resolve);
        }
        return l_element;
    };

    inline void remove_node(const Resolve& p_node)
    {
        this->Memory.release_element(token_build_from<tMemoryPool::sTokenValue>(p_node.node_token));
        this->Ranges.release_vector(p_node.Node->childs);
        this->Ranges.release_vector(p_node.Node->parents);
        this->Nodes.release_element(p_node.node_token);
    };

    inline void remove_link_bidirectional(const Resolve& p_child_node, const Resolve& p_parent_node)
    {
        this->remove_parent_to(p_child_node, p_parent_node.node_token, p_parent_node);
        this->remove_child_to(p_parent_node, p_child_node.node_token, p_child_node);
    };

    inline void add_parent_to(const Resolve& p_node, const sToken p_parent_token, const Resolve& p_parent)
    {
        typename tRangesPoolOfVector::Element_iVector l_parents = this->Ranges.get_element_as_iVector(p_node.Node->parents);

#if __DEBUG
        for (loop(i, 0, l_parents.get_size()))
        {
            if (token_equals(l_parents.get(i), p_parent_token))
            {
                abort();
            }
        }
#endif

        l_parents.push_back_element(p_parent_token);
    };

    inline void add_child_to(const Resolve& p_node, const sToken p_child_token, const Resolve& p_child)
    {
        typename tRangesPoolOfVector::Element_iVector l_childs = this->Ranges.get_element_as_iVector(p_node.Node->childs);

#if __DEBUG
        for (loop(i, 0, l_childs.get_size()))
        {
            if (token_equals(l_childs.get(i), p_child_token))
            {
                abort();
            }
        }
#endif

        l_childs.push_back_element(p_child_token);
    };

    inline void remove_parent_to(const Resolve& p_node, const sToken p_parent_token, const Resolve& p_parent)
    {
        typename tRangesPoolOfVector::Element_iVector l_parents = this->Ranges.get_element_as_iVector(p_node.Node->parents);

        for (loop(i, 0, l_parents.get_size()))
        {
            if (token_equals(l_parents.get(i), p_parent_token))
            {
                l_parents.erase_element_at_always(i);
                return;
            }
        }
    };

    inline void remove_child_to(const Resolve& p_node, const sToken p_child_token, const Resolve& p_child)
    {
        typename tRangesPoolOfVector::Element_iVector l_childs = this->Ranges.get_element_as_iVector(p_node.Node->childs);

        for (loop(i, 0, l_childs.get_size()))
        {
            if (token_equals(l_childs.get(i), p_child_token))
            {
                l_childs.erase_element_at_always(i);
                return;
            }
        }
    };
};