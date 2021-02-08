#pragma once

#include "./scene2_header.hpp"

namespace v2
{
	inline Node::State Node::State::build(const int8 p_matrices_must_be_recalculated, const int8 p_haschanged_thisframe)
	{
		return State{ p_matrices_must_be_recalculated, p_haschanged_thisframe };
	};

	inline Node Node::build_default()
	{
		Node l_node;
		l_node.state = State{ true, false };
		return l_node;
	};

	inline Node Node::build(const State& p_state, const transform& p_local_transform)
	{
		return Node{
			p_state,
			p_local_transform
		};
	};

	inline void Node::mark_for_recaluclation()
	{
		this->state = State{ true, true };
	};

	inline NodeComponent NodeComponent::build_default()
	{
		return NodeComponent{ (component_t)-1, (token_t)-1 };
	};

	inline NodeComponent NodeComponent::build(const component_t p_type, const token_t p_resource)
	{
		return NodeComponent{ p_type, p_resource };
	};

	inline SceneTree SceneTree::allocate_default()
	{
		SceneTree l_scene = SceneTree{
			NTree<Node>::allocate_default()
		};

		l_scene.allocate_root_node();

		return l_scene;
	};

	inline void SceneTree::free()
	{
		this->node_tree.free();
	};

	inline Token(Node) SceneTree::add_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		return this->allocate_node(p_initial_local_transform, p_parent);
	};

	inline NodeEntry SceneTree::get_node(const Token(Node) p_node)
	{
		return this->node_tree.get(p_node);
	};

	inline NodeEntry SceneTree::get_node_parent(const NodeEntry& p_node)
	{
		return this->get_node(tk_bf(Node, p_node.Node->parent));
	};

	inline Slice<Token(Node)> SceneTree::get_node_childs(const NodeEntry& p_node)
	{
		Slice<Token(NTreeNode)> l_childs = this->node_tree.get_childs(p_node.Node->childs);
		return sliceoftoken_cast(Node, l_childs);
	};

	inline void SceneTree::add_child(const NodeEntry& p_parent, const NodeEntry& p_child)
	{
		if (this->node_tree.add_child(p_parent, p_child))
		{
			this->mark_node_for_recalculation_recursive(p_child);
		}
	};

	inline void SceneTree::remove_node(const NodeEntry& p_node)
	{
		this->free_node_recurvise(p_node);
	};

	inline v3f& SceneTree::get_localposition(const NodeEntry& p_node)
	{
		return p_node.Element->local_transform.position;
	};

	inline quat& SceneTree::get_localrotation(const NodeEntry& p_node)
	{
		return p_node.Element->local_transform.rotation;
	};

	inline v3f& SceneTree::get_localscale(const NodeEntry& p_node)
	{
		return p_node.Element->local_transform.scale;
	};

	inline void SceneTree::set_localposition(const NodeEntry& p_node, const v3f& p_local_position)
	{
		if (p_node.Element->local_transform.position != p_local_position)
		{
			this->mark_node_for_recalculation_recursive(p_node);
			p_node.Element->local_transform.position = p_local_position;
		}
	};

	inline void SceneTree::set_localrotation(const NodeEntry& p_node, const quat& p_local_rotation)
	{
		if (p_node.Element->local_transform.rotation != p_local_rotation)
		{
			this->mark_node_for_recalculation_recursive(p_node);
			p_node.Element->local_transform.rotation = p_local_rotation;
		}
	};

	inline void SceneTree::set_localscale(const NodeEntry& p_node, const v3f& p_local_scale)
	{
		if (p_node.Element->local_transform.scale != p_local_scale)
		{
			this->mark_node_for_recalculation_recursive(p_node);
			p_node.Element->local_transform.scale = p_local_scale;
		}
	};

	inline void SceneTree::set_worldposition(const NodeEntry& p_node, const v3f& p_world_position)
	{
		if (!p_node.has_parent())
		{
			this->set_localposition(p_node, p_world_position);
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			this->set_localposition(p_node, (this->get_worldtolocal(l_parent) * v4f::build_v3f_s(p_world_position, 1.0f)).Vec3);
		}
	};

	inline void SceneTree::set_worldrotation(const NodeEntry& p_node, const quat& p_world_rotation)
	{
		if (!p_node.has_parent())
		{
			this->set_localrotation(p_node, p_world_rotation);
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			this->set_localrotation(p_node, this->get_worldrotation(l_parent).inv() * p_world_rotation);
		}
	};

	inline void SceneTree::set_worldscale(const NodeEntry& p_node, const v3f& p_world_scale)
	{
		if (!p_node.has_parent())
		{
			set_localscale(p_node, p_world_scale);
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			set_localscale(p_node, p_world_scale * this->get_worldscalefactor(l_parent).inv());
		}
	};

	inline v3f SceneTree::get_worldposition(const NodeEntry& p_node)
	{
		return this->get_localtoworld(p_node).get_translation();
	};

	inline quat SceneTree::get_worldrotation(const NodeEntry& p_node)
	{
		if (!p_node.has_parent())
		{
			return p_node.Element->local_transform.rotation;
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			return this->get_worldrotation(l_parent) * p_node.Element->local_transform.rotation;
		}
	};

	inline v3f SceneTree::get_worldscalefactor(const NodeEntry& p_node)
	{
		if (!p_node.has_parent())
		{
			return p_node.Element->local_transform.scale;
		}
		else
		{
			NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
			return this->get_worldscalefactor(l_parent) * p_node.Element->local_transform.scale;
		}
	};

	inline m44f& SceneTree::get_localtoworld(const NodeEntry& p_node)
	{
		this->updatematrices_if_necessary(p_node);
		return p_node.Element->localtoworld;
	};

	inline m44f SceneTree::get_worldtolocal(const NodeEntry& p_node)
	{
		return this->get_localtoworld(p_node).inv();
	};

	inline void SceneTree::clear_nodes_state()
	{
		this->node_tree.traverse3(tk_b(NTreeNode, 0),
			[](const NodeEntry& p_node) { p_node.Element->state.haschanged_thisframe = false;
			}
		);
	};

	inline Token(Node) SceneTree::allocate_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		Token(Node) l_node = this->node_tree.push_value(
			Node::build(Node::State::build(1, 1), p_initial_local_transform),
			p_parent
		);
		return l_node;
	};

	inline Token(Node) SceneTree::allocate_root_node()
	{
		Token(Node) l_node = this->node_tree.push_root_value(
			Node::build(Node::State::build(1, 1), transform_const::ORIGIN)
		);
		return l_node;
	};

	inline void SceneTree::mark_node_for_recalculation_recursive(const NodeEntry& p_node)
	{
		this->node_tree.traverse3(tk_bf(NTreeNode, p_node.Node->index),
			[](const NodeEntry& p_node) {
				p_node.Element->mark_for_recaluclation();
			}
		);
	};

	inline void SceneTree::updatematrices_if_necessary(const NodeEntry& p_node)
	{
		if (p_node.Element->state.matrices_mustBe_recalculated)
		{
			p_node.Element->localtoworld = m44f::trs(p_node.Element->local_transform.position, p_node.Element->local_transform.rotation.to_axis(), p_node.Element->local_transform.scale);

			if (p_node.has_parent())
			{
				NodeEntry l_parent = this->get_node(tk_bf(Node, p_node.Node->parent));
				p_node.Element->localtoworld = this->get_localtoworld(l_parent) * p_node.Element->localtoworld;
			}
			p_node.Element->state.matrices_mustBe_recalculated = false;
		}
	};

	inline void SceneTree::free_node_recurvise(const NodeEntry& p_node)
	{
		Vector<NodeEntry> l_deleted_nodes = Vector<NodeEntry>::allocate(0);
		this->node_tree.get_nodes(p_node.Node->index, &l_deleted_nodes);

		Slice<NodeEntry> l_deleted_nodes_slice = l_deleted_nodes.to_slice();
		this->node_tree.remove_nodes_and_detach(l_deleted_nodes_slice);
		l_deleted_nodes.free();
	};

	inline Scene::ComponentRemovedEvent Scene::ComponentRemovedEvent::build(const Token(Node) p_node, const NodeComponent& p_component_value)
	{
		return ComponentRemovedEvent{ p_node, p_component_value };
	};



	inline Scene Scene::allocate_default()
	{
		Scene l_scene = Scene{
			SceneTree::allocate_default(),
			PoolOfVector<NodeComponent>::allocate_default(),
			Vector<Token(Node)>::allocate(0),
			Vector<Token(Node)>::allocate(0),
			Vector<ComponentRemovedEvent>::allocate(0)
		};
		l_scene.node_to_components.alloc_vector();
		return l_scene;
	};

	inline void Scene::free()
	{
		this->step_destroy_resource_only();

		this->tree.free();
		this->orphan_nodes.free();
		this->node_that_will_be_destroyed.free();
		this->component_removed_events.free();
		this->node_to_components.free();
	};

	template<class ComponentRemovedCallbackFunc>
	inline void Scene::free_and_consume_component_events()
	{
		this->consume_component_events<ComponentRemovedCallbackFunc>();
		this->free();
	};

	template<class ComponentRemovedCallbackObj>
	inline void Scene::free_and_consume_component_events_stateful(ComponentRemovedCallbackObj& p_closure)
	{
		this->consume_component_events_stateful(p_closure);
		this->free();
	};

	template<class ComponentRemovedCallbackFunc>
	inline void Scene::consume_component_events()
	{
		for (vector_loop(&this->component_removed_events, i))
		{
			ComponentRemovedEvent& l_component_event = this->component_removed_events.get(i);
			ComponentRemovedCallbackFunc::on_component_removed(this, this->get_node(l_component_event.node), l_component_event.value);
		};
		this->component_removed_events.clear();
	};

	template<class ComponentRemovedCallbackObj>
	inline void Scene::consume_component_events_stateful(ComponentRemovedCallbackObj& p_closure)
	{
		for (vector_loop(&this->component_removed_events, i))
		{
			ComponentRemovedEvent& l_component_event = this->component_removed_events.get(i);
			p_closure.on_component_removed(this, this->get_node(l_component_event.node), l_component_event.value);
		};
		this->component_removed_events.clear();
	};

	inline void Scene::step()
	{
		this->destroy_orphan_nodes();
		this->destroy_component_removed_events();
		this->tree.clear_nodes_state();
	};


	inline Token(Node) Scene::add_node(const transform& p_initial_local_transform, const Token(Node) p_parent)
	{
		Token(Node) l_node = this->tree.add_node(p_initial_local_transform, p_parent);
		this->node_to_components.alloc_vector();
		return l_node;
	};

	inline NodeEntry Scene::get_node(const Token(Node) p_node)
	{
		return this->tree.get_node(p_node);
	};

	inline NodeEntry Scene::get_node_parent(const NodeEntry& p_node)
	{
		return this->tree.get_node_parent(p_node);
	};

	inline Slice<Token(Node)> Scene::get_node_childs(const NodeEntry& p_node)
	{
		return this->tree.get_node_childs(p_node);
	};

	inline void Scene::remove_node(const NodeEntry& p_node)
	{
		// get all components -> for pushing events

		NodeEntry l_node_copy = p_node;
		this->tree.node_tree.make_node_orphan(l_node_copy);
		this->orphan_nodes.push_back_element(tk_bf(Node, p_node.Node->index));

		this->tree.node_tree.traverse3(tk_bf(NTreeNode, p_node.Node->index),
			[this](const NodeEntry& p_tree_node) {
				this->node_that_will_be_destroyed.push_back_element(tk_bf(Node, p_tree_node.Node->index));

				Slice<NodeComponent> l_node_component_tokens = this->node_to_components.get_vector(tk_bf(Slice<NodeComponent>, p_tree_node.Node->index));
				for (loop(i, 0, l_node_component_tokens.Size))
				{
					this->component_removed_events.push_back_element(ComponentRemovedEvent::build(tk_bf(Node, p_tree_node.Node->index), l_node_component_tokens.get(i)));
				}
				this->node_to_components.release_vector(tk_bf(Slice<NodeComponent>, p_tree_node.Node->index));
			}
		);
	};

	inline void Scene::add_node_component_by_value(const Token(Node) p_node, const NodeComponent& p_component)
	{
		this->node_to_components.element_push_back_element(tk_bf(Slice<NodeComponent>, p_node), p_component);
	};

	template<class ComponentType>
	inline void Scene::add_node_component_typed(const Token(Node) p_node, const token_t p_component_ressource)
	{
		this->add_node_component_by_value(p_node, NodeComponent{ ComponentType::Type , p_component_ressource });
	};

	inline NodeComponent* Scene::get_node_component_by_type(const Token(Node) p_node, const component_t p_type)
	{
		Slice<NodeComponent> l_components = this->node_to_components.get_vector(tk_bf(Slice<NodeComponent>, p_node));
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


	template<class ComponentType>
	inline NodeComponent* Scene::get_node_component_typed(const Token(Node) p_node)
	{
		return this->get_node_component_by_type(p_node, ComponentType::Type);
	};

	inline Slice<NodeComponent> Scene::get_node_components(const Token(Node) p_node)
	{
		return this->node_to_components.get_vector(tk_bf(Slice<NodeComponent>, p_node));
	};

	inline void Scene::remove_node_component(const Token(Node) p_node, const component_t p_component_type)
	{
		NodeComponent l_detached_component;
		if (!this->remove_node_component_by_type(p_node, p_component_type, &l_detached_component))
		{
			abort();
		};
	};

	template<class ComponentType>
	inline void Scene::remove_node_component_typed(const Token(Node) p_node)
	{
		this->remove_node_component(p_node, ComponentType::Type);
	};

	inline int8 Scene::remove_node_component_by_type(const Token(Node) p_node, const component_t p_type, NodeComponent* out_component)
	{
		Slice<NodeComponent> l_components = this->node_to_components.get_vector(tk_bf(Slice<NodeComponent>, p_node));
		for (loop(i, 0, l_components.Size))
		{
			NodeComponent& l_component = l_components.get(i);
			if (l_component.type == p_type)
			{
				*out_component = l_component;
				this->node_to_components.element_erase_element_at_always(tk_bf(Slice<NodeComponent>, p_node), i);
				this->component_removed_events.push_back_element(ComponentRemovedEvent::build(p_node, *out_component));
				return 1;
			}
		}

		*out_component = NodeComponent::build_default();
		return 0;
	};


	inline void Scene::step_destroy_resource_only()
	{
		this->destroy_orphan_nodes();
		this->destroy_component_removed_events();
	};

	inline void Scene::destroy_orphan_nodes()
	{
		for (vector_loop(&this->orphan_nodes, i))
		{
			this->tree.remove_node(this->tree.get_node(this->orphan_nodes.get(i)));
		}
		this->orphan_nodes.clear();
		this->node_that_will_be_destroyed.clear();
	};

	inline void Scene::destroy_component_removed_events()
	{
		/*
		for (vector_loop(&this->component_events, i))
		{
			ComponentEvent& l_component_event = this->component_events.get(i);
			if (l_component_event.state == ComponentEvent::State::REMOVED)
			{
				this->tree.free_component(l_component_event.component);
			};
		}
		*/
		this->component_removed_events.clear();
	};
}


#include "./scene2_serialization.hpp"
