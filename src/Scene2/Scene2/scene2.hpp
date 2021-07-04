#pragma once

#include "Math2/math.hpp"

using component_t = hash_t;

/*
    Value associated to every node of the SceneTree.
    The Node stores local_transform and localtoworld matrix.
*/
struct Node
{
    /*
        The State of a Node idicates wether the node has moved since last Scene step and if the model matrices must be recalculated.
    */
    struct State
    {
        // TODO -> we can merge these two int8 (bool) as a single one and execute bit comparisons
        int8 matrices_mustBe_recalculated; /* = true;*/
        int8 haschanged_thisframe;         /* = false; */

        inline static State build(const int8 p_matrices_must_be_recalculated, const int8 p_haschanged_thisframe)
        {
            return State{p_matrices_must_be_recalculated, p_haschanged_thisframe};
        };

    } state;

    // transform
    transform local_transform;

    /** This matrix will always be relative to the root Node (a Node without parent). */
    m44f localtoworld;

    inline static Node build_default()
    {
        Node l_node;
        l_node.state = State{true, false};
        return l_node;
    };
    inline static Node build(const State& p_state, const transform& p_local_transform)
    {
        return Node{p_state, p_local_transform};
    };

    inline void mark_for_recaluclation()
    {
        this->state = State{1, 1};
    };
};

using NodeEntry = NTree<Node>::Resolve;
using Node_Token = NTree<Node>::sToken;
using Node_TokenValue = NTree<Node>::sTokenValue;

struct NodeComponent
{
    component_t type;
    token_t resource;

    inline static NodeComponent build_default()
    {
        return NodeComponent{(component_t)-1, (token_t)-1};
    };
    inline static NodeComponent build(const component_t p_type, const token_t p_resource)
    {
        return NodeComponent{p_type, p_resource};
    };
};

/*
    A SceneTree provides convenient methods to add, detach and remove nodes from the tree.
    It handles mathematic operations for local and world (position/rotation/scale/model matrix).
*/
struct SceneTree
{
    NTree<Node> node_tree;

    inline static SceneTree allocate_default()
    {
        SceneTree l_scene = SceneTree{NTree<Node>::allocate_default()};

        l_scene.allocate_root_node();

        return l_scene;
    };

    inline void free()
    {
#if __DEBUG
        NodeEntry l_root_node = this->get_node(token_build<Node_TokenValue>(0));
        assert_true(this->get_node_childs(l_root_node).Size == 0);
#endif

        this->node_tree.free();
    };

    inline Node_Token add_node(const transform& p_initial_local_transform, const Node_Token p_parent)
    {
        return this->allocate_node(p_initial_local_transform, p_parent);
    };

    inline NodeEntry get_node(const Node_Token p_node)
    {
        return this->node_tree.get(p_node);
    };

    inline NodeEntry get_node_parent(const NodeEntry& p_node)
    {
        return this->get_node(p_node.Node->parent);
    };

    inline Slice<Node_Token> get_node_childs(const NodeEntry& p_node)
    {
        Slice<Token<NTree<Node>::Node>> l_childs = this->node_tree.get_childs(p_node.Node->childs);
        return Slice<Node_Token>::build_memory_elementnb((Node_Token*)l_childs.Begin, l_childs.Size);
    };

    inline void add_child(const NodeEntry& p_parent, const NodeEntry& p_child)
    {
        if (this->node_tree.add_child_silent(p_parent, p_child))
        {
            this->mark_node_for_recalculation_recursive(p_child);
        }
    };

    inline void remove_node(const NodeEntry& p_node)
    {
        this->free_node_recurvise(p_node);
    };

    inline v3f& get_localposition(const NodeEntry& p_node)
    {
        return p_node.Element->local_transform.position;
    };
    inline quat& get_localrotation(const NodeEntry& p_node)
    {
        return p_node.Element->local_transform.rotation;
    };
    inline v3f& get_localscale(const NodeEntry& p_node)
    {
        return p_node.Element->local_transform.scale;
    };

