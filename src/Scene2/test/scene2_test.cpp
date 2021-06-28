
#include "Scene2/scene2.hpp"

struct ComponentTest
{
    static constexpr component_t Type = HashFunctions::hash_compile<strlen_compile::get_size(STR(ComponentTest))>(STR(ComponentTest));
    int i0, i1, i2;
};

struct ComponentTest2
{
    static constexpr component_t Type = HashFunctions::hash_compile<strlen_compile::get_size(STR(ComponentTest2))>(STR(ComponentTest2));
    uimax i0, i1, i2;
};

struct DefaulSceneComponentReleaser
{
    inline static void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component){};
};

// There is no component removal test here
inline void add_remove_setparent_node()
{
    Scene l_scene = Scene::allocate_default();

    transform l_node_1_transform = transform{v3f_const::FORWARD.vec3, quat_const::IDENTITY, v3f_const::ZERO.vec3};

    Node_Token l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
    {
        assert_true(token_value(l_node_1) == 1);

        NodeEntry l_node_1_value = l_scene.get_node(l_node_1);
        assert_true(l_node_1_value.has_parent() == 1);
        assert_true(l_node_1_value.Element->local_transform == l_node_1_transform);
        assert_true(l_node_1_value.Element->state.haschanged_thisframe == 1);
        assert_true(l_node_1_value.Element->state.matrices_mustBe_recalculated == 1);
    }

    Node_Token l_node_2 = l_scene.add_node(l_node_1_transform, l_node_1);
    Node_Token l_node_3 = l_scene.add_node(l_node_1_transform, l_node_1);

    {
        NodeEntry l_node_2_value = l_scene.get_node(l_node_2);
        NodeEntry l_node_3_value = l_scene.get_node(l_node_3);
        assert_true(token_equals(l_scene.get_node_parent(l_node_2_value).Node->index, l_node_1));
        assert_true(token_equals(l_scene.get_node_parent(l_node_3_value).Node->index, l_node_1));
        assert_true(l_scene.get_node_childs(l_scene.get_node(l_node_1)).Size == 2);
    }

    Node_Token l_node_4 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
    Node_Token l_node_5 = l_scene.add_node(l_node_1_transform, l_node_3);

    // set_parent
    l_scene.step();
    assert_true(l_scene.get_node(l_node_3).Element->state.haschanged_thisframe == 0);
    assert_true(l_scene.get_node(l_node_5).Element->state.haschanged_thisframe == 0);

    // When a node parent has changed, the state of the node is set as if it's position has changed
    {
        l_scene.tree.add_child(l_scene.get_node(l_node_1), l_scene.get_node(l_node_3));
        assert_true(l_scene.get_node(l_node_3).Element->state.haschanged_thisframe == 1);
        assert_true(l_scene.get_node(l_node_5).Element->state.haschanged_thisframe == 1);
        assert_true(token_equals(l_scene.get_node(l_node_3).Node->parent, l_scene.get_node(l_node_1).Node->index));
    }

    // remove node
    {
        l_scene.remove_node(l_scene.get_node(l_node_1));
        assert_true(l_scene.get_node_childs(l_scene.get_node(Scene_const::root_node)).Size == 1);

        // deleted nodes have been pushed to the deleted node stack
        assert_true(l_scene.scene_events.orphan_nodes_to_be_destroyed.Size == 1);

        // the link between node and components is only broken after the step call
        assert_true(!l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_1)));
        assert_true(!l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_2)));
        assert_true(!l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_3)));
        assert_true(!l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_5)));

        l_scene.step();
        assert_true(l_scene.scene_events.orphan_nodes_to_be_destroyed.Size == 0);
        assert_true(l_scene.tree.node_tree.is_node_free(l_node_1));
        assert_true(l_scene.tree.node_tree.is_node_free(l_node_2));
        assert_true(l_scene.tree.node_tree.is_node_free(l_node_3));
        assert_true(l_scene.tree.node_tree.is_node_free(l_node_5));

        assert_true(l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_1)));
        assert_true(l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_2)));
        assert_true(l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_3)));
        assert_true(l_scene.node_to_components.is_element_free(token_build_from<Scene::NodeToComponents_TokenValue>(l_node_5)));
    }

    l_scene.remove_node(l_scene.get_node(l_node_4));
    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();
};

