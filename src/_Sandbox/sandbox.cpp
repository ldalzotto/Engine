

#include "./Sandbox/sandbox.hpp"
#include "AssetCompiler/asset_compiler.hpp"

struct SandboxTestUtil
{
    inline static void render_texture_compare(GPUContext& p_gpu_context, D3Renderer& p_renderer, const Slice<int8>& p_compared_image_path)
    {
        ImageFormat l_rendertarget_texture_format;
        Token<BufferHost> l_rendertarget_texture = GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost_with_imageformat(
            p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass), 0, &l_rendertarget_texture_format);

        p_gpu_context.buffer_step_and_wait_for_completion();
        Slice<int8> l_rendertarget_texture_value = p_gpu_context.buffer_memory.allocator.host_buffers.get(l_rendertarget_texture).get_mapped_effective_memory();

        Span<int8> l_image = ImgCompiler::read_image(p_compared_image_path);
        assert_true(l_rendertarget_texture_value.compare(l_image.slice));
        l_image.free();

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, l_rendertarget_texture);
    };

    inline static void render_texture_screenshot(GPUContext& p_gpu_context, D3Renderer& p_renderer, const Slice<int8>& p_path)
    {
        ImageFormat l_rendertarget_texture_format;
        Token<BufferHost> l_rendertarget_texture = GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost_with_imageformat(
            p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass), 0, &l_rendertarget_texture_format);

        p_gpu_context.buffer_step_and_wait_for_completion();
        Slice<int8> l_rendertarget_texture_value = p_gpu_context.buffer_memory.allocator.host_buffers.get(l_rendertarget_texture).get_mapped_effective_memory();

        ImgCompiler::write_to_image(p_path, l_rendertarget_texture_format.extent.x, l_rendertarget_texture_format.extent.y, 4, l_rendertarget_texture_value);

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, l_rendertarget_texture);
    };
};

inline void resize_test()
{
    String l_database_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("/resize_test/asset.db"));
    {
    }
    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration l_configuration;
    l_configuration.core = EngineModuleCore::RuntimeConfiguration{1000000 / 60};
    l_configuration.database_path = l_database_path.to_slice();
    l_configuration.render_size = v2ui{400, 400};
    l_configuration.render_target_host_readable = 1;

    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present l_engine = Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::allocate(l_configuration);

    l_engine.single_frame_forced_delta(0.1f, [](auto){});
    WindowNative::simulate_resize_appevent(g_app_windows.get(l_engine.window).handle, 500, 500);
    l_engine.single_frame_forced_delta(0.1f, [](auto){});
    assert_true(g_app_windows.get(l_engine.window).client_width == 500);
    assert_true(g_app_windows.get(l_engine.window).client_height == 500);
    l_engine.core.close();
    l_engine.single_frame_forced_delta(0.1f, [](auto){});

    l_engine.free();
    l_database_path.free();
};

