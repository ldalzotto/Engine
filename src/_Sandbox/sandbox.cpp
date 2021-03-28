

#include "./Sandbox/sandbox.hpp"
#include "AssetCompiler/asset_compiler.hpp"

struct SandboxTestUtil
{
    inline static void render_texture_compare(Engine& p_engine, const Slice<int8>& p_compared_image_path)
    {
        ImageFormat l_rendertarget_texture_format;
        Token(BufferHost) l_rendertarget_texture = GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost_with_imageformat(
            p_engine.gpu_context.buffer_memory, p_engine.gpu_context.graphics_allocator, p_engine.gpu_context.graphics_allocator.heap.graphics_pass.get(p_engine.renderer.color_step.pass), 0,
            &l_rendertarget_texture_format);

        p_engine.gpu_context.buffer_step_and_wait_for_completion();
        Slice<int8> l_rendertarget_texture_value = p_engine.gpu_context.buffer_memory.allocator.host_buffers.get(l_rendertarget_texture).get_mapped_effective_memory();


        Span<int8> l_image = ImgCompiler::read_image(p_compared_image_path);
        assert_true(l_rendertarget_texture_value.compare(l_image.slice));
        l_image.free();

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_engine.gpu_context.buffer_memory.allocator, p_engine.gpu_context.buffer_memory.events,
                                                                                 l_rendertarget_texture);
    };

    inline static void render_texture_screenshot(Engine& p_engine, const Slice<int8>& p_path)
    {
        ImageFormat l_rendertarget_texture_format;
        Token(BufferHost) l_rendertarget_texture = GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost_with_imageformat(
            p_engine.gpu_context.buffer_memory, p_engine.gpu_context.graphics_allocator, p_engine.gpu_context.graphics_allocator.heap.graphics_pass.get(p_engine.renderer.color_step.pass), 0,
            &l_rendertarget_texture_format);

        p_engine.gpu_context.buffer_step_and_wait_for_completion();
        Slice<int8> l_rendertarget_texture_value = p_engine.gpu_context.buffer_memory.allocator.host_buffers.get(l_rendertarget_texture).get_mapped_effective_memory();

        ImgCompiler::write_to_image(p_path, l_rendertarget_texture_format.extent.x, l_rendertarget_texture_format.extent.y, 4, l_rendertarget_texture_value);

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_engine.gpu_context.buffer_memory.allocator, p_engine.gpu_context.buffer_memory.events, l_rendertarget_texture);
    };
};

inline void resize_test()
{
    String l_database_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(Slice_int8_build_rawstr("/resize_test/asset.db"));
    {
    }
    EngineConfiguration l_configuration{};
    l_configuration.asset_database_path = l_database_path.to_slice();
    l_configuration.render_size = v2ui{400, 400};
    l_configuration.render_target_host_readable = 1;

    float32 l_delta_tume = 1.0f / 60.0f;
    Engine l_engine = Engine::allocate(EngineConfiguration{l_configuration});

    struct engine_loop
    {
        inline static void step(const EngineExternalStep p_step, Engine& p_engine)
        {
            if (p_engine.clock.framecount == 1)
            {
                if (p_step == EngineExternalStep::END_OF_FRAME)
                {
                    WindowNative::simulate_resize_appevent(g_app_windows.get(p_engine.window).handle, 500, 500);
                }
            }
            else if (p_engine.clock.framecount == 3)
            {
                if (p_step == EngineExternalStep::BEFORE_COLLISION)
                {
                    p_engine.close();
                }
            }
        };
    };

    engine_loop l_engine_loop = engine_loop{};
    EngineRunner::main_loop(l_engine, l_engine_loop);

    assert_true(g_app_windows.get(l_engine.window).client_width == 500);
    assert_true(g_app_windows.get(l_engine.window).client_height == 500);

    l_engine.free();
    l_database_path.free();
};

