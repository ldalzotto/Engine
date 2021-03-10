#pragma once

struct NTreeNode
{
    TokenT(NTreeNode) index;
    TokenT(NTreeNode) parent;
    PoolOfVectorToken<TokenT(NTreeNode)> childs;

    inline static NTreeNode build(const TokenT(NTreeNode) p_index, const TokenT(NTreeNode) p_parent, const PoolOfVectorToken<TokenT(NTreeNode)> p_childs)
    {
        return NTreeNode{p_index, p_parent, p_childs};
    };

    inline static NTreeNode build_index_childs(const TokenT(NTreeNode) p_index, const PoolOfVectorToken<TokenT(NTreeNode)> p_childs)
    {
        return NTreeNode{p_index, tk_bdT(NTreeNode), p_childs};
    };
};

using NTreeChildsToken = PoolOfVectorToken<TokenT(NTreeNode)>;

/*
    A NTree is a hierarchy of objects with ( Parent 1 <----> N Child ) relation ship.
*/
template <class ElementType> struct NTree
{
    Pool<ElementType> Memory;
    Pool<NTreeNode> Indices;
    PoolOfVector<TokenT(NTreeNode)> Indices_childs;

    struct Resolve
    {
        ElementType* Element;
        NTreeNode* Node;

        inline static Resolve build(ElementType* p_element, NTreeNode* p_node)
        {
            return Resolve{p_element, p_node};
        };

        inline int8 has_parent() const
        {
            return tk_v(this->Node->parent) != (token_t)-1;
        };
    };

    inline static NTree<ElementType> allocate_default()
    {
        return NTree<ElementType>{Pool<ElementType>::allocate(0), Pool<NTreeNode>::allocate(0), VectorOfVector<TokenT(NTreeNode)>::allocate_default()};
    };

    inline void free()
    {
        this->Memory.free();
        this->Indices.free();
        this->Indices_childs.free();
    };

    inline Resolve get(const TokenT(ElementType) p_token)
    {
        return Resolve::build(&this->Memory.get(p_token), &this->Indices.get(tk_bfT(NTreeNode, p_token)));
    };

    inline Resolve get_from_node(const TokenT(NTreeNode) p_token)
    {
        return this->get(tk_bfT(ElementType, p_token));
    };

    inline ElementType& get_value(const TokenT(ElementType) p_token)
    {
        return this->Memory.get(p_token);
    };

    inline Slice<TokenT(NTreeNode)> get_childs(const NTreeChildsToken p_child_token)
    {
        return this->Indices_childs.get_vector(p_child_token);
    };

    inline Slice<TokenT(NTreeNode)> get_childs_from_node(const TokenT(NTreeNode) p_node)
    {
        return this->get_childs(this->get_from_node(p_node).Node->childs);
    };

    inline int8 add_child(const Resolve& p_parent, const Resolve& p_new_child)
    {
        if (!tk_eq(p_parent.Node->index, p_new_child.Node->index))
        {
            this->detach_from_tree(p_new_child);

            p_new_child.Node->parent = p_parent.Node->index;
            this->Indices_childs.element_push_back_element(p_parent.Node->childs, p_new_child.Node->index);

            return 1;
        }
        return 0;
    };

    inline int8 add_child(const TokenT(ElementType) p_parent, const TokenT(ElementType) p_new_child)
    {
        Resolve l_new_child_value = this->get(p_new_child);
        return this->add_child(this->get(p_parent), l_new_child_value);
    };

    inline TokenT(ElementType) push_root_value(const ElementType& p_element)
    {
#if CONTAINER_BOUND_TEST
        assert_true(this->Memory.get_size() == 0);
#endif
        TokenT(ElementType) l_element;
        TokenT(NTreeNode) l_node;
        NTreeChildsToken l_childs;
        this->allocate_root_node(p_element, &l_element, &l_node, &l_childs);
        return l_element;
    };

    inline TokenT(ElementType) push_value(const ElementType& p_element, const TokenT(ElementType) p_parent)
    {
        TokenT(ElementType) l_element;
        TokenT(NTreeNode) l_node;
        NTreeChildsToken l_childs;
        this->allocate_node(tk_bfT(NTreeNode, p_parent), p_element, &l_element, &l_node, &l_childs);
        return l_element;
    };

    template <class ForEachFunc> inline void traverse3(const TokenT(NTreeNode) p_current_node, const ForEachFunc& p_foreach_func)
    {
        Resolve l_node = this->get(tk_bfT(ElementType, p_current_node));
        p_foreach_func(l_node);
        Slice<TokenT(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
        for (uimax i = 0; i < l_childs.Size; i++)
        {
            this->traverse3(l_childs.get(i), p_foreach_func);
        };
    };

    template <class ForEachFunc> inline void traverse3_excluded(const TokenT(NTreeNode) p_current_node, const ForEachFunc& p_foreach_func)
    {
        Resolve l_node = this->get(tk_bfT(ElementType, p_current_node));
        Slice<TokenT(NTreeNode)> l_childs = this->get_childs(l_node.Node->childs);
        for (uimax i = 0; i < l_childs.Size; i++)
        {
            this->traverse3(l_childs.get(i), p_foreach_func);
        };
    };

    inline void get_nodes(const TokenT(NTreeNode) p_start_node_included, Vector<Resolve>* in_out_nodes)
    {
        this->traverse3(p_start_node_included, [in_out_nodes](const Resolve& p_node) { in_out_nodes->push_back_element(p_node); });
    };

    inline void remove_node_recursively(const TokenT(NTreeNode) p_node)
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
            this->Memory.release_element(tk_bfT(ElementType, l_removed_node.Node->index));
            this->Indices.release_element(l_removed_node.Node->index);
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
    inline void allocate_node(const TokenT(NTreeNode) p_parent, const ElementType& p_element, TokenT(ElementType) * out_created_element, TokenT(NTreeNode) * out_created_index,
                              NTreeChildsToken* out_created_childs)
    {
        *out_created_element = this->Memory.alloc_element(p_element);
        *out_created_childs = this->Indices_childs.alloc_vector();
        *out_created_index = this->Indices.alloc_element(NTreeNode::build(tk_bfT(NTreeNode, *out_created_element), p_parent, *out_created_childs));

        this->Indices_childs.element_push_back_element(this->get_from_node(p_parent).Node->childs, *out_created_index);
    };

    inline void allocate_root_node(const ElementType& p_element, TokenT(ElementType) * out_created_element, TokenT(NTreeNode) * out_created_index, NTreeChildsToken* out_created_childs)
    {
        *out_created_element = this->Memory.alloc_element(p_element);
        *out_created_childs = this->Indices_childs.alloc_vector();
        *out_created_index = this->Indices.alloc_element(NTreeNode::build_index_childs(tk_bfT(NTreeNode, *out_created_element), *out_created_childs));
    };

    inline void detach_from_tree(const Resolve& p_node)
    {
        if (p_node.has_parent())
        {
            Resolve l_parent = this->get_from_node(p_node.Node->parent);
            Slice<TokenT(NTreeNode)> l_parent_childs = this->Indices_childs.get_vector(l_parent.Node->childs);
            for (loop(i, 0, l_parent_childs.Size))
            {
                if (tk_eq(l_parent_childs.get(i), p_node.Node->index))
                {
                    this->Indices_childs.element_erase_element_at_always(l_parent.Node->childs, i);
                    break;
                }
            }
        }
        p_node.Node->parent = tk_bdT(NTreeNode);
    };
};