inline void engine_thread_test()
{
    EngineRunnerThread l_thread = EngineRunnerThread::allocate();
    l_thread.start();

    String l_database_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("/thread_test/asset.db"));

    struct s_engine_cb
    {
        uimax frame_count;
        int8 cleanup_called;
        inline static void step(EngineExternalStep p_step, Engine& p_engine, s_engine_cb* thiz)
        {
            thiz->frame_count = FrameCount(p_engine);
        };

        inline static void cleanup_resources(Engine& p_engine, s_engine_cb* thiz)
        {
            thiz->cleanup_called = 1;
        };

        inline EngineExternalStepCallback build_external_step_callback()
        {
            return EngineExternalStepCallback{this, (EngineExternalStepCallback::cb_t)s_engine_cb::step};
        };

        inline EngineExecutionUnit::CleanupCallback build_cleanup_callback()
        {
            return EngineExecutionUnit::CleanupCallback{this, (EngineExecutionUnit::CleanupCallback::cb_t)s_engine_cb::cleanup_resources};
        };
    };

    // only one engine
    {
        s_engine_cb engine_cb{};
        Token<EngineExecutionUnit> l_engine_eu_1 =
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
        Token<EngineExecutionUnit> l_engine_eu_1 =
            l_thread.allocate_engine_execution_unit(l_database_path.to_slice(), 50, 50, engine_cb_1.build_external_step_callback(), engine_cb_1.build_cleanup_callback());
        Token<EngineExecutionUnit> l_engine_eu_2 =
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

struct BoxCollisionSandboxEnvironmentV3
{
    Engine_Scene_Collision engine;

    Token<Node> moving_node;
    Token<ColliderDetector> moving_node_collider_detector;
    Token<Node> static_node;
    Token<BoxColliderComponent> static_node_boxcollider_component;

    inline static BoxCollisionSandboxEnvironmentV3 allocate_default()
    {
        return BoxCollisionSandboxEnvironmentV3{Engine_Scene_Collision::allocate(EngineModuleCore::RuntimeConfiguration{0}), token_build_default<Node>(), token_build_default<ColliderDetector>(),
                                                token_build_default<Node>(), token_build_default<BoxColliderComponent>()};
    };

    inline void free()
    {
        this->engine.free();
    };

    inline void main(const float32 p_delta)
    {
        this->engine.main_loop_forced_delta(p_delta, [&](const float32 p_delta) {
            if (FrameCount_v2(this->engine) == 1)
            {
                this->moving_node = CreateNode_v2(this->engine, transform{v3f{0.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});
                this->static_node = CreateNode_v2(this->engine, transform{v3f{2.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});

                Token<BoxColliderComponent> l_node_1_box_collider_component =
                    this->engine.collision_middleware.allocator.allocate_box_collider_component(this->engine.collision, this->moving_node, BoxColliderComponentAsset{v3f_const::ONE.vec3});
                this->engine.scene.add_node_component_by_value(this->moving_node, NodeComponent::build(BoxColliderComponent::Type, token_value(l_node_1_box_collider_component)));
                this->moving_node_collider_detector = this->engine.collision_middleware.allocator.attach_collider_detector(this->engine.collision, l_node_1_box_collider_component);

                this->static_node_boxcollider_component =
                    this->engine.collision_middleware.allocator.allocate_box_collider_component(this->engine.collision, this->static_node, BoxColliderComponentAsset{v3f_const::ONE.vec3});
                this->engine.scene.add_node_component_by_value(this->static_node, NodeComponent::build(BoxColliderComponent::Type, token_value(this->static_node_boxcollider_component)));
            }
            else if (FrameCount_v2(this->engine) == 2)
            {
                Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                BoxColliderComponent l_static_node_boxcollider =
                    this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_ENTER);

                this->update_node();
            }
            else if (FrameCount_v2(this->engine) == 3 || FrameCount_v2(this->engine) == 4 || FrameCount_v2(this->engine) == 5)
            {
                this->update_node();
            }
            else if (FrameCount_v2(this->engine) == 6)
            {
                Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                BoxColliderComponent l_static_node_boxcollider =
                    this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_STAY);

                this->update_node();
            }
            else if (FrameCount_v2(this->engine) == 7)
            {
                Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                BoxColliderComponent l_static_node_boxcollider =
                    this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_EXIT);

                this->update_node();
            }
            else if (FrameCount_v2(this->engine) == 8)
            {
                Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                BoxColliderComponent l_static_node_boxcollider =
                    this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                assert_true(l_0_trigger_events.Size == 1);
                assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                assert_true(l_0_trigger_events.get(0).state == Trigger::State::NONE);

                this->engine.core.close();
            }
        });

        RemoveNode_v2(this->engine, this->moving_node);
        RemoveNode_v2(this->engine, this->static_node);
    };

    inline void update_node()
    {
        NodeEntry l_node_1_value = this->engine.scene.get_node(this->moving_node);
        this->engine.scene.tree.set_localposition(l_node_1_value, this->engine.scene.tree.get_localposition(l_node_1_value) + v3f{1.0f, 0.0f, 0.0f});
    }
};