inline void add_remove_component()
{
    Scene l_scene = Scene::allocate_default();

    // Added components are directly added to the associated node of the SceneTree
    {
        transform l_node_1_transform = transform{v3f_const::FORWARD.vec3, quat_const::IDENTITY, v3f_const::ZERO.vec3};
        Node_Token l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
        token_t l_component_test_resource = 1;
        l_scene.add_node_component_typed<ComponentTest>(l_node_1, l_component_test_resource);

        NodeComponent* l_component_test = l_scene.get_node_component_typed<ComponentTest>(l_node_1);
        assert_true(l_component_test->type == ComponentTest::Type);
        assert_true(l_component_test->resource == l_component_test_resource);

        token_t l_component_test_2_resource = 2;
        l_scene.add_node_component_typed<ComponentTest2>(l_node_1, l_component_test_2_resource);

        l_scene.step();

        assert_true(l_scene.scene_events.component_removed_events.Size == 0);

        l_scene.remove_node_component_typed<ComponentTest>(l_node_1); // ensuring that no error
        assert_true(l_scene.get_node_component_typed<ComponentTest2>(l_node_1) != NULL);
        assert_true(l_scene.get_node_component_typed<ComponentTest2>(l_node_1)->resource == l_component_test_2_resource);

        // One component removed event have been generated
        assert_true(l_scene.scene_events.component_removed_events.Size == 1);
        assert_true(l_scene.scene_events.component_removed_events.get(0).value.type == ComponentTest::Type);
        assert_true(token_equals(l_scene.scene_events.component_removed_events.get(0).node, l_node_1));

        l_scene.remove_node(l_scene.get_node(l_node_1));
    }

    {
    }

    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();

    l_scene = Scene::allocate_default();

    // deleting a node with components will generate events that can be consumed
    {

        transform l_node_1_transform = transform{v3f_const::FORWARD.vec3, quat_const::IDENTITY, v3f_const::ZERO.vec3};
        Node_Token l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
        l_scene.add_node_component_typed<ComponentTest>(l_node_1, 0);
        l_scene.add_node_component_typed<ComponentTest2>(l_node_1, 0);

        l_scene.step();

        assert_true(l_scene.scene_events.component_removed_events.Size == 0);

        l_scene.remove_node(l_scene.get_node(l_node_1));

        assert_true(l_scene.scene_events.component_removed_events.Size == 2);
        assert_true(l_scene.scene_events.component_removed_events.get(0).value.type == ComponentTest::Type);
        assert_true(token_equals(l_scene.scene_events.component_removed_events.get(0).node, l_node_1));
        assert_true(l_scene.scene_events.component_removed_events.get(1).value.type == ComponentTest2::Type);
        assert_true(token_equals(l_scene.scene_events.component_removed_events.get(1).node, l_node_1));

        l_scene.step();
    }

    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();
};

inline void component_consume()
{

    struct component_consume_callbacks
    {
        struct closure
        {
            int8 component_test_1_removed_called;
            int8 component_test_2_removed_called;
        };
        closure* thiz;

        inline static component_consume_callbacks build_default(closure& p_closure)
        {
            return component_consume_callbacks{&p_closure};
        };

        inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component) const
        {
            switch (p_component.type)
            {
            case ComponentTest::Type:
                thiz->component_test_1_removed_called += 1;
                break;
            case ComponentTest2::Type:
                thiz->component_test_2_removed_called += 1;
                break;
            default:
                abort();
            }
        };
    };

    Scene l_scene = Scene::allocate_default();

    // Checking that when components are removed and events are consumed, proper on_component_removed callback is called
    {
        transform l_node_1_transform = transform{v3f_const::FORWARD.vec3, quat_const::IDENTITY, v3f_const::ZERO.vec3};
        Node_Token l_node_1 = l_scene.add_node(l_node_1_transform, Scene_const::root_node);
        Node_Token l_node_2 = l_scene.add_node(l_node_1_transform, l_node_1);

        l_scene.add_node_component_typed<ComponentTest>(l_node_1, 0);
        l_scene.add_node_component_typed<ComponentTest2>(l_node_1, 1);
        l_scene.add_node_component_typed<ComponentTest2>(l_node_2, 1);

        component_consume_callbacks::closure l_callbacks_closure = component_consume_callbacks::closure{0};
        component_consume_callbacks l_callbacks = component_consume_callbacks::build_default(l_callbacks_closure);
        assert_true(l_scene.scene_events.component_removed_events.Size == 0);
        // l_scene.consume_component_events_stateful(l_callbacks);

        // assert_true(l_callbacks.component_test_1_added_called);
        // assert_true(l_callbacks.component_test_2_added_called);

        l_scene.remove_node_component_typed<ComponentTest2>(l_node_1);
        assert_true(l_scene.scene_events.component_removed_events.Size == 1); // 1 component from l_node_1

        l_scene.consume_component_events_stateful(l_callbacks);

        assert_true(l_callbacks_closure.component_test_2_removed_called == 1);
        assert_true(l_callbacks_closure.component_test_1_removed_called == 0);

        l_scene.remove_node(l_scene.get_node(l_node_1));

        assert_true(l_scene.scene_events.component_removed_events.Size == 2); // 1 components from l_node_1 and 1 from l_node_2

        l_scene.consume_component_events_stateful(l_callbacks);
        assert_true(l_callbacks_closure.component_test_2_removed_called == 2);
        assert_true(l_callbacks_closure.component_test_1_removed_called == 1);
    }

    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();
};

