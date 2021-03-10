

#include "./Sandbox/sandbox.hpp"
#include "AssetCompiler/asset_compiler.hpp"

struct BoxCollisionSandboxEnvironment
{
    TokenT(v2::Node) moving_node;
    TokenT(ColliderDetector) moving_node_collider_detector;
    TokenT(v2::Node) static_node;
    TokenT(v2::BoxColliderComponent) static_node_boxcollider_component;

    inline static BoxCollisionSandboxEnvironment build_default()
    {
        return BoxCollisionSandboxEnvironment{tk_bdT(v2::Node), tk_bdT(ColliderDetector), tk_bdT(v2::Node), tk_bdT(v2::BoxColliderComponent)};
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

            TokenT(v2::BoxColliderComponent) l_node_1_box_collider_component =
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

    inline void end_of_frame(Engine& p_engine){

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

namespace D3RendererCubeSandboxEnvironment_Const
{
const hash_t block_1x1_material = HashSlice(slice_int8_build_rawstr("block_1x1_material.json"));
const hash_t block_1x1_obj = HashSlice(slice_int8_build_rawstr("block_1x1.obj"));
} // namespace D3RendererCubeSandboxEnvironment_Const

struct D3RendererCubeSandboxEnvironment
{

    TokenT(v2::Node) camera_node;
    TokenT(v2::Node) l_square_root_node;

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
            quat l_rot = m33f::lookat(v3f{7.0f, 7.0f, 7.0f}, v3f{0.0f, 0.0f, 0.0f}, v3f_const::UP).to_rotation();
            v3f l_euler = l_rot.euler();
            this->camera_node = p_engine.scene.add_node(transform{v3f{7.0f, 7.0f, 7.0f}, l_rot, v3f_const::ONE}, v2::Scene_const::root_node);
            p_engine.scene_middleware.render_middleware.allocate_camera_inline(v2::CameraComponent::Asset{1.0f, 30.0f, 45.0f}, camera_node);
            p_engine.scene.add_node_component_by_value(camera_node, v2::CameraComponentAsset_SceneCommunication::construct_nodecomponent());

            {
                {
                    this->l_square_root_node = p_engine.scene.add_node(transform_const::ORIGIN, v2::Scene_const::root_node);
                }
                {
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{-2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{-2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{-2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                    {
                        TokenT(v2::Node) l_node = p_engine.scene.add_node(transform{v3f{-2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                        SandboxUtility::add_meshrenderer_component_to_node_with_database(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material,
                                                                                         D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                    }
                }
            }
        }
        else if (p_engine.clock.framecount == 60)
        {

            p_engine.scene.remove_node(p_engine.scene.get_node(this->camera_node));
            p_engine.scene.remove_node(p_engine.scene.get_node(this->l_square_root_node));
            p_engine.close();
        }
        else
        {
            NTree<v2::Node>::Resolve l_node = p_engine.scene.get_node(this->l_square_root_node);
            p_engine.scene.tree.set_worldrotation(l_node,
                                                  p_engine.scene.tree.get_worldrotation(l_node) * quat::rotate_around(v3f_const::UP, 45.0f * v2::Math_const::DEG_TO_RAD * p_engine.clock.deltatime));
        }
    };

    inline void end_of_frame(Engine& p_engine)
    {
        if (p_engine.clock.framecount == 20 || p_engine.clock.framecount == 40)
        {
            ImageFormat l_rendertarget_texture_format;
            TokenT(v2::BufferHost) l_rendertarget_texture = v2::GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost_with_imageformat(
                p_engine.gpu_context.buffer_memory, p_engine.gpu_context.graphics_allocator, p_engine.gpu_context.graphics_allocator.heap.graphics_pass.get(p_engine.renderer.color_step.pass), 0,
                &l_rendertarget_texture_format);

            p_engine.gpu_context.buffer_step_and_wait_for_completion();
            Slice<int8> l_rendertarget_texture_value = p_engine.gpu_context.buffer_memory.allocator.host_buffers.get(l_rendertarget_texture).get_mapped_memory();

            String l_image_path = String::allocate_elements(slice_int8_build_rawstr(ASSET_FOLDER_PATH));
            l_image_path.append(slice_int8_build_rawstr("/d3renderer_cube/frame/frame_"));
            ToString::auimax_append(p_engine.clock.framecount, l_image_path);
            l_image_path.append(slice_int8_build_rawstr(".jpg"));

#if 0
            ImgCompiler::write_to_image(l_image_path.to_slice_wuth_null_termination(), l_rendertarget_texture_format.extent.x, l_rendertarget_texture_format.extent.y, 4,
                                        l_rendertarget_texture_value);
#endif

            Span<int8> l_image = ImgCompiler::read_image(l_image_path.to_slice_with_null_termination());
            assert_true(l_rendertarget_texture_value.compare(l_image.slice));
            l_image.free();
            l_image_path.free();

            v2::BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_engine.gpu_context.buffer_memory.allocator, p_engine.gpu_context.buffer_memory.events,
                                                                                         l_rendertarget_texture);
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
    l_configuration.render_size = v2ui{800, 600};
    l_configuration.render_target_host_readable = 1;
    SandboxEngineRunner l_runner = SandboxEngineRunner::allocate(EngineConfiguration{l_configuration}, 1.0f / 60.0f);
    l_database_path.free();

    D3RendererCubeSandboxEnvironment l_sandbox_environment = D3RendererCubeSandboxEnvironment::build_default();
    l_runner.main_loop(l_sandbox_environment);
}

int main()
{
    boxcollision();
    d3renderer_cube();

    memleak_ckeck();
};