inline void engine_thread_test()
{
    EngineRunnerThread l_thread = EngineRunnerThread::allocate();
    l_thread.start();

    String l_database_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(Slice_int8_build_rawstr("/thread_test/asset.db"));

    struct s_engine_cb
    {
        uimax frame_count;
        int8 cleanup_called;
        inline static void step(EngineExternalStep p_step, Engine& p_engine, s_engine_cb* thiz)
        {
            thiz->frame_count = FrameCount(p_engine);
        };

        inline static void cleanup_ressources(Engine& p_engine, s_engine_cb* thiz)
        {
            thiz->cleanup_called = 1;
        };

        inline EngineExternalStepCallback build_external_step_callback()
        {
            return EngineExternalStepCallback{this, (EngineExternalStepCallback::cb_t)s_engine_cb::step};
        };

        inline EngineExecutionUnit::CleanupCallback build_cleanup_callback()
        {
            return EngineExecutionUnit::CleanupCallback{this, (EngineExecutionUnit::CleanupCallback::cb_t)s_engine_cb::cleanup_ressources};
        };
    };

    // only one engine
    {
        s_engine_cb engine_cb{};
        Token(EngineExecutionUnit) l_engine_eu_1 =
            l_thread.allocate_engine_execution_unit(l_database_path.to_slice(), 50, 50, engine_cb.build_external_step_callback(), engine_cb.build_cleanup_callback());
        l_thread.sync_wait_for_engine_execution_unit_to_be_allocated(l_engine_eu_1);
        l_thread.sync_engine_at_end_of_frame(l_engine_eu_1, [&]() {
            assert_true(engine_cb.frame_count == 2);
        });
        l_thread.sync_engine_at_end_of_frame(l_engine_eu_1, [&]() {
            assert_true(engine_cb.frame_count == 3);
        });
        l_thread.sync_engine_at_end_of_frame(l_engine_eu_1, [&]() {
            assert_true(engine_cb.frame_count == 4);
            assert_true(engine_cb.cleanup_called == 0);
        });
        l_thread.free_engine_execution_unit_sync(l_engine_eu_1);
        assert_true(engine_cb.cleanup_called == 1);
    }
    // two engine in //
    {
        s_engine_cb engine_cb_1{};
        s_engine_cb engine_cb_2{};
        Token(EngineExecutionUnit) l_engine_eu_1 =
            l_thread.allocate_engine_execution_unit(l_database_path.to_slice(), 50, 50, engine_cb_1.build_external_step_callback(), engine_cb_1.build_cleanup_callback());
        Token(EngineExecutionUnit) l_engine_eu_2 =
            l_thread.allocate_engine_execution_unit(l_database_path.to_slice(), 50, 50, engine_cb_2.build_external_step_callback(), engine_cb_2.build_cleanup_callback());

        l_thread.sync_wait_for_engine_execution_unit_to_be_allocated(l_engine_eu_1);
        l_thread.sync_wait_for_engine_execution_unit_to_be_allocated(l_engine_eu_2);

        uimax l_start_frame_1;
        uimax l_start_frame_2;

        int8 l_exit = 0;
        int8 l_step_count = 0;
        l_thread.sync_step_loop(
            [&]() {
                if (l_step_count == 0)
                {
                    l_start_frame_1 = engine_cb_1.frame_count;
                    l_start_frame_2 = engine_cb_2.frame_count;
                }
                else if (l_step_count == 1)
                {
                    assert_true(engine_cb_1.frame_count > l_start_frame_1);
                    assert_true(engine_cb_2.frame_count > l_start_frame_2);
                    l_start_frame_1 = engine_cb_1.frame_count;
                    l_start_frame_2 = engine_cb_2.frame_count;
                }
                else if (l_step_count == 2)
                {
                    assert_true(engine_cb_1.frame_count > l_start_frame_1);
                    assert_true(engine_cb_2.frame_count > l_start_frame_2);
                    l_start_frame_1 = engine_cb_1.frame_count;
                    l_start_frame_2 = engine_cb_2.frame_count;
                }
            },
            [&]() {
                l_step_count += 1;
                if (l_step_count == 2)
                {
                    l_exit = 1;
                }
            },
            &l_exit);

        l_thread.free_engine_execution_unit(l_engine_eu_1);
        l_thread.sync_end_of_step([&]() {
            assert_true(l_thread.engines.Memory.is_element_free(l_engine_eu_1));
            assert_true(engine_cb_1.cleanup_called == 1);
            l_start_frame_2 = engine_cb_2.frame_count;
        });

        l_thread.sync_engine_at_end_of_frame(l_engine_eu_2, [&]() {
            assert_true(engine_cb_2.frame_count == (l_start_frame_2 + 1));
        });

        l_thread.free_engine_execution_unit_sync(l_engine_eu_2);
    }

    l_thread.free();
    l_database_path.free();
};

struct BoxCollisionSandboxEnvironment
{
    Token(Node) moving_node;
    Token(ColliderDetector) moving_node_collider_detector;
    Token(Node) static_node;
    Token(BoxColliderComponent) static_node_boxcollider_component;