    inline void set_localposition(const NodeEntry& p_node, const v3f& p_local_position)
    {
        if (p_node.Element->local_transform.position != p_local_position)
        {
            this->mark_node_for_recalculation_recursive(p_node);
            p_node.Element->local_transform.position = p_local_position;
        }
    };
    inline void set_localrotation(const NodeEntry& p_node, const quat& p_local_rotation)
    {
        if (p_node.Element->local_transform.rotation != p_local_rotation)
        {
            this->mark_node_for_recalculation_recursive(p_node);
            p_node.Element->local_transform.rotation = p_local_rotation;
        }
    };
    inline void set_localscale(const NodeEntry& p_node, const v3f& p_local_scale)
    {
        if (p_node.Element->local_transform.scale != p_local_scale)
        {
            this->mark_node_for_recalculation_recursive(p_node);
            p_node.Element->local_transform.scale = p_local_scale;
        }
    };
    inline void set_worldposition(const NodeEntry& p_node, const v3f& p_world_position)
    {
        if (!p_node.has_parent())
        {
            this->set_localposition(p_node, p_world_position);
        }
        else
        {
            NodeEntry l_parent = this->get_node(p_node.Node->parent);
            this->set_localposition(p_node, (this->get_worldtolocal(l_parent) * v4f::build_v3f_s(p_world_position, 1.0f)).Vec3);
        }
    };
    inline void set_worldrotation(const NodeEntry& p_node, const quat& p_world_rotation)
    {
        if (!p_node.has_parent())
        {
            this->set_localrotation(p_node, p_world_rotation);
        }
        else
        {
            NodeEntry l_parent = this->get_node(p_node.Node->parent);
            this->set_localrotation(p_node, this->get_worldrotation(l_parent).inv() * p_world_rotation);
        }
    };
    inline void add_worldrotation(const NodeEntry& p_node, const quat& p_delta_world_rotation)
    {
        this->set_worldrotation(p_node, this->get_worldrotation(p_node) * p_delta_world_rotation);
    };
    inline void set_worldscale(const NodeEntry& p_node, const v3f& p_world_scale)
    {
        if (!p_node.has_parent())
        {
            set_localscale(p_node, p_world_scale);
        }
        else
        {
            NodeEntry l_parent = this->get_node(p_node.Node->parent);
            set_localscale(p_node, p_world_scale * this->get_worldscalefactor(l_parent).inv());
        }
    };

    inline v3f get_worldposition(const NodeEntry& p_node)
    {
        return this->get_localtoworld(p_node).get_translation();
    };
    inline quat get_worldrotation(const NodeEntry& p_node)
    {
        if (!p_node.has_parent())
        {
            return p_node.Element->local_transform.rotation;
        }
        else
        {
            NodeEntry l_parent = this->get_node(p_node.Node->parent);
            return this->get_worldrotation(l_parent) * p_node.Element->local_transform.rotation;
        }
    };
    inline v3f get_worldscalefactor(const NodeEntry& p_node)
    {
        if (!p_node.has_parent())
        {
            return p_node.Element->local_transform.scale;
        }
        else
        {
            NodeEntry l_parent = this->get_node(p_node.Node->parent);
            return this->get_worldscalefactor(l_parent) * p_node.Element->local_transform.scale;
        }
    };
    inline m44f& get_localtoworld(const NodeEntry& p_node)
    {
        this->updatematrices_if_necessary(p_node);
        return p_node.Element->localtoworld;
    };
    inline m44f get_worldtolocal(const NodeEntry& p_node)
    {
        return this->get_localtoworld(p_node).inv();
    };

    inline void clear_nodes_state()
    {
        // TODO -> can't we iterate on the pool tree pool vector instead of traversing ?
        this->node_tree.traverse3(token_build<NTree<Node>::Node>(0), [](const NodeEntry& p_node) {
            p_node.Element->state.haschanged_thisframe = false;
        });
    };

  private:
    inline Node_Token allocate_node(const transform& p_initial_local_transform, const Node_Token p_parent)
    {
        return this->node_tree.push_value(Node::build(Node::State::build(1, 1), p_initial_local_transform), token_build_from<NTree<Node>::Node>(p_parent));
    };
    inline Node_Token allocate_root_node()
    {
        return this->node_tree.push_root_value(Node::build(Node::State::build(1, 1), transform_const::ORIGIN));
    };
    inline void mark_node_for_recalculation_recursive(const NodeEntry& p_node)
    {
        this->node_tree.traverse3(token_build_from<NTree<Node>::Node>(p_node.Node->index), [](const NodeEntry& p_node) {
            p_node.Element->mark_for_recaluclation();
        });
    };
    inline void updatematrices_if_necessary(const NodeEntry& p_node)
    {
        if (p_node.Element->state.matrices_mustBe_recalculated)
        {
            p_node.Element->localtoworld = m44f::trs(p_node.Element->local_transform.position, p_node.Element->local_transform.rotation.to_axis(), p_node.Element->local_transform.scale);

            if (p_node.has_parent())
            {
                NodeEntry l_parent = this->get_node(p_node.Node->parent);
                p_node.Element->localtoworld = this->get_localtoworld(l_parent) * p_node.Element->localtoworld;
            }
            p_node.Element->state.matrices_mustBe_recalculated = false;
        }
    };

    inline void free_node_recurvise(const NodeEntry& p_node)
    {
        Vector<NodeEntry> l_deleted_nodes = Vector<NodeEntry>::allocate(0);
        this->node_tree.get_nodes(p_node.Node->index, &l_deleted_nodes);

        Slice<NodeEntry> l_deleted_nodes_slice = l_deleted_nodes.to_slice();
        this->node_tree.remove_nodes_and_detach(l_deleted_nodes_slice);
        l_deleted_nodes.free();
    };
};

