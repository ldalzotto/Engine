
#include "./Sandbox/engine_runner.hpp"

struct BoxCollisionSandboxEnvironment
{
    Token(v2::Node) moving_node;
    Token(ColliderDetector) moving_node_collider_detector;
    Token(v2::Node) static_node;
    Token(v2::BoxColliderComponent) static_node_boxcollider_component;

    inline static BoxCollisionSandboxEnvironment build_default()
    {
        return BoxCollisionSandboxEnvironment{tk_bd(v2::Node), tk_bd(ColliderDetector), tk_bd(v2::Node), tk_bd(v2::BoxColliderComponent)};
    };

    inline void before_collision(Engine& p_engine){};

    inline void after_collision(Engine& p_engine)
    {

        if (p_engine.clock.framecount >= 2 && p_engine.clock.framecount <= 8)
        {
            Slice<TriggerEvent> l_0_trigger_events = p_engine.collision.get_collision_events(this->moving_node_collider_detector);

            switch (p_engine.clock.framecount)
            {
            case 2:
            {
                v2::BoxColliderComponent l_static_node_boxcollider =
                    p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(tk_eq(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_ENTER);
            }
            break;
            case 3:
            case 4:
            case 5:
            case 6:
            {
                v2::BoxColliderComponent l_static_node_boxcollider =
                    p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(tk_eq(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_STAY);
            }
            break;
            case 7:
            {
                v2::BoxColliderComponent l_static_node_boxcollider =
                    p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(tk_eq(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_EXIT);
            }
            break;
            case 8:
            {
                v2::BoxColliderComponent l_static_node_boxcollider =
                    p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(tk_eq(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::NONE);
            }
            break;
            }
        }
    };

    inline void before_update(Engine& p_engine)
    {
        if (p_engine.clock.framecount == 1)
        {
            this->moving_node = p_engine.scene.add_node(transform{v3f{0.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE}, v2::Scene_const::root_node);
            this->static_node = p_engine.scene.add_node(transform{v3f{2.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE}, v2::Scene_const::root_node);

            Token(v2::BoxColliderComponent) l_node_1_box_collider_component =
                p_engine.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(p_engine.collision, this->moving_node, v2::BoxColliderComponentAsset{v3f_const::ONE});
            p_engine.scene.add_node_component_by_value(this->moving_node, v2::NodeComponent::build(v2::BoxColliderComponent::Type, tk_v(l_node_1_box_collider_component)));
            this->moving_node_collider_detector = p_engine.scene_middleware.collision_middleware.allocator.attach_collider_detector(p_engine.collision, l_node_1_box_collider_component);

            this->static_node_boxcollider_component =
                p_engine.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(p_engine.collision, this->static_node, v2::BoxColliderComponentAsset{v3f_const::ONE});
            p_engine.scene.add_node_component_by_value(this->static_node, v2::NodeComponent::build(v2::BoxColliderComponent::Type, tk_v(this->static_node_boxcollider_component)));
        }
        else if (p_engine.clock.framecount == 2 || p_engine.clock.framecount == 3 || p_engine.clock.framecount == 4 || p_engine.clock.framecount == 5 || p_engine.clock.framecount == 6 ||
                 p_engine.clock.framecount == 7)
        {
            v2::NodeEntry l_node_1_value = p_engine.scene.get_node(this->moving_node);
            p_engine.scene.tree.set_localposition(l_node_1_value, p_engine.scene.tree.get_localposition(l_node_1_value) + v3f{1.0f, 0.0f, 0.0f});
        }

        if (p_engine.clock.framecount == 9)
        {
            p_engine.scene.remove_node(p_engine.scene.get_node(this->moving_node));
            p_engine.scene.remove_node(p_engine.scene.get_node(this->static_node));

            p_engine.close();
        }
    };
};

inline void boxcollision()
{
    String l_database_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(slice_int8_build_rawstr("/boxcollision/asset.db"));
    {
        File l_tmp_file = File::create_or_open(l_database_path.to_slice());
        l_tmp_file.erase_with_slicepath();
        AssetDatabase::initialize_database(l_database_path.to_slice());
    }
    EngineConfiguration l_condfiguration;
    l_condfiguration.asset_database_path = l_database_path.to_slice();
    l_condfiguration.headless = 1;
    SandboxEngineRunner l_runner = SandboxEngineRunner::allocate(l_condfiguration, 1.0f / 60.0f);
    l_database_path.free();

    BoxCollisionSandboxEnvironment l_sandbox_environment = BoxCollisionSandboxEnvironment::build_default();
    l_runner.main_loop_headless(l_sandbox_environment);
};

struct D3RendererCubeSandboxEnvironment
{
    Token(v2::Node) camera_node;
    Token(v2::Node) cube_node;

    inline static D3RendererCubeSandboxEnvironment build_default()
    {
        return D3RendererCubeSandboxEnvironment{};
    };

    inline void before_collision(Engine& p_engine){};

    inline void after_collision(Engine& p_engine){};

    inline void before_update(Engine& p_engine)
    {
        if (p_engine.clock.framecount == 1)
        {
            this->camera_node = p_engine.scene.add_node(transform{v3f{5.0f, 5.0f, 5.0f}, quat{-0.106073f, 0.867209f, -0.283699f, -0.395236f}, v3f_const::ONE}, v2::Scene_const::root_node);
            p_engine.scene_middleware.render_middleware.allocate_camera_inline(v2::CameraComponent::Asset{1.0f, 30.0f, 45.0f}, camera_node);
            p_engine.scene.add_node_component_by_value(camera_node, v2::CameraComponentAsset_SceneCommunication::construct_nodecomponent());

            this->cube_node = p_engine.scene.add_node(transform_const::ORIGIN, v2::Scene_const::root_node);
            Token(v2::MeshRendererComponent) l_mesh_renderer = v2::RenderMiddleWare_AllocationComposition::allocate_meshrenderer_database_and_load_dependecies(
                p_engine.scene_middleware.render_middleware, p_engine.renderer_ressource_allocator, p_engine.asset_database,
                v2::MeshRendererComponent::AssetDependencies{HashSlice(slice_int8_build_rawstr("block_1x1_ALT_1_material.json")), HashSlice(slice_int8_build_rawstr("block_1x1_ALT_1.obj"))}, this->cube_node);
            p_engine.scene.add_node_component_by_value(this->cube_node, v2::MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer));
        }
        /*
        else if (p_engine.clock.framecount == 60)
        {
            p_engine.scene.remove_node(p_engine.scene.get_node(this->camera_node));
            p_engine.scene.remove_node(p_engine.scene.get_node(this->cube_node));
            p_engine.close();
        }
         */
        else
        {
            NTree<v2::Node>::Resolve l_node = p_engine.scene.get_node(this->cube_node);
            p_engine.scene.tree.set_worldrotation(l_node,
                                                  p_engine.scene.tree.get_worldrotation(l_node) * quat::rotate_around(v3f_const::UP, 45.0f * v2::Math_const::DEG_TO_RAD * p_engine.clock.deltatime));
        }
    };
};

inline void d3renderer_cube()
{
    String l_database_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(slice_int8_build_rawstr("/d3renderer_cube/asset.db"));
    {
    }
    EngineConfiguration l_configuration{};
    l_configuration.asset_database_path = l_database_path.to_slice();
    l_configuration.render_size = v2ui{128, 128};
    SandboxEngineRunner l_runner = SandboxEngineRunner::allocate(EngineConfiguration{l_configuration}, 1.0f / 60.0f);
    l_database_path.free();

    D3RendererCubeSandboxEnvironment l_sandbox_environment = D3RendererCubeSandboxEnvironment::build_default();
    l_runner.main_loop(l_sandbox_environment);
}

int main()
{
    boxcollision();
    // d3renderer_cube();

    memleak_ckeck();
};