    inline static BoxCollisionSandboxEnvironment build_default()
    {
        return BoxCollisionSandboxEnvironment{token_build_default(Node), token_build_default(ColliderDetector), token_build_default(Node), token_build_default(BoxColliderComponent)};
    };

    inline void step(EngineExternalStep p_step, Engine& p_engine)
    {
        switch (p_step)
        {
        case EngineExternalStep::AFTER_COLLISION:
        {
            if (FrameCount(p_engine) >= 2 && FrameCount(p_engine) <= 8)
            {
                Slice<TriggerEvent> l_0_trigger_events = p_engine.collision.get_collision_events(this->moving_node_collider_detector);

                switch (FrameCount(p_engine))
                {
                case 2:
                {
                    BoxColliderComponent l_static_node_boxcollider =
                        p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(Slice_get(&l_0_trigger_events, 0)->other, l_static_node_boxcollider.box_collider));
                    assert_true(Slice_get(&l_0_trigger_events, 0)->state == Trigger::State::TRIGGER_ENTER);
                }
                break;
                case 3:
                case 4:
                case 5:
                case 6:
                {
                    BoxColliderComponent l_static_node_boxcollider =
                        p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(Slice_get(&l_0_trigger_events, 0)->other, l_static_node_boxcollider.box_collider));
                    assert_true(Slice_get(&l_0_trigger_events, 0)->state == Trigger::State::TRIGGER_STAY);
                }
                break;
                case 7:
                {
                    BoxColliderComponent l_static_node_boxcollider =
                        p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(Slice_get(&l_0_trigger_events, 0)->other, l_static_node_boxcollider.box_collider));
                    assert_true(Slice_get(&l_0_trigger_events, 0)->state == Trigger::State::TRIGGER_EXIT);
                }
                break;
                case 8:
                {
                    BoxColliderComponent l_static_node_boxcollider =
                        p_engine.scene_middleware.collision_middleware.allocator.get_or_allocate_box_collider(p_engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(Slice_get(&l_0_trigger_events, 0)->other, l_static_node_boxcollider.box_collider));
                    assert_true(Slice_get(&l_0_trigger_events, 0)->state == Trigger::State::NONE);
                }
                break;
                }
            }
        }
        break;
        case EngineExternalStep::BEFORE_UPDATE:
        {
            if (FrameCount(p_engine) == 1)
            {
                this->moving_node = p_engine.scene.add_node(transform{v3f{0.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE}, Scene_const::root_node);
                this->static_node = p_engine.scene.add_node(transform{v3f{2.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE}, Scene_const::root_node);

                Token(BoxColliderComponent) l_node_1_box_collider_component =
                    p_engine.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(p_engine.collision, this->moving_node, BoxColliderComponentAsset{v3f_const::ONE});
                p_engine.scene.add_node_component_by_value(this->moving_node, NodeComponent::build(BoxColliderComponent::Type, token_get_value(l_node_1_box_collider_component)));
                this->moving_node_collider_detector = p_engine.scene_middleware.collision_middleware.allocator.attach_collider_detector(p_engine.collision, l_node_1_box_collider_component);

                this->static_node_boxcollider_component =
                    p_engine.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(p_engine.collision, this->static_node, BoxColliderComponentAsset{v3f_const::ONE});
                p_engine.scene.add_node_component_by_value(this->static_node, NodeComponent::build(BoxColliderComponent::Type, token_get_value(this->static_node_boxcollider_component)));
            }
            else if (FrameCount(p_engine) == 2 || FrameCount(p_engine) == 3 || FrameCount(p_engine) == 4 || FrameCount(p_engine) == 5 || FrameCount(p_engine) == 6 || FrameCount(p_engine) == 7)
            {
                NodeEntry l_node_1_value = p_engine.scene.get_node(this->moving_node);
                p_engine.scene.tree.set_localposition(l_node_1_value, p_engine.scene.tree.get_localposition(l_node_1_value) + v3f{1.0f, 0.0f, 0.0f});
            }

            if (FrameCount(p_engine) == 9)
            {
                p_engine.scene.remove_node(p_engine.scene.get_node(this->moving_node));
                p_engine.scene.remove_node(p_engine.scene.get_node(this->static_node));

                p_engine.close();
            }
        }
        break;
        default:
            break;
        }
    };
};