inline void math_hierarchy()
{
    Scene l_scene = Scene::allocate_default();

    Node_Token l_node_1 = l_scene.add_node(transform_const::ORIGIN, token_build<Node_TokenValue>(0));
    Node_Token l_node_2 = l_scene.add_node(transform{v3f_const::ONE.vec3, quat_const::IDENTITY, v3f_const::ONE.vec3}, l_node_1);
    Node_Token l_node_3 = l_scene.add_node(transform{v3f{-1.0f, -1.0f, -1.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, l_node_1);

    NodeEntry l_node_1_val = l_scene.get_node(l_node_1);
    NodeEntry l_node_2_val = l_scene.get_node(l_node_2);
    NodeEntry l_node_3_val = l_scene.get_node(l_node_3);

    // Position
    {
        l_scene.tree.set_localposition(l_node_1_val, v3f{1.0f, 1.0f, 1.0f});
        assert_true(l_scene.tree.get_localposition(l_node_1_val) == v3f{1.00000000f, 1.00000000f, 1.00000000f});
        assert_true(l_scene.tree.get_worldposition(l_node_2_val) == v3f{2.00000000f, 2.00000000f, 2.00000000f});
        assert_true(l_scene.tree.get_worldposition(l_node_3_val) == v3f{0.000000000f, 0.000000000f, 0.000000000f});

        l_scene.tree.set_localposition(l_node_2_val, v3f{1.0f, 2.0f, 3.0f});
        assert_true(l_scene.tree.get_localposition(l_node_2_val) == v3f{1.00000000f, 2.00000000f, 3.00000000f});
        assert_true(l_scene.tree.get_worldposition(l_node_2_val) == v3f{2.00000000f, 3.00000000f, 4.00000000f});

        l_scene.tree.set_worldposition(l_node_3_val, v3f{-1.0f, -2.0f, 3.0f});
        assert_true(l_scene.tree.get_localposition(l_node_3_val) == v3f{-2.00000000f, -3.00000000f, 2.00000000f});
        assert_true(l_scene.tree.get_worldposition(l_node_3_val) == v3f{-1.00000000f, -2.00000000f, 3.00000000f});
    }

    // Rotations
    {
        l_scene.tree.set_localrotation(l_node_1_val, v3f{0.32f, 0.9f, 0.7f}.euler_to_quat());
        assert_true(l_scene.tree.get_localrotation(l_node_1_val) == quat{-0.0124835223f, 0.452567190f, 0.239721030f, 0.858813643f});
        assert_true(l_scene.tree.get_worldrotation(l_node_2_val) == quat{-0.0124835242f, 0.452567250f, 0.239721060f, 0.858813763f});
        assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == quat{-0.0124835242f, 0.452567250f, 0.239721060f, 0.858813763f});

        l_scene.tree.set_localrotation(l_node_2_val, v3f{0.32f, -0.9f, -0.7f}.euler_to_quat());
        assert_true(l_scene.tree.get_localrotation(l_node_2_val) == quat{-0.0124835223f, -0.452567190f, -0.239721030f, 0.858813643f});
        assert_true(l_scene.tree.get_worldrotation(l_node_2_val) == quat{-0.0214420408f, -0.00598512636f, 0.0112992665f, 0.999688387f});

        l_scene.tree.set_worldrotation(l_node_3_val, v3f{-1.0f, -2.0f, 3.0f}.euler_to_quat());
        assert_true(l_scene.tree.get_localrotation(l_node_3_val) == quat{0.346717477f, -0.641801417f, 0.598375380f, 0.331398427f});
        assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == quat{0.718287051f, -0.310622454f, 0.444435090f, 0.435952842f});

        l_scene.tree.set_worldrotation(l_node_3_val, quat_const::IDENTITY);
        l_scene.tree.add_worldrotation(l_node_3_val, quat{0.0f, 0.1f, 0.2f, 0.3f});
        l_scene.tree.add_worldrotation(l_node_3_val, quat{0.0f, 0.2f, 0.4f, 0.3f});
        assert_true(l_scene.tree.get_worldrotation(l_node_3_val) == (quat{0.0f, 0.1f, 0.2f, 0.3f} * quat{0.0f, 0.2f, 0.4f, 0.3f}));
    }

    // Scale
    {
        l_scene.tree.set_localscale(l_node_1_val, v3f{2.0f, 0.5f, 1.0f});
        assert_true(l_scene.tree.get_localscale(l_node_1_val) == v3f{2.00000000f, 0.500000000f, 1.00000000f});
        assert_true(l_scene.tree.get_worldscalefactor(l_node_2_val) == v3f{2.00000000f, 0.500000000f, 1.00000000f});
        assert_true(l_scene.tree.get_worldscalefactor(l_node_3_val) == v3f{2.00000000f, 0.500000000f, 1.00000000f});

        l_scene.tree.set_localscale(l_node_2_val, v3f{1.0f, 2.0f, 3.0f});
        assert_true(l_scene.tree.get_localscale(l_node_2_val) == v3f{1.00000000f, 2.00000000f, 3.00000000f});
        assert_true(l_scene.tree.get_worldscalefactor(l_node_2_val) == v3f{2.00000000f, 1.00000000f, 3.00000000f});

        l_scene.tree.set_worldscale(l_node_3_val, v3f{-1.0f, -2.0f, 3.0f});
        assert_true(l_scene.tree.get_localscale(l_node_3_val) == v3f{-0.500000000f, -4.00000000f, 3.00000000f});
        assert_true(l_scene.tree.get_worldscalefactor(l_node_3_val) == v3f{-1.00000000f, -2.00000000f, 3.00000000f});
    }

    l_scene.remove_node(l_scene.get_node(l_node_1));

    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();
};