namespace Scene_const
{
const Node_Token root_node = token_build<Node_TokenValue>(0);
};

struct SceneEvents
{
    Vector<Node_Token> orphan_nodes_to_be_destroyed;
    Vector<Node_Token> nodes_to_be_destroyed; // This event is used to break links between node and components

    /*
        When we add or remove components on a Scene, component events are generated.
        These events can be consumed by consumer.
        However, when the step method is called, Component events are automatically discarded.
    */
    struct ComponentRemovedEvent
    {
        Node_Token node;
        // The component has already been detached. So we copy it's value;
        NodeComponent value;

        inline static ComponentRemovedEvent build(const Node_Token p_node, const NodeComponent& p_component_value)
        {
            return ComponentRemovedEvent{p_node, p_component_value};
        };
    };
    Vector<ComponentRemovedEvent> component_removed_events;

    inline static SceneEvents allocate_default()
    {
        return SceneEvents{Vector<Node_Token>::allocate(0), Vector<Node_Token>::allocate(0), Vector<ComponentRemovedEvent>::allocate(0)};
    };

    inline void free()
    {
#if __DEBUG
        assert_true(this->orphan_nodes_to_be_destroyed.empty());
        assert_true(this->component_removed_events.empty());
        assert_true(this->nodes_to_be_destroyed.empty());
#endif

        this->orphan_nodes_to_be_destroyed.free();
        this->component_removed_events.free();
        this->nodes_to_be_destroyed.free();
    };
};

/*
    A Scene is a functional object that enhance the SceneTree by allocating Components to every Nodes.
    Components are typed tokens that points to an external allocated resource.
    - When a Component is added, we suppose that external resource token has already been allocated.
    - When a Component is released, then it's value is pushed to the component_removed_events stack. This stack is consumed by caller and release the external allocated resource.
*/
struct Scene
{
    SceneTree tree;

    using t_NodeToComponents = PoolOfVector<NodeComponent>;
    using NodeToComponents_Token = t_NodeToComponents::sToken;
    using NodeToComponents_TokenValue = t_NodeToComponents::sTokenValue;

    t_NodeToComponents node_to_components;

    SceneEvents scene_events;

    inline static Scene allocate_default()
    {
        Scene l_scene = Scene{SceneTree::allocate_default(), PoolOfVector<NodeComponent>::allocate_default(), SceneEvents::allocate_default()};
        l_scene.node_to_components.alloc_vector();
        return l_scene;
    };

    inline void free()
    {
        this->step_destroy_resource_only();

        this->tree.free();
        this->scene_events.free();
        this->node_to_components.free();
    };

    template <class ComponentRemovedCallbackFunc> inline void consume_component_events()
    {
        for (loop(i, 0, this->scene_events.component_removed_events.Size))
        {
            SceneEvents::ComponentRemovedEvent& l_component_event = this->scene_events.component_removed_events.get(i);
            ComponentRemovedCallbackFunc::on_component_removed(this, this->get_node(l_component_event.node), l_component_event.value);
        };
        this->scene_events.component_removed_events.clear();
    };
    template <class ComponentRemovedCallbackObj> inline void consume_component_events_stateful(const ComponentRemovedCallbackObj& p_closure)
    {
        for (loop(i, 0, this->scene_events.component_removed_events.Size))
        {
            SceneEvents::ComponentRemovedEvent& l_component_event = this->scene_events.component_removed_events.get(i);
            p_closure.on_component_removed(this, this->get_node(l_component_event.node), l_component_event.value);
        };
        this->scene_events.component_removed_events.clear();
    };

    template <class ComponentRemovedCallbackFunc> inline void free_and_consume_component_events()
    {
        this->consume_component_events<ComponentRemovedCallbackFunc>();
        this->free();
    };
    template <class ComponentRemovedCallbackObj> inline void free_and_consume_component_events_stateful(const ComponentRemovedCallbackObj& p_closure)
    {
        this->consume_component_events_stateful(p_closure);
        this->free();
    };

    inline void step()
    {
        this->step_destroy_resource_only();
        this->tree.clear_nodes_state();
    };

    inline Node_Token add_node(const transform& p_initial_local_transform, const Node_Token p_parent)
    {
        Node_Token l_node = this->tree.add_node(p_initial_local_transform, p_parent);
        this->node_to_components.alloc_vector();
        return l_node;
    };