inline void boxcollision()
{
    String l_database_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(Slice_int8_build_rawstr("/boxcollision/asset.db"));
    {
        File l_tmp_file = File::create_or_open(l_database_path.to_slice());
        l_tmp_file.erase();
        DatabaseConnection l_conection = DatabaseConnection::allocate(l_database_path.to_slice());
        AssetDatabase::initialize_database(l_conection);
        l_conection.free();
    }
    EngineConfiguration l_condfiguration;
    l_condfiguration.asset_database_path = l_database_path.to_slice();
    l_condfiguration.headless = 1;
    Engine l_engine = SpawnEngine(l_condfiguration);

    float32 l_delta = 1.0f / 60.0f;
    BoxCollisionSandboxEnvironment l_sandbox_environment = BoxCollisionSandboxEnvironment::build_default();
    EngineRunner::main_loop_headless_forced_delta(l_engine, l_sandbox_environment, &l_delta);

    l_engine.free_headless();
    l_database_path.free();
};

namespace D3RendererCubeSandboxEnvironment_Const
{
const hash_t block_1x1_material = HashSlice(Slice_int8_build_rawstr("block_1x1_material.json"));
const hash_t block_1x1_obj = HashSlice(Slice_int8_build_rawstr("block_1x1.obj"));
} // namespace D3RendererCubeSandboxEnvironment_Const

struct D3RendererCubeSandboxEnvironment
{

    Token(Node) camera_node;
    Token(Node) l_square_root_node;

    inline static D3RendererCubeSandboxEnvironment build_default()
    {
        return D3RendererCubeSandboxEnvironment{};
    };

    inline void step(EngineExternalStep p_step, Engine& p_engine)
    {
        switch (p_step)
        {
        case EngineExternalStep::BEFORE_UPDATE:
        {
            switch (p_engine.clock.framecount)
            {
            case 1:
            {
                quat l_rot = m33f::lookat(v3f{7.0f, 7.0f, 7.0f}, v3f{0.0f, 0.0f, 0.0f}, v3f_const::UP).to_rotation();
                this->camera_node = CreateNode(p_engine, transform{v3f{7.0f, 7.0f, 7.0f}, l_rot, v3f_const::ONE});
                NodeAddCamera(p_engine, camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});

                {
                    this->l_square_root_node = p_engine.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);

                    Token(Node) l_node = CreateNode(p_engine, transform{v3f{2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{-2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{-2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{-2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode(p_engine, transform{v3f{-2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE}, this->l_square_root_node);
                    NodeAddMeshRenderer(p_engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                }
            }
                return;
            case 60:
            {
                RemoveNode(p_engine, this->camera_node);
                RemoveNode(p_engine, this->l_square_root_node);
                p_engine.close();
            }
                return;
            }

            quat l_delta_rotation = quat::rotate_around(v3f_const::UP, 45.0f * Math_const::DEG_TO_RAD * p_engine.clock.deltatime);
            NodeAddWorldRotation(p_engine, this->l_square_root_node, l_delta_rotation);
        }
        break;
        case EngineExternalStep::END_OF_FRAME:
        {
            if (p_engine.clock.framecount == 20 || p_engine.clock.framecount == 40)
            {
                String l_image_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
                l_image_path.append(Slice_int8_build_rawstr("/d3renderer_cube/frame/frame_"));
                ToString::auimax_append(p_engine.clock.framecount, l_image_path);
                l_image_path.append(Slice_int8_build_rawstr(".jpg"));

                SandboxTestUtil::render_texture_compare(p_engine, l_image_path.to_slice_with_null_termination());

#if 0
                SandboxTestUtil::render_texture_screenshot(p_engine, l_image_path.to_slice_with_null_termination());
#endif

                l_image_path.free();
            }
        }
        break;
        default:
            break;
        }
    };
};

inline void d3renderer_cube()
{
    String l_database_path = String::allocate_elements(Slice_int8_build_rawstr(ASSET_FOLDER_PATH));
    l_database_path.append(Slice_int8_build_rawstr("/d3renderer_cube/asset.db"));
    {
    }
    EngineConfiguration l_configuration{};
    l_configuration.asset_database_path = l_database_path.to_slice();
    l_configuration.render_size = v2ui{800, 600};
    l_configuration.render_target_host_readable = 1;
    Engine l_engine = SpawnEngine(l_configuration);
    float32 l_delta = 1.0f / 60.0f;

    D3RendererCubeSandboxEnvironment l_sandbox_environment = D3RendererCubeSandboxEnvironment::build_default();
    EngineRunner::main_loop_forced_delta(l_engine, l_sandbox_environment, &l_delta);

    DestroyEngine(l_engine);

    l_database_path.free();
}

int main()
{
    resize_test();
    engine_thread_test();
    boxcollision();
    d3renderer_cube();

    memleak_ckeck();
};