struct CameraTestComponent
{
    static constexpr component_t Type = HashFunctions::hash_compile<strlen_compile::get_size(STR(CameraTestComponent))>(STR(CameraTestComponent));
    float i0, i1;
};

struct MeshRendererTestComponent
{
    static constexpr component_t Type = HashFunctions::hash_compile<strlen_compile::get_size(STR(MeshRendererTestComponent))>(STR(MeshRendererTestComponent));
    float i0, i1, i2;
};

struct SceneJSONTestAsset
{
    static const char* scene_json;

    inline static void push_json_to_sceneassettree(JSONDeserializer& p_component_object, const hash_t p_type, const Token<NTree<transform>::Node> p_node, SceneAsset* in_out_SceneAssetTree)
    {
        switch (p_type)
        {
        case CameraTestComponent::Type:
        {
            CameraTestComponent l_component_test;

            p_component_object.next_field("test_value_1");
            l_component_test.i0 = FromString::afloat32(p_component_object.get_currentfield().value);

            p_component_object.next_field("test_value_2");
            l_component_test.i1 = FromString::afloat32(p_component_object.get_currentfield().value);

            in_out_SceneAssetTree->add_component_typed(p_node, l_component_test);
        }
        break;
        case MeshRendererTestComponent::Type:
        {
            MeshRendererTestComponent l_mesh_renderer_test;

            p_component_object.next_field("i0");
            l_mesh_renderer_test.i0 = FromString::afloat32(p_component_object.get_currentfield().value);

            p_component_object.next_field("i1");
            l_mesh_renderer_test.i1 = FromString::afloat32(p_component_object.get_currentfield().value);

            p_component_object.next_field("i2");
            l_mesh_renderer_test.i2 = FromString::afloat32(p_component_object.get_currentfield().value);

            in_out_SceneAssetTree->add_component_typed(p_node, l_mesh_renderer_test);
        }
        break;
        }
    };
};

