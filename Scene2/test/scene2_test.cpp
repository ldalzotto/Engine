
#include "Scene2/scene2.hpp"

namespace v2
{

	struct ComponentTest
	{
		static const ComponentType Type;

		int i0, i1, i2;
	};

	constexpr ComponentType ComponentTest::Type = 1;

	struct ComponentTest2
	{
		static const ComponentType Type;

		uimax i0, i1, i2;
	};
	constexpr ComponentType ComponentTest2::Type = 2;

	// There is no component removal test here
	inline void add_remove_setparent_node()
	{
		Scene l_scene = Scene::allocate_default();

		transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };

		Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
		{
			assert_true(tk_v(l_node_1) == 1);

			NodeEntry l_node_1_value = l_scene.get_node(l_node_1);
			assert_true(l_node_1_value.has_parent() == 1);
			assert_true(l_node_1_value.Element->local_transform == l_node_1_transform);
			assert_true(l_node_1_value.Element->state.haschanged_thisframe == 1);
			assert_true(l_node_1_value.Element->state.matrices_mustBe_recalculated == 1);
		}

		Token(Node) l_node_2 = l_scene.add_node(l_node_1_transform, l_node_1);
		Token(Node) l_node_3 = l_scene.add_node(l_node_1_transform, l_node_1);

		{
			NodeEntry l_node_2_value = l_scene.get_node(l_node_2);
			NodeEntry l_node_3_value = l_scene.get_node(l_node_3);
			assert_true(tk_eq(l_scene.get_node_parent(l_node_2_value).Node->index, l_node_1));
			assert_true(tk_eq(l_scene.get_node_parent(l_node_3_value).Node->index, l_node_1));
			assert_true(l_scene.get_node_childs(l_scene.get_node(l_node_1)).Size == 2);
		}


		l_scene.add_node(l_node_1_transform, Scene_const::root_node);
		Token(Node) l_node_5 = l_scene.add_node(l_node_1_transform, l_node_3);

		// set_parent
		l_scene.step();
		assert_true(l_scene.get_node(l_node_3).Element->state.haschanged_thisframe == 0);
		assert_true(l_scene.get_node(l_node_5).Element->state.haschanged_thisframe == 0);

		// When a node parent has changed, the state of the node is set as if it's position has changed
		{
			l_scene.tree.add_child(l_scene.get_node(l_node_1), l_scene.get_node(l_node_3));
			assert_true(l_scene.get_node(l_node_3).Element->state.haschanged_thisframe == 1);
			assert_true(l_scene.get_node(l_node_5).Element->state.haschanged_thisframe == 1);
			assert_true(tk_eq(l_scene.get_node(l_node_3).Node->parent, l_scene.get_node(l_node_1).Node->index));
		}

		// remove node
		{
			l_scene.remove_node(l_scene.get_node(l_node_1));
			assert_true(l_scene.get_node_childs(l_scene.get_node(Scene_const::root_node)).Size == 1);

			// deleted nodes have been pushed to the deleted node stack
			assert_true(l_scene.node_that_will_be_destroyed.Size == 4);
			l_scene.step();
			assert_true(l_scene.node_that_will_be_destroyed.Size == 0);
		}