inline void boxcollision()
{
    BoxCollisionSandboxEnvironmentV3 l_sandbox_environment = BoxCollisionSandboxEnvironmentV3::allocate_default();
    l_sandbox_environment.main(1.0f / 60.0f);
    l_sandbox_environment.free();
};

namespace D3RendererCubeSandboxEnvironment_Const
{
const hash_t block_1x1_material = HashSlice(slice_int8_build_rawstr("block_1x1_material.json"));
const hash_t block_1x1_obj = HashSlice(slice_int8_build_rawstr("block_1x1.obj"));
} // namespace D3RendererCubeSandboxEnvironment_Const

struct D3RendererCubeSandboxEnvironmentV2
{
    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present engine;
    Token<Node> camera_node;
    Token<Node> l_square_root_node;

    inline static D3RendererCubeSandboxEnvironmentV2 allocate(const Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration& p_configuration)
    {
        return D3RendererCubeSandboxEnvironmentV2{Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::allocate(p_configuration)};
    };

    inline void free()
    {
        this->engine.free();
    };

    inline void main(float32 p_forced_delta)
    {
        this->engine.main_loop_forced_delta(p_forced_delta, [&](const float32 p_delta) {
            uimax l_frame_count = FrameCount_v2(this->engine);
            if (l_frame_count == 1)
            {

                quat l_rot = m33f::lookat(v3f{7.0f, 7.0f, 7.0f}, v3f{0.0f, 0.0f, 0.0f}, v3f_const::UP).to_rotation();
                this->camera_node = CreateNode_v2(this->engine, transform{v3f{7.0f, 7.0f, 7.0f}, l_rot, v3f_const::ONE.vec3});
                NodeAddCamera_v2(this->engine, camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});

                {
                    this->l_square_root_node = CreateNode_v2(this->engine, transform_const::ORIGIN);

                    Token<Node> l_node = CreateNode_v2(this->engine, transform{v3f{2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{-2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{-2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{-2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                    l_node = CreateNode_v2(this->engine, transform{v3f{-2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                    NodeAddMeshRenderer_v2(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
                }
                return;
            }

            if (l_frame_count == 60)
            {
                RemoveNode_v2(this->engine, this->camera_node);
                RemoveNode_v2(this->engine, this->l_square_root_node);
                this->engine.core.close();
            }

            if (l_frame_count == 21 || l_frame_count == 41)
            {
                String l_image_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("d3renderer_cube/frame/frame_"));
                ToString::auimax_append(FrameCount_v2(this->engine) - 1, l_image_path);
                l_image_path.append(slice_int8_build_rawstr(".jpg"));

                SandboxTestUtil::render_texture_compare(this->engine.gpu_context, this->engine.renderer, l_image_path.to_slice_with_null_termination());

#if 0
                SandboxTestUtil::render_texture_screenshot(this->engine.gpu_context, this->engine.renderer, l_image_path.to_slice_with_null_termination());
#endif

                l_image_path.free();
            }

            quat l_delta_rotation = quat::rotate_around(v3f_const::UP, 45.0f * Math_const::DEG_TO_RAD * DeltaTime_v2(this->engine));
            NodeAddWorldRotation_v2(this->engine, this->l_square_root_node, l_delta_rotation);
        });
    };
};

inline void d3renderer_cube()
{
    String l_database_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("/d3renderer_cube/asset.db"));
    {
    }

    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration l_configuration{};
    l_configuration.core = EngineModuleCore::RuntimeConfiguration{0};
    l_configuration.render_target_host_readable = 1;
    l_configuration.render_size = v2ui{800, 600};
    l_configuration.database_path = l_database_path.to_slice();

    D3RendererCubeSandboxEnvironmentV2 l_sandbox_environment = D3RendererCubeSandboxEnvironmentV2::allocate(l_configuration);
    l_sandbox_environment.main(1.0f / 60.0f);
    l_sandbox_environment.free();

    l_database_path.free();
};

int main()
{
    resize_test();
    engine_thread_test();
    boxcollision();
    d3renderer_cube();
    memleak_ckeck();
};