const char* SceneJSONTestAsset::scene_json =
    "{\"type\":\"scene\",\"nodes\":[{\"local_position\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"local_rotation\":{\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\",\"w\":\"1.0\"},\"local_scale\":{"
    "\"x\":\"1.0\",\"y\":\"1.0\",\"z\":\"1.0\"},\"components\":[{\"type\":\"CameraTestComponent\",\"object\":{\"test_value_1\":\"10.0\",\"test_value_2\":\"20.0\"}}],\"childs\":[]},{\"local_"
    "position\":{\"x\":\"2.0\",\"y\":\"2.0\",\"z\":\"2.0\"},\"local_rotation\":{\"x\":\"2.0\",\"y\":\"2.0\",\"z\":\"2.0\",\"w\":\"2.0\"},\"local_scale\":{\"x\":\"2.0\",\"y\":\"2.0\",\"z\":\"2.0\"},"
    "\"components\":[{\"type\":\"CameraTestComponent\",\"object\":{\"test_value_1\":\"20.0\",\"test_value_2\":\"30.0\"}}],\"childs\":[{\"local_position\":{\"x\":\"3.0\",\"y\":\"3.0\",\"z\":\"3.0\"},"
    "\"local_rotation\":{\"x\":\"3.0\",\"y\":\"3.0\",\"z\":\"3.0\",\"w\":\"3.0\"},\"local_scale\":{\"x\":\"3.0\",\"y\":\"3.0\",\"z\":\"3.0\"},\"components\":[],\"childs\":[{\"local_position\":{\"x\":"
    "\"4.0\",\"y\":\"4.0\",\"z\":\"4.0\"},\"local_rotation\":{\"x\":\"4.0\",\"y\":\"4.0\",\"z\":\"4.0\",\"w\":\"4.0\"},\"local_scale\":{\"x\":\"4.0\",\"y\":\"4.0\",\"z\":\"4.0\"},\"components\":[{"
    "\"type\":\"MeshRendererTestComponent\",\"object\":{\"i0\":\"40.0\",\"i1\":\"60.0\",\"i2\":\"80.0\"}}],\"childs\":[]}]},{\"local_position\":{\"x\":\"5.0\",\"y\":\"5.0\",\"z\":\"5.0\"},\"local_"
    "rotation\":{\"x\":\"5.0\",\"y\":\"5.0\",\"z\":\"5.0\",\"w\":\"5.0\"},\"local_scale\":{\"x\":\"5.0\",\"y\":\"5.0\",\"z\":\"5.0\"},\"components\":[{\"type\":\"MeshRendererTestComponent\","
    "\"object\":{\"i0\":\"50.0\",\"i1\":\"70.0\",\"i2\":\"90.0\"}}],\"childs\":[]}]}]}";