		l_scene.free();
	};

	inline void add_remove_component()
	{
		Scene l_scene = Scene::allocate_default();

		// Added components are directly added to the associated node of the SceneTree
		{
			transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };
			Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
			token_t l_component_test_resource = 1;
			l_scene.add_node_component_typed<ComponentTest>(l_node_1, l_component_test_resource);

			NodeComponent* l_component_test = l_scene.get_node_component_typed<ComponentTest>(l_node_1);
			assert_true(l_component_test->type == ComponentTest::Type);
			assert_true(l_component_test->resource == l_component_test_resource);

			token_t l_component_test_2_resource = 2;
			l_scene.add_node_component_typed<ComponentTest2>(l_node_1, l_component_test_2_resource);

			l_scene.step();

			assert_true(l_scene.component_removed_events.Size == 0);

			l_scene.remove_node_component_typed<ComponentTest>(l_node_1); //ensuring that no error
			assert_true(l_scene.get_node_component_typed<ComponentTest2>(l_node_1) != NULL);
			assert_true(l_scene.get_node_component_typed<ComponentTest2>(l_node_1)->resource == l_component_test_2_resource);

			// One component removed event have been generated
			assert_true(l_scene.component_removed_events.Size == 1);
			assert_true(l_scene.component_removed_events.get(0).value.type == ComponentTest::Type);
			assert_true(tk_eq(l_scene.component_removed_events.get(0).node, l_node_1));
		}

		{
		
		}

		l_scene.free();

		l_scene = Scene::allocate_default();

		// deleting a node with components will generate events that can be consumed
		{

			transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };
			Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
			l_scene.add_node_component_typed<ComponentTest>(l_node_1, 0);
			l_scene.add_node_component_typed<ComponentTest2>(l_node_1, 0);

			l_scene.step();

			assert_true(l_scene.component_removed_events.Size == 0);

			l_scene.remove_node(l_scene.get_node(l_node_1));

			assert_true(l_scene.component_removed_events.Size == 2);
			assert_true(l_scene.component_removed_events.get(0).value.type == ComponentTest::Type);
			assert_true(tk_eq(l_scene.component_removed_events.get(0).node, l_node_1));
			assert_true(l_scene.component_removed_events.get(1).value.type == ComponentTest2::Type);
			assert_true(tk_eq(l_scene.component_removed_events.get(1).node, l_node_1));

			l_scene.step();
		}

		l_scene.free();
	};

	inline void component_consume()
	{

		struct component_consume_callbacks
		{
			int8 component_test_1_removed_called;
			int8 component_test_2_removed_called;

			inline static component_consume_callbacks build_default()
			{
				return component_consume_callbacks{ 0,0 };
			};

			inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component)
			{
				if (p_component.type == ComponentTest::Type)
				{
					this->component_test_1_removed_called += 1;
				}
				else if (p_component.type == ComponentTest2::Type)
				{
					this->component_test_2_removed_called += 1;
				}
			};

		};

		Scene l_scene = Scene::allocate_default();

		// Checking that when components are removed and events are consumed, proper on_component_removed callback is called
		{
			transform l_node_1_transform = transform{ v3f_const::FORWARD, quat_const::IDENTITY, v3f_const::ZERO };
			Token(Node) l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
			Token(Node) l_node_2 = l_scene.add_node(l_node_1_transform, l_node_1);

			l_scene.add_node_component_typed<ComponentTest>(l_node_1, 0);
			l_scene.add_node_component_typed<ComponentTest2>(l_node_1, 1);
			l_scene.add_node_component_typed<ComponentTest2>(l_node_2, 1);

			component_consume_callbacks l_callbacks = component_consume_callbacks::build_default();
			assert_true(l_scene.component_removed_events.Size == 0);
			// l_scene.consume_component_events_stateful(l_callbacks);

			// assert_true(l_callbacks.component_test_1_added_called);
			// assert_true(l_callbacks.component_test_2_added_called);

			l_scene.remove_node_component_typed<ComponentTest2>(l_node_1);
			assert_true(l_scene.component_removed_events.Size == 1); // 1 component from l_node_1

			l_scene.consume_component_events_stateful(l_callbacks);

			assert_true(l_callbacks.component_test_2_removed_called == 1);
			assert_true(l_callbacks.component_test_1_removed_called == 0);

			l_scene.remove_node(l_scene.get_node(l_node_1));

			assert_true(l_scene.component_removed_events.Size == 2); // 1 components from l_node_1 and 1 from l_node_2

			l_scene.consume_component_events_stateful(l_callbacks);
			assert_true(l_callbacks.component_test_2_removed_called == 2);
			assert_true(l_callbacks.component_test_1_removed_called == 1);

		}

		l_scene.free();

	};


	inline void math_hierarchy()
	{
		Scene l_scene = Scene::allocate_default();

		Token(Node) l_node_1 = l_scene.add_node(transform_const::ORIGIN, tk_b(Node, 0));
		Token(Node) l_node_2 = l_scene.add_node(transform{ v3f_const::ONE, quat_const::IDENTITY, v3f_const::ONE }, l_node_1);
		Token(Node) l_node_3 = l_scene.add_node(transform{ v3f{-1.0f, -1.0f, -1.0f}, quat_const::IDENTITY, v3f_const::ONE }, l_node_1);

		NodeEntry l_node_1_val = l_scene.get_node(l_node_1);
		NodeEntry l_node_2_val = l_scene.get_node(l_node_2);
		NodeEntry l_node_3_val = l_scene.get_node(l_node_3);

		// Position
		{
			l_scene.tree.set_localposition(l_node_1_val, v3f{ 1.0f, 1.0f, 1.0f });
			assert_true(l_scene.tree.get_localposition(l_node_1_val) == v3f{ 1.00000000f, 1.00000000f, 1.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_2_val) == v3f{ 2.00000000f, 2.00000000f, 2.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_3_val) == v3f{ 0.000000000f, 0.000000000f, 0.000000000f });

			l_scene.tree.set_localposition(l_node_2_val, v3f{ 1.0f, 2.0f, 3.0f });
			assert_true(l_scene.tree.get_localposition(l_node_2_val) == v3f{ 1.00000000f, 2.00000000f, 3.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_2_val) == v3f{ 2.00000000f, 3.00000000f, 4.00000000f });

			l_scene.tree.set_worldposition(l_node_3_val, v3f{ -1.0f, -2.0f, 3.0f });
			assert_true(l_scene.tree.get_localposition(l_node_3_val) == v3f{ -2.00000000f, -3.00000000f, 2.00000000f });
			assert_true(l_scene.tree.get_worldposition(l_node_3_val) == v3f{ -1.00000000f, -2.00000000f, 3.00000000f });
		}

		// Rotations
		{
			l_scene.tree.set_localrotation(l_node_1_val, v3f{ 0.32f, 0.9f, 0.7f }.euler_to_quat());
			assert_true(l_scene.tree.get_localrotation(l_node_1_val) == quat{ -0.0124835223f, 0.452567190f, 0.239721030f, 0.858813643f });
			assert_true(l_scene.tree.get_worldrotation(l_node_2_val) == quat{ -0.0124835242f, 0.452567250f, 0.239721060f, 0.858813763f });
			assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == quat{ -0.0124835242f, 0.452567250f, 0.239721060f, 0.858813763f });

			l_scene.tree.set_localrotation(l_node_2_val, v3f{ 0.32f, -0.9f, -0.7f }.euler_to_quat());
			assert_true(l_scene.tree.get_localrotation(l_node_2_val) == quat{ -0.0124835223f, -0.452567190f, -0.239721030f, 0.858813643f });
			assert_true(l_scene.tree.get_worldrotation(l_node_2_val) == quat{ -0.0214420408f, -0.00598512636f, 0.0112992665f, 0.999688387f });

			l_scene.tree.set_worldrotation(l_node_3_val, v3f{ -1.0f, -2.0f, 3.0f }.euler_to_quat());
			assert_true(l_scene.tree.get_localrotation(l_node_3_val) == quat{ 0.346717477f, -0.641801417f, 0.598375380f, 0.331398427f });
			assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == quat{ 0.718287051f, -0.310622454f, 0.444435090f, 0.435952842f });
		}

		// Scale
		{
			l_scene.tree.set_localscale(l_node_1_val, v3f{ 2.0f, 0.5f, 1.0f });
			assert_true(l_scene.tree.get_localscale(l_node_1_val) == v3f{ 2.00000000f, 0.500000000f, 1.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_2_val) == v3f{ 2.00000000f, 0.500000000f, 1.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_3_val) == v3f{ 2.00000000f, 0.500000000f, 1.00000000f });

			l_scene.tree.set_localscale(l_node_2_val, v3f{ 1.0f, 2.0f, 3.0f });
			assert_true(l_scene.tree.get_localscale(l_node_2_val) == v3f{ 1.00000000f, 2.00000000f, 3.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_2_val) == v3f{ 2.00000000f, 1.00000000f, 3.00000000f });


			l_scene.tree.set_worldscale(l_node_3_val, v3f{ -1.0f, -2.0f, 3.0f });
			assert_true(l_scene.tree.get_localscale(l_node_3_val) == v3f{ -0.500000000f, -4.00000000f, 3.00000000f });
			assert_true(l_scene.tree.get_worldscalefactor(l_node_3_val) == v3f{ -1.00000000f, -2.00000000f, 3.00000000f });
		}


		l_scene.free();
	};

	//TODO
	inline void json_deserialization()
	{
#if 1
		struct TMP
		{
			inline static void push_to_scene(JSONDeserializer& p_component_object, const Slice<int8>& p_type, const Token(Node) p_node, SceneTree* in_out_scene_tree)
			{
				if (p_type.compare(slice_int8_build_rawstr("ComponentTest1")))
				{
					// ComponentTest2 l_c = ComponentTest2{};
					// in_out_scene_tree->add_node_component(p_node, ComponentTest2::Type, (int8*)&l_c);
					// int ads = 10;
				}
				else if (p_type.compare(slice_int8_build_rawstr("ComponentTest1")))
				{

				}
			};
		};

		const char* l_scene_json = "{\"type\":\"scene\",\"nodes\":[{\"local_position\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"local_rotation\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\",\"w\":\"1.0\"},\"local_scale\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"components\":[{\"type\":\"Camera\",\"object\":{\"test_value_1\":\"10\",\"test_value_2\":\"10\"}}],\"childs\":[]},{\"local_position\":{\"x\":\"2.0\",\"y\":\"2.0\",\"z\":\"2.0\"},\"local_rotation\":{\"x\":\"2.0\",\"y\":\"2.0\",\"z\":\"2.0\",\"w\":\"2.0\"},\"local_scale\":{\"x\":\"2.0\",\"y\":\"2.0\",\"z\":\"2.0\"},\"components\":[{\"type\":\"Camera\",\"object\":{\"test_value_1\":\"20\",\"test_value_2\":\"20\"}}],\"childs\":[{\"local_position\":{\"x\":\"3.0\",\"y\":\"3.0\",\"z\":\"3.0\"},\"local_rotation\":{\"x\":\"3.0\",\"y\":\"3.0\",\"z\":\"3.0\",\"w\":\"3.0\"},\"local_scale\":{\"x\":\"3.0\",\"y\":\"3.0\",\"z\":\"3.0\"},\"components\":[{\"type\":\"Camera\",\"object\":{\"test_value_1\":\"30\",\"test_value_2\":\"30\"}}],\"childs\":[{\"local_position\":{\"x\":\"4.0\",\"y\":\"4.0\",\"z\":\"4.0\"},\"local_rotation\":{\"x\":\"4.0\",\"y\":\"4.0\",\"z\":\"4.0\",\"w\":\"4.0\"},\"local_scale\":{\"x\":\"4.0\",\"y\":\"4.0\",\"z\":\"4.0\"},\"components\":[{\"type\":\"ComponentTest\",\"object\":{\"test_value_1\":\"40\",\"test_value_2\":\"40\"}}],\"childs\":[]}]},{\"local_position\":{\"x\":\"5.0\",\"y\":\"5.0\",\"z\":\"5.0\"},\"local_rotation\":{\"x\":\"5.0\",\"y\":\"5.0\",\"z\":\"5.0\",\"w\":\"5.0\"},\"local_scale\":{\"x\":\"5.0\",\"y\":\"5.0\",\"z\":\"5.0\"},\"components\":[{\"type\":\"ComponentTest2\",\"object\":{\"test_value_1\":\"50\",\"test_value_2\":\"50\"}}],\"childs\":[]}]}]}";

		String l_scene_json_string = String::allocate(0);
		l_scene_json_string.append(slice_int8_build_rawstr(l_scene_json));
		JSONDeserializer l_serailizer = JSONDeserializer::start(l_scene_json_string);
		SceneTree l_scene_tree = SceneTree::allocate_default();
		SceneDeserialization::json_to_scenetree<TMP>(l_serailizer, &l_scene_tree);

		int16 l_counter = 0;

		tree_traverse2_stateful_begin(Node, int16 * _counter, ForEach);
		*_counter += 1;
		tree_traverse2_stateful_end(Node, &l_scene_tree.node_tree, tk_b(NTreeNode, 0), &l_counter, ForEach);


		l_scene_tree.free();
		l_scene_json_string.free();

#endif
	};
}

int main()
{
	v2::add_remove_setparent_node();
	v2::add_remove_component();
	v2::component_consume();
	v2::math_hierarchy();
	v2::json_deserialization();
};