    inline NodeEntry get_node(const Node_Token p_node)
    {
        return this->tree.get_node(p_node);
    };
    inline NodeEntry get_node_parent(const NodeEntry& p_node)
    {
        return this->tree.get_node_parent(p_node);
    };
    inline Slice<Node_Token> get_node_childs(const NodeEntry& p_node)
    {
        return this->tree.get_node_childs(p_node);
    };
    inline void remove_node(const NodeEntry& p_node)
    {
        // get all components -> for pushing events

        NodeEntry l_node_copy = p_node;
        this->tree.node_tree.make_node_orphan(l_node_copy);
        this->scene_events.orphan_nodes_to_be_destroyed.push_back_element(p_node.Node->index);

        this->tree.node_tree.traverse3(p_node.Node->index, [this](const NodeEntry& p_tree_node) {
            Slice<NodeComponent> l_node_component_tokens = this->node_to_components.get_vector(token_build_from<NodeToComponents_TokenValue>(p_tree_node.Node->index));
            for (loop(i, 0, l_node_component_tokens.Size))
            {
                this->scene_events.component_removed_events.push_back_element(
                    SceneEvents::ComponentRemovedEvent::build(p_tree_node.Node->index, l_node_component_tokens.get(i)));
            }
            this->scene_events.nodes_to_be_destroyed.push_back_element(p_tree_node.Node->index);
        });
    };

    inline void add_node_component_by_value(const Node_Token p_node, const NodeComponent& p_component)
    {
        this->node_to_components.element_push_back_element(token_build_from<NodeToComponents_TokenValue>(p_node), p_component);
    };

    template <class ComponentType> inline void add_node_component_typed(const Node_Token p_node, const token_t p_component_resource)
    {
        this->add_node_component_by_value(p_node, NodeComponent{ComponentType::Type, p_component_resource});
    };

    inline NodeComponent* get_node_component_by_type(const Node_Token p_node, const component_t p_type)
    {
        Slice<NodeComponent> l_components = this->node_to_components.get_vector(token_build_from<NodeToComponents_TokenValue>(p_node));
        for (loop(i, 0, l_components.Size))
        {
            NodeComponent& l_component = l_components.get(i);
            if (l_component.type == p_type)
            {
                return &l_component;
            }
        }

        return NULL;
    };

    template <class ComponentType> inline NodeComponent* get_node_component_typed(const Node_Token p_node)
    {
        return this->get_node_component_by_type(p_node, ComponentType::Type);
    };

    inline Slice<NodeComponent> get_node_components(const Node_Token p_node)
    {
        return this->node_to_components.get_vector(token_build_from<NodeToComponents_TokenValue>(p_node));
    };

    inline void remove_node_component(const Node_Token p_node, const component_t p_component_type)
    {
        NodeComponent l_detached_component;
        if (!this->remove_node_component_by_type(p_node, p_component_type, &l_detached_component))
        {
            abort();
        };
    };

    template <class ComponentType> inline void remove_node_component_typed(const Node_Token p_node)
    {
        this->remove_node_component(p_node, ComponentType::Type);
    };

  private:
    inline int8 remove_node_component_by_type(const Node_Token p_node, const component_t p_type, NodeComponent* out_component)
    {
        Slice<NodeComponent> l_components = this->node_to_components.get_vector(token_build_from<NodeToComponents_TokenValue>(p_node));
        for (loop(i, 0, l_components.Size))
        {
            NodeComponent& l_component = l_components.get(i);
            if (l_component.type == p_type)
            {
                *out_component = l_component;
                this->node_to_components.element_erase_element_at_always(token_build_from<NodeToComponents_TokenValue>(p_node), i);
                this->scene_events.component_removed_events.push_back_element(SceneEvents::ComponentRemovedEvent::build(p_node, *out_component));
                return 1;
            }
        }

        *out_component = NodeComponent::build_default();
        return 0;
    };

    inline void step_destroy_resource_only()
    {
        this->destroy_orphan_nodes();
        this->destroy_node_to_component_links();
        this->destroy_component_removed_events();
    };
    inline void destroy_orphan_nodes()
    {
        for (loop(i, 0, this->scene_events.orphan_nodes_to_be_destroyed.Size))
        {
            Node_Token l_node_to_destroy = this->scene_events.orphan_nodes_to_be_destroyed.get(i);
            this->tree.remove_node(this->tree.get_node(l_node_to_destroy));
        }
        this->scene_events.orphan_nodes_to_be_destroyed.clear();
    };

    inline void destroy_node_to_component_links()
    {
        for (loop(i, 0, this->scene_events.nodes_to_be_destroyed.Size))
        {
            this->node_to_components.release_vector(token_build_from<NodeToComponents_TokenValue>(this->scene_events.nodes_to_be_destroyed.get(i)));
        }
        this->scene_events.nodes_to_be_destroyed.clear();
    };

    inline void destroy_component_removed_events()
    {
        this->scene_events.component_removed_events.clear();
    };
};

#include "./scene2_serialization.hpp"