inline void json_deserialization()
{
#if 1
    String l_scene_json_string = String::allocate(0);
    l_scene_json_string.append(slice_int8_build_rawstr(SceneJSONTestAsset::scene_json));
    JSONDeserializer l_serailizer = JSONDeserializer::sanitize_and_start(l_scene_json_string.Memory.to_ivector());
    SceneAsset l_scene_asset_tree = SceneAsset::allocate_default();
    SceneJSON_TO_SceneAsset::json_to_SceneAsset<SceneJSONTestAsset>(l_serailizer, &l_scene_asset_tree);

    // we check if the tree structure is respected
    {
        NTree<transform>::Resolve l_first = l_scene_asset_tree.nodes.get(token_build<NTree<transform>::Node>(1));
        assert_true(l_first.Element->operator==(transform{v3f{1.0f, 1.0f, 1.0f}, quat{1.0f, 1.0f, 1.0f, 1.0f}, v3f{1.0f, 1.0f, 1.0f}}));
        Slice<Token<SliceIndex>> l_first_components = l_scene_asset_tree.get_components(l_first);
        assert_true(l_first_components.Size == 1);
        CameraTestComponent* l_camera_component = l_scene_asset_tree.get_component_typed<CameraTestComponent>(l_first_components.get(0));
        assert_true(l_camera_component->i0 == 10.0f);
        assert_true(l_camera_component->i1 == 20.0f);
        assert_true(l_scene_asset_tree.nodes.get_childs(l_first.Node->childs).Size == 0);
    }

    {
        NTree<transform>::Resolve l_second = l_scene_asset_tree.nodes.get(token_build<NTree<transform>::Node>(2));
        assert_true(l_second.Element->operator==(transform{v3f{2.0f, 2.0f, 2.0f}, quat{2.0f, 2.0f, 2.0f, 2.0f}, v3f{2.0f, 2.0f, 2.0f}}));
        Slice<Token<SliceIndex>> l_second_components = l_scene_asset_tree.get_components(l_second);
        assert_true(l_second_components.Size == 1);
        CameraTestComponent* l_camera_component = l_scene_asset_tree.get_component_typed<CameraTestComponent>(l_second_components.get(0));
        assert_true(l_camera_component->i0 == 20.0f);
        assert_true(l_camera_component->i1 == 30.0f);
        assert_true(l_scene_asset_tree.nodes.get_childs(l_second.Node->childs).Size == 2);
    }

    {
        NTree<transform>::Resolve l_2_1 = l_scene_asset_tree.nodes.get(token_build<NTree<transform>::Node>(3));
        assert_true(l_2_1.Element->operator==(transform{v3f{3.0f, 3.0f, 3.0f}, quat{3.0f, 3.0f, 3.0f, 3.0f}, v3f{3.0f, 3.0f, 3.0f}}));
        Slice<Token<SliceIndex>> l_2_1_components = l_scene_asset_tree.get_components(l_2_1);
        assert_true(l_2_1_components.Size == 0);
        assert_true(l_scene_asset_tree.nodes.get_childs(l_2_1.Node->childs).Size == 1);
    }

    {
        NTree<transform>::Resolve l_2_1_1 = l_scene_asset_tree.nodes.get(token_build<NTree<transform>::Node>(4));
        assert_true(l_2_1_1.Element->operator==(transform{v3f{4.0f, 4.0f, 4.0f}, quat{4.0f, 4.0f, 4.0f, 4.0f}, v3f{4.0f, 4.0f, 4.0f}}));
        Slice<Token<SliceIndex>> l_2_1_1_components = l_scene_asset_tree.get_components(l_2_1_1);
        assert_true(l_2_1_1_components.Size == 1);
        MeshRendererTestComponent* l_mesh_renderer_test_component = l_scene_asset_tree.get_component_typed<MeshRendererTestComponent>(l_2_1_1_components.get(0));
        assert_true(l_mesh_renderer_test_component->i0 == 40.0f);
        assert_true(l_mesh_renderer_test_component->i1 == 60.0f);
        assert_true(l_mesh_renderer_test_component->i2 == 80.0f);
        assert_true(l_scene_asset_tree.nodes.get_childs(l_2_1_1.Node->childs).Size == 0);
    }

    {
        NTree<transform>::Resolve l_2_2 = l_scene_asset_tree.nodes.get(token_build<NTree<transform>::Node>(5));
        assert_true(l_2_2.Element->operator==(transform{v3f{5.0f, 5.0f, 5.0f}, quat{5.0f, 5.0f, 5.0f, 5.0f}, v3f{5.0f, 5.0f, 5.0f}}));
        Slice<Token<SliceIndex>> l_2_2_components = l_scene_asset_tree.get_components(l_2_2);
        assert_true(l_2_2_components.Size == 1);
        MeshRendererTestComponent* l_mesh_renderer_test_component = l_scene_asset_tree.get_component_typed<MeshRendererTestComponent>(l_2_2_components.get(0));
        assert_true(l_mesh_renderer_test_component->i0 == 50.0f);
        assert_true(l_mesh_renderer_test_component->i1 == 70.0f);
        assert_true(l_mesh_renderer_test_component->i2 == 90.0f);
        assert_true(l_scene_asset_tree.nodes.get_childs(l_2_2.Node->childs).Size == 0);
    }

    int16 l_counter = 0;
    l_scene_asset_tree.nodes.traverse3(token_build<NTree<transform>::Node>(0), [&l_counter](const auto&) {
        l_counter += 1;
    });

    assert_true(l_counter == 6);

    l_scene_asset_tree.free();
    l_scene_json_string.free();
#endif
};

