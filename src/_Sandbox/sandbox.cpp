

#include "./Sandbox/sandbox.hpp"
#include "AssetCompiler/asset_compiler.hpp"

struct SandboxTestUtil
{
    inline static void render_texture_compare(GPUContext& p_gpu_context, D3Renderer& p_renderer, const Slice<int8>& p_compared_image_path)
    {
        ImageFormat l_rendertarget_texture_format;
        Token<BufferHost> l_rendertarget_texture = GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost_with_imageformat(
            p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass), 0, &l_rendertarget_texture_format);

        p_gpu_context.buffer_step_force_execution();
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

        p_gpu_context.buffer_step_force_execution();
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

    l_engine.single_frame_forced_delta(0.1f, [](auto) {
    });
    WindowNative::simulate_resize_appevent(g_app_windows.get(l_engine.window).handle, 500, 500);
    l_engine.single_frame_forced_delta(0.1f, [](auto) {
    });
    assert_true(g_app_windows.get(l_engine.window).client_width == 500);
    assert_true(g_app_windows.get(l_engine.window).client_height == 500);
    l_engine.core.close();
    l_engine.single_frame_forced_delta(0.1f, [](auto) {
    });

    l_engine.free();
    l_database_path.free();
};

inline void engine_thread_test()
{
    String l_database_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("/thread_test/asset.db"));

    {
        struct engine_thread
        {
            struct Shared
            {
                volatile uimax frame_count;
                volatile int8 cleanup_called;
            } shared;

            Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present engine;
            EngineThreadSync engine_synchronization;

            Slice<int8> database_slice;
            thread_t thread;

            struct Exec
            {
                engine_thread* thiz;
                inline int8 operator()() const
                {
                    thiz->shared.frame_count = 0;
                    thiz->shared.cleanup_called = 0;
                    thiz->engine = Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::allocate(
                        Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration{EngineModuleCore::RuntimeConfiguration{1000000 / 60}, thiz->database_slice, v2ui{50, 50}, 0});

                    int8 l_running = 1;
                    while (l_running)
                    {
                        switch (thiz->engine.main_loop_forced_delta(0.01f))
                        {
                        case EngineLoopState::FRAME:
                        {
                            thiz->engine.frame_before();

                            thiz->engine_synchronization.on_end_of_frame();
                            thiz->shared.frame_count = FrameCount(thiz->engine);
                            thiz->engine_synchronization.on_start_of_frame();
                            if (thiz->engine_synchronization.ask_exit)
                            {
                                thiz->engine.core.close();
                            }

                            thiz->engine.frame_after();
                        }
                        break;
                        case EngineLoopState::ABORTED:
                            l_running = 0;
                            break;
                        default:
                            break;
                        }
                    }

                    thiz->engine.free();
                    thiz->shared.cleanup_called = 1;

                    return 0;
                };
            } exec;

            inline void start()
            {
                this->exec = Exec{this};
                this->thread = Thread::spawn_thread(this->exec);
            };

            inline void free()
            {
                this->engine_synchronization.exit();
                Thread::wait_for_end_and_terminate(this->thread, -1);
            };
        };

        engine_thread l_engine_thread;
        l_engine_thread.database_slice = l_database_path.to_slice();
        l_engine_thread.start();
        l_engine_thread.engine_synchronization.sync_end_of_frame([&]() {
            assert_true(l_engine_thread.shared.frame_count == 0);
            assert_true(l_engine_thread.shared.cleanup_called == 0);
        });
        l_engine_thread.engine_synchronization.sync_end_of_frame([&]() {
            assert_true(l_engine_thread.shared.frame_count == 1);
            assert_true(l_engine_thread.shared.cleanup_called == 0);
        });
        l_engine_thread.free();
        assert_true(l_engine_thread.shared.cleanup_called == 1);
    }

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
        int8 l_running = 1;
        while (l_running)
        {
            l_running = this->engine.main_loop_forced_delta_v2(p_delta, [&](const float32 p_delta) {
                if (FrameCount(this->engine) == 1)
                {
                    this->moving_node = CreateNode(this->engine, transform{v3f{0.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});
                    this->static_node = CreateNode(this->engine, transform{v3f{2.0f, 1.0f, 0.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3});

                    Token<BoxColliderComponent> l_node_1_box_collider_component =
                        this->engine.collision_middleware.allocator.allocate_box_collider_component(this->engine.collision, this->moving_node, BoxColliderComponentAsset{v3f_const::ONE.vec3});
                    this->engine.scene.add_node_component_by_value(this->moving_node, NodeComponent::build(BoxColliderComponent::Type, token_value(l_node_1_box_collider_component)));
                    this->moving_node_collider_detector = this->engine.collision_middleware.allocator.attach_collider_detector(this->engine.collision, l_node_1_box_collider_component);

                    this->static_node_boxcollider_component =
                        this->engine.collision_middleware.allocator.allocate_box_collider_component(this->engine.collision, this->static_node, BoxColliderComponentAsset{v3f_const::ONE.vec3});
                    this->engine.scene.add_node_component_by_value(this->static_node, NodeComponent::build(BoxColliderComponent::Type, token_value(this->static_node_boxcollider_component)));
                }
                else if (FrameCount(this->engine) == 2)
                {
                    Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                    BoxColliderComponent l_static_node_boxcollider =
                        this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                    assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_ENTER);

                    this->update_node();
                }
                else if (FrameCount(this->engine) == 3 || FrameCount(this->engine) == 4 || FrameCount(this->engine) == 5)
                {
                    this->update_node();
                }
                else if (FrameCount(this->engine) == 6)
                {
                    Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                    BoxColliderComponent l_static_node_boxcollider =
                        this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                    assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_STAY);

                    this->update_node();
                }
                else if (FrameCount(this->engine) == 7)
                {
                    Slice<TriggerEvent> l_0_trigger_events = this->engine.collision.get_collision_events(this->moving_node_collider_detector);
                    BoxColliderComponent l_static_node_boxcollider =
                        this->engine.collision_middleware.allocator.get_or_allocate_box_collider(this->engine.collision, this->static_node_boxcollider_component);
                    assert_true(l_0_trigger_events.Size == 1);
                    assert_true(token_equals(l_0_trigger_events.get(0).other, l_static_node_boxcollider.box_collider));
                    assert_true(l_0_trigger_events.get(0).state == Trigger::State::TRIGGER_EXIT);

                    this->update_node();
                }
                else if (FrameCount(this->engine) == 8)
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
        }

        RemoveNode(this->engine, this->moving_node);
        RemoveNode(this->engine, this->static_node);
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
const hash_t block_1x1_material = HashFunctions::hash(slice_int8_build_rawstr("block_1x1_material.json"));
const hash_t block_1x1_obj = HashFunctions::hash(slice_int8_build_rawstr("block_1x1.obj"));
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
        int8 l_running = 1;
        while (l_running)
        {

            switch (this->engine.main_loop_forced_delta(p_forced_delta))
            {
            case EngineLoopState::FRAME:
                this->engine.frame_before();
                _frame_function();
                this->engine.frame_after();
                break;
            case EngineLoopState::ABORTED:
                l_running = 0;
                break;
            default:
                break;
            }
        }
    };

  private:
    inline void _frame_function()
    {
        uimax l_frame_count = FrameCount(this->engine);
        if (l_frame_count == 1)
        {
            quat l_rot = m33f::lookat(v3f{7.0f, 7.0f, 7.0f}, v3f{0.0f, 0.0f, 0.0f}, v3f_const::UP).to_rotation();
            this->camera_node = CreateNode(this->engine, transform{v3f{7.0f, 7.0f, 7.0f}, l_rot, v3f_const::ONE.vec3});
            NodeAddCamera(this->engine, camera_node, CameraComponent::Asset{1.0f, 30.0f, 45.0f});

            {
                this->l_square_root_node = CreateNode(this->engine, transform_const::ORIGIN);

                Token<Node> l_node = CreateNode(this->engine, transform{v3f{2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{-2.0f, 2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{-2.0f, -2.0f, 2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{-2.0f, 2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);

                l_node = CreateNode(this->engine, transform{v3f{-2.0f, -2.0f, -2.0f}, quat_const::IDENTITY, v3f_const::ONE.vec3}, this->l_square_root_node);
                NodeAddMeshRenderer(this->engine, l_node, D3RendererCubeSandboxEnvironment_Const::block_1x1_material, D3RendererCubeSandboxEnvironment_Const::block_1x1_obj);
            }
            return;
        }

        if (l_frame_count == 60)
        {
            RemoveNode(this->engine, this->camera_node);
            RemoveNode(this->engine, this->l_square_root_node);
            this->engine.core.close();
        }

        if (l_frame_count == 21 || l_frame_count == 41)
        {
            String l_image_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("d3renderer_cube/frame/frame_"));
            ToString::auimax_append(FrameCount(this->engine) - 1, l_image_path);
            l_image_path.append(slice_int8_build_rawstr(".jpg"));

            SandboxTestUtil::render_texture_compare(this->engine.gpu_context, this->engine.renderer.d3_renderer, l_image_path.to_slice_with_null_termination());

#if 0
            SandboxTestUtil::render_texture_screenshot(this->engine.gpu_context, this->engine.renderer.d3_renderer, l_image_path.to_slice_with_null_termination());
#endif

            l_image_path.free();
        }
        quat l_delta_rotation = quat::rotate_around(v3f_const::UP, 45.0f * Math_const::DEG_TO_RAD * DeltaTime(this->engine));
        NodeAddWorldRotation(this->engine, this->l_square_root_node, l_delta_rotation);
    }
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

#if 0
struct ProceduralMeshEnvironment
{
    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present engine;
    inline static ProceduralMeshEnvironment allocate(const Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration& p_configuration)
    {
        return ProceduralMeshEnvironment{Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::allocate(p_configuration)};
    };

    // TODO -> having a bufferhostvector object. That behaves like vector but allocates memory in host gpu instead
    struct MeshProcedural
    {
        Token<Mesh> mesh;
        Vector<Vertex> vertices;
        Vector<uint32> indices;
        Token<BufferHost> vertices_source_buffer;
        Token<BufferHost> indices_source_buffer;

        inline static MeshProcedural allocate_default(GPUContext& p_gpu_context, const Token<Mesh> p_mesh)
        {
            MeshProcedural l_return;
            l_return.mesh = p_mesh;
            l_return.vertices = Vector<Vertex>::allocate(1);
            l_return.vertices_source_buffer = p_gpu_context.buffer_memory.allocator.allocate_bufferhost(l_return.vertices.Memory.slice.build_asint8(), BufferUsageFlag::TRANSFER_READ);
            l_return.indices = Vector<uint32>::allocate(1);
            l_return.indices_source_buffer = p_gpu_context.buffer_memory.allocator.allocate_bufferhost(l_return.indices.Memory.slice.build_asint8(), BufferUsageFlag::TRANSFER_READ);

            return l_return;
        };

        inline void free(GPUContext& p_gpu_context)
        {
            this->vertices.free();
            this->indices.free();

            BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, this->vertices_source_buffer);
            BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, this->indices_source_buffer);
        };

        inline void push_to_gpu(GPUContext& p_gpu_context, D3Renderer& p_rendenrer)
        {
            Mesh& l_mesh = p_rendenrer.heap().meshes.get(this->mesh);
            ;
            // TOOD -> add a write to BufferHost ?
            {
                Slice<int8> l_mesh_source_vertex_memory = p_gpu_context.buffer_memory.allocator.host_buffers.get(this->vertices_source_buffer).get_mapped_effective_memory();
                l_mesh_source_vertex_memory.copy_memory(this->vertices.to_slice().build_asint8());
                BufferReadWrite::write_to_buffergpu_no_allocation(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, l_mesh.vertices_buffer, this->vertices_source_buffer);
            }
            {
                Slice<int8> l_indices_source_indices_memory = p_gpu_context.buffer_memory.allocator.host_buffers.get(this->indices_source_buffer).get_mapped_effective_memory();
                l_indices_source_indices_memory.copy_memory(this->vertices.to_slice().build_asint8());
                BufferReadWrite::write_to_buffergpu_no_allocation(p_gpu_context.buffer_memory.allocator, p_gpu_context.buffer_memory.events, l_mesh.vertices_buffer, this->vertices_source_buffer);
            }
        };
    };

    Token<MeshResource> mesh_resource;
    MeshProcedural mesh_procedural;

    inline void main(const float32 p_forced_delta)
    {

        this->engine.main_loop_forced_delta(p_forced_delta, [&](const float32 p_delta) {
            uimax l_frame = FrameCount(this->engine);
            if (l_frame == 1)
            {
                SliceN<Vertex, 4> l_vertices_arr = {Vertex{v3f{0.0f, 0.0f, 0.0f}, v2f{0.0f, 0.0f}}, Vertex{v3f{1.0f, 0.0f, 0.0f}, v2f{0.0f, 0.0f}}, Vertex{v3f{0.0f, 1.0f, 0.0f}, v2f{0.0f, 0.0f}},
                                                    Vertex{v3f{0.0f, 0.0f, 1.0f}, v2f{0.0f, 0.0f}}};
                SliceN<uint32, 6> l_indices_arr = {0, 1, 2, 1, 2, 3};
                MeshResource::Asset::Value l_mesh_value;
                l_mesh_value.initial_indices = slice_from_slicen(&l_indices_arr);
                l_mesh_value.initial_vertices = slice_from_slicen(&l_vertices_arr);

                MeshResource::InlineAllocationInput l_mesh_allocation;
                l_mesh_allocation.id = 985;
                l_mesh_allocation.asset = MeshResource::Asset::allocate_from_values(l_mesh_value);

                // TODO -> having something like forced allocation ?
                //         no, this is not necessary
                this->mesh_resource = this->engine.renderer_resource_allocator.mesh_unit.allocate_or_increment_inline(l_mesh_allocation);

                // this->mesh_source_buffer = this->engine.gpu_context.buffer_memory.allocator.allocate_bufferhost(this->procedural_vertices.slice.build_asint8(), BufferUsageFlag::TRANSFER_READ);
            }
            else
            {
                if (l_frame == 2)
                {
                    MeshResource& l_mesh_resource = this->engine.renderer_resource_allocator.mesh_unit.meshes.pool.get(this->mesh_resource);
                    this->mesh_procedural = MeshProcedural::allocate_default(this->engine.gpu_context, l_mesh_resource.resource);
                    // Mesh& l_mesh = this->engine.renderer.d3_renderer.heap().meshes.get(l_mesh_resource.resource);
                    // this->mesh_vertex_buffer = l_mesh.vertices_buffer;
                }

                this->mesh_procedural.push_to_gpu(this->engine.gpu_context, this->engine.renderer.d3_renderer);
            }
        });

        this->engine.renderer_resource_allocator.mesh_unit.decrement_or_release(this->mesh_resource);
    };
};

inline void procedural_mesh()
{
    String l_database_path = String::allocate_elements_2(slice_int8_build_rawstr(ASSET_FOLDER_PATH), slice_int8_build_rawstr("/d3renderer_cube/asset.db"));
    {
    }

    Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present::RuntimeConfiguration l_configuration{};
    l_configuration.core = EngineModuleCore::RuntimeConfiguration{0};
    l_configuration.render_target_host_readable = 1;
    l_configuration.render_size = v2ui{800, 600};
    l_configuration.database_path = l_database_path.to_slice();

    ProceduralMeshEnvironment l_sandbox_environment = ProceduralMeshEnvironment::allocate(l_configuration);
    l_sandbox_environment.main(1.0f / 60.0f);

    l_database_path.free();
};
#endif

int main()
{
    resize_test();
    engine_thread_test();
    boxcollision();
    d3renderer_cube();

    memleak_ckeck();
};
