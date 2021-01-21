#pragma once

#include "Math2/math.hpp"

namespace v2
{

	using ComponentType = uimax;

	struct Node
	{
		struct State
		{
			int8 matrices_mustBe_recalculated; /* = true;*/
			int8 haschanged_thisframe; /* = false; */

			static State build(const int8 p_matrices_must_be_recalculated, const int8 p_haschanged_thisframe);

		} state;

		//transform
		transform local_transform;

		/** This matrix will always be relative to the root Node (a Node without parent). */
		m44f localtoworld;

		static Node build_default();
		static Node build(const State& p_state, const transform& p_local_transform);

		void mark_for_recaluclation();
	};

	using NodeEntry = NTree<Node>::Resolve;

	struct NodeComponent
	{
		ComponentType type;
		token_t resource;

		static NodeComponent build_default();
		static NodeComponent build(const ComponentType p_type, const token_t p_resource);
	};

	struct SceneTree
	{
		NTree<Node> node_tree;
		
		static SceneTree allocate_default();
		void free();

		Token(Node) add_node(const transform& p_initial_local_transform, const Token(Node) p_parent);
		NodeEntry get_node(const Token(Node) p_node);
		NodeEntry get_node_parent(const NodeEntry& p_node);

		Slice<Token(Node)> get_node_childs(const NodeEntry& p_node);
		void get_node_and_all_its_childrens(const NodeEntry& p_node, Vector<NodeEntry>* out_childs);

		void add_child(const NodeEntry& p_parent, const  NodeEntry& p_child);

		void remove_node(const NodeEntry& p_node);

		v3f& get_localposition(const NodeEntry& p_node);
		quat& get_localrotation(const NodeEntry& p_node);
		v3f& get_localscale(const NodeEntry& p_node);

		void set_localposition(const NodeEntry& p_node, const v3f& p_local_position);
		void set_localrotation(const NodeEntry& p_node, const quat& p_local_rotation);
		void set_localscale(const NodeEntry& p_node, const v3f& p_local_scale);
		void set_worldposition(const NodeEntry& p_node, const v3f& p_world_position);
		void set_worldrotation(const NodeEntry& p_node, const quat& p_world_rotation);
		void set_worldscale(const NodeEntry& p_node, const v3f& p_world_scale);

		v3f get_worldposition(const NodeEntry& p_node);
		quat get_worldrotation(const NodeEntry& p_node);
		v3f get_worldscalefactor(const NodeEntry& p_node);
		m44f& get_localtoworld(const NodeEntry& p_node);
		m44f get_worldtolocal(const NodeEntry& p_node);

		void clear_nodes_state();

	private:
		Token(Node) allocate_node(const transform& p_initial_local_transform, const Token(Node) p_parent);
		Token(Node) allocate_root_node();
		void mark_node_for_recalculation_recursive(const NodeEntry& p_node);
		void updatematrices_if_necessary(const NodeEntry& p_node);

		void free_node_recurvise(const NodeEntry& p_node);
	};

	namespace Scene_const
	{
		const Token(Node) root_node = tk_b(Node, 0);
	};


	struct Scene
	{
		/*
			When we add or remove components on a Scene, component events are generated.
			These events can be consumed by consumer.
			However, when the step method is called, Component events are automatically discarded;
		*/
		struct ComponentRemovedEvent
		{
			Token(Node) node;
			// The component has already been detached. So we copy it's value;
			NodeComponent value;

			static ComponentRemovedEvent build(const Token(Node) p_node, const NodeComponent& p_component_value);
		};

		SceneTree tree;
		PoolOfVector<NodeComponent> node_to_components;

		Vector<Token(Node)> orphan_nodes;
		Vector<Token(Node)> node_that_will_be_destroyed;
		Vector<ComponentRemovedEvent> component_removed_events;

		static Scene allocate_default();

		void free();

		template<class ComponentRemovedCallbackFunc>
		void consume_component_events();

		template<class ComponentRemovedCallbackObj>
		void consume_component_events_stateful(ComponentRemovedCallbackObj& p_closure);

		void step();


		Token(Node) add_node(const transform& p_initial_local_transform, const Token(Node) p_parent);
		NodeEntry get_node(const Token(Node) p_node);
		NodeEntry get_node_parent(const NodeEntry& p_node);
		Slice<Token(Node)> get_node_childs(const NodeEntry& p_node);
		void remove_node(const NodeEntry& p_node);


		void add_node_component_by_value(const Token(Node) p_node, const NodeComponent& p_component);

		template<class ComponentType>
		void add_node_component_typed(const Token(Node) p_node, const token_t p_component_ressource);
		
		NodeComponent* get_node_component_by_type(const Token(Node) p_node, const ComponentType p_type);
		template<class ComponentType>
		NodeComponent* get_node_component_typed(const Token(Node) p_node);

		void remove_node_component(const Token(Node) p_node, const ComponentType p_component_type);
		template<class ComponentType>
		void remove_node_component_typed(const Token(Node) p_node);

		// void merge_scenetree(const SceneTree& p_scene_tree, const Token(Node) p_parent);

	private:
		int8 remove_node_component_by_type(const Token(Node) p_node, const ComponentType p_type, NodeComponent* out_component);

		void step_destroy_resource_only();
		void destroy_orphan_nodes();

		void destroy_component_removed_events();
	};

}