inline void scenetreeasset_merge()
{
#if 1
    String l_scene_json_string = String::allocate(0);
    l_scene_json_string.append(slice_int8_build_rawstr(SceneJSONTestAsset::scene_json));
    JSONDeserializer l_serailizer = JSONDeserializer::sanitize_and_start(l_scene_json_string.Memory.to_ivector());
    SceneAsset l_scene_asset_tree = SceneAsset::allocate_default();
    SceneJSON_TO_SceneAsset::json_to_SceneAsset<SceneJSONTestAsset>(l_serailizer, &l_scene_asset_tree);

    Scene l_scene = Scene::allocate_default();

    Node_Token l_node_1 = l_scene.add_node(transform_const::ORIGIN, token_build<Node_TokenValue>(0));
    Node_Token l_node_2 = l_scene.add_node(transform{v3f_const::ONE.vec3, quat_const::IDENTITY, v3f_const::ONE.vec3}, l_node_1);
    Node_Token l_node_3 = l_scene.add_node(transform{v3f{-1.0f, -1.0f, -1.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, l_node_1);

    assert_true(l_scene.get_node_childs(l_scene.get_node(l_node_3)).Size == 0);

    uimax l_counter = 0;
    auto l_component_allocator = [&l_counter](const SceneAssetComponent& p_sceneasset_component) {
        uimax l_old_counter = l_counter;
        l_counter += 1;
        return l_old_counter;
    };
    l_scene_asset_tree.merge_to_scene(&l_scene, l_node_3, l_component_allocator);

    assert_true(l_scene.get_node_childs(l_scene.get_node(l_node_3)).Size == 1);

    Node_Token l_sub_tree_root = l_scene.get_node_childs(l_scene.get_node(l_node_3)).get(0);
    Slice<Node_Token> l_sub_tree_root_childs = l_scene.get_node_childs(l_scene.get_node(l_sub_tree_root));
    {
        NodeEntry l_first = l_scene.get_node(l_sub_tree_root_childs.get(0));
        assert_true(l_first.Element->local_transform == transform{v3f{1.0f, 1.0f, 1.0f}, quat{1.0f, 1.0f, 1.0f, 1.0f}, v3f{1.0f, 1.0f, 1.0f}});
        NodeComponent* l_camera_component = l_scene.get_node_component_typed<CameraTestComponent>(l_first.Node->index);
        assert_true(l_camera_component->type == CameraTestComponent::Type);
        assert_true(l_camera_component->resource == 0);
        assert_true(l_scene.get_node_childs(l_first).Size == 0);
    }

    {
        NodeEntry l_second = l_scene.get_node(l_sub_tree_root_childs.get(1));
        assert_true(l_second.Element->local_transform == transform{v3f{2.0f, 2.0f, 2.0f}, quat{2.0f, 2.0f, 2.0f, 2.0f}, v3f{2.0f, 2.0f, 2.0f}});
        NodeComponent* l_camera_component = l_scene.get_node_component_typed<CameraTestComponent>(l_second.Node->index);
        assert_true(l_camera_component->type == CameraTestComponent::Type);
        assert_true(l_camera_component->resource == 1);

        Slice<Node_Token> l_second_childs = l_scene.get_node_childs(l_second);
        assert_true(l_second_childs.Size == 2);

        {
            NodeEntry l_2_1 = l_scene.get_node(l_second_childs.get(0));
            assert_true(l_2_1.Element->local_transform == transform{v3f{3.0f, 3.0f, 3.0f}, quat{3.0f, 3.0f, 3.0f, 3.0f}, v3f{3.0f, 3.0f, 3.0f}});

            Slice<Node_Token> l_2_1_childs = l_scene.get_node_childs(l_2_1);
            assert_true(l_2_1_childs.Size == 1);

            {
                NodeEntry l_2_1_1 = l_scene.get_node(l_2_1_childs.get(0));
                assert_true(l_2_1_1.Element->local_transform == transform{v3f{4.0f, 4.0f, 4.0f}, quat{4.0f, 4.0f, 4.0f, 4.0f}, v3f{4.0f, 4.0f, 4.0f}});
                NodeComponent* l_camera_component = l_scene.get_node_component_typed<MeshRendererTestComponent>(l_2_1_1.Node->index);
                assert_true(l_camera_component->type == MeshRendererTestComponent::Type);
                assert_true(l_camera_component->resource == 2);
                assert_true(l_scene.get_node_childs(l_2_1_1).Size == 0);
            }
        }

        {
            NodeEntry l_2_2 = l_scene.get_node(l_second_childs.get(1));
            assert_true(l_2_2.Element->local_transform == transform{v3f{5.0f, 5.0f, 5.0f}, quat{5.0f, 5.0f, 5.0f, 5.0f}, v3f{5.0f, 5.0f, 5.0f}});
            NodeComponent* l_camera_component = l_scene.get_node_component_typed<MeshRendererTestComponent>(l_2_2.Node->index);
            assert_true(l_camera_component->type == MeshRendererTestComponent::Type);
            assert_true(l_camera_component->resource == 3);
            assert_true(l_scene.get_node_childs(l_2_2).Size == 0);
        }
    }

    assert_true(l_counter == 4);

    l_scene_asset_tree.free();
    l_scene_json_string.free();
    l_scene.remove_node(l_scene.get_node(l_node_1));
    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();
#endif
};

inline void scene_to_sceneasset()
{
    Scene l_scene = Scene::allocate_default();

    Node_Token l_node_1 = l_scene.add_node(transform_const::ORIGIN, token_build<Node_TokenValue>(0));
    Node_Token l_node_2 = l_scene.add_node(transform{v3f_const::ONE.vec3, quat_const::IDENTITY, v3f_const::ONE.vec3}, l_node_1);
    Node_Token l_node_3 = l_scene.add_node(transform{v3f{-1.0f, -1.0f, -1.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, l_node_1);

    l_scene.add_node_component_typed<CameraTestComponent>(l_node_1, 0);
    l_scene.add_node_component_typed<MeshRendererTestComponent>(l_node_1, 1);

    l_scene.add_node_component_typed<CameraTestComponent>(l_node_2, 2);
    l_scene.add_node_component_typed<MeshRendererTestComponent>(l_node_3, 3);

    SceneAsset l_scene_asset = SceneAsset::allocate_default();

    l_scene_asset.scene_copied_to(&l_scene, l_node_1,
                                  [&l_scene_asset](const NodeComponent& p_node_component, const SceneAsset::AddComponentAssetToSceneAsset& p_add_componentasset_to_sceneasset_callback) {
                                      switch (p_node_component.type)
                                      {
                                      case CameraTestComponent::Type:
                                          p_add_componentasset_to_sceneasset_callback.add<CameraTestComponent>(CameraTestComponent{});
                                          break;
                                      case MeshRendererTestComponent::Type:
                                          p_add_componentasset_to_sceneasset_callback.add<MeshRendererTestComponent>(MeshRendererTestComponent{});
                                          break;
                                      }
                                  });

    Slice<Token<SliceIndex>> l_1_components = l_scene_asset.get_components(l_scene_asset.nodes.get(token_build<NTree<transform>::Node>(0)));
    assert_true(l_1_components.Size == 2);
    l_scene_asset.get_component_typed<CameraTestComponent>(l_1_components.get(0));
    l_scene_asset.get_component_typed<MeshRendererTestComponent>(l_1_components.get(1));

    Slice<Token<SliceIndex>> l_2_components = l_scene_asset.get_components(l_scene_asset.nodes.get(token_build<NTree<transform>::Node>(1)));
    assert_true(l_2_components.Size == 1);
    l_scene_asset.get_component_typed<CameraTestComponent>(l_2_components.get(0));

    Slice<Token<SliceIndex>> l_3_components = l_scene_asset.get_components(l_scene_asset.nodes.get(token_build<NTree<transform>::Node>(2)));
    assert_true(l_3_components.Size == 1);
    l_scene_asset.get_component_typed<MeshRendererTestComponent>(l_3_components.get(0));

    l_scene_asset.free();
    l_scene.remove_node(l_scene.get_node(l_node_1));
    l_scene.free_and_consume_component_events<DefaulSceneComponentReleaser>();
};

inline void sceneasset_to_json(){
    // TODO sceneasset_to_json
};

int main()
{
    add_remove_setparent_node();
    add_remove_component();
    component_consume();
    math_hierarchy();
    json_deserialization();
    scenetreeasset_merge();
    scene_to_sceneasset();
    sceneasset_to_json();

    memleak_ckeck();
};

#include "Common2/common2_external_implementation.hpp"