#pragma once

struct EngineModuleCore
{
    int8 abort_condition;
    Clock clock;
    EngineLoop engine_loop;

    struct RuntimeConfiguration
    {
        time_t target_framerate_mics;
    };

    inline static EngineModuleCore allocate(const RuntimeConfiguration& p_runtime_configuration)
    {
        EngineModuleCore l_engine_core;
        l_engine_core.abort_condition = 0;
        l_engine_core.clock = Clock::allocate_default();
        l_engine_core.engine_loop = EngineLoop::allocate_default(p_runtime_configuration.target_framerate_mics);
        return l_engine_core;
    };

    template <class LoopFunc> inline void single_frame_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        AppNativeEvent::poll_events();
        this->engine_loop.update_forced_delta(p_delta);
        this->clock.newframe();
        this->clock.newupdate(p_delta);
        p_loop_func.operator()(p_delta);
    };

    template <class LoopFunc> inline int8 single_frame(const LoopFunc& p_loop_func)
    {
        AppNativeEvent::poll_events();
        float32 l_delta;
        if (this->engine_loop.update_thread_block(&l_delta))
        {
            this->single_frame_forced_delta(l_delta, p_loop_func);
            return 1;
        };
        return 0;
    };

    template <class LoopFunc> inline void main_loop(const LoopFunc& p_loop_func)
    {
        while (!this->abort_condition)
        {
            this->single_frame(p_loop_func);
        }
    };

    template <class LoopFunc> inline void main_loop_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        while (!this->abort_condition)
        {
            this->single_frame_forced_delta(p_delta, p_loop_func);
        }
    };

    inline void close()
    {
        this->abort_condition = 1;
    };
};

struct EngineAllocationFragments
{
    inline static GPUContext gpucontext_allocate()
    {
        SliceN<GPUExtension, 1> tmp_gpu_extensions{GPUExtension::WINDOW_PRESENT};
        return GPUContext::allocate(slice_from_slicen(&tmp_gpu_extensions));
    };

    inline static GPUContext gpucontext_allocate_headless()
    {
        return GPUContext::allocate(Slice<GPUExtension>::build_default());
    };

    inline static D3Renderer d3renderer_allocate(GPUContext& p_gpu_context, const v2ui& p_render_size, const int8 p_render_target_host_readable)
    {
        ColorStep::AllocateInfo l_colorstep_allocate_info;
        l_colorstep_allocate_info.attachment_host_read = p_render_target_host_readable;
        l_colorstep_allocate_info.render_target_dimensions = v3ui{p_render_size.x, p_render_size.y, 1};
        l_colorstep_allocate_info.color_attachment_sample = 1;

        return D3Renderer::allocate(p_gpu_context, l_colorstep_allocate_info);
    };

    inline static D3Renderer d3renderer_allocate_headless(GPUContext& p_gpu_context, const v2ui& p_render_size, const int8 p_render_target_host_readable)
    {
        ColorStep::AllocateInfo l_colorstep_allocate_info;
        l_colorstep_allocate_info.attachment_host_read = p_render_target_host_readable;
        l_colorstep_allocate_info.render_target_dimensions = v3ui{p_render_size.x, p_render_size.y, 1};
        l_colorstep_allocate_info.color_attachment_sample = 0;
        return D3Renderer::allocate(p_gpu_context, l_colorstep_allocate_info);
    };

    inline static GPUPresent present_allocate(GPUContext& p_gpu_context, D3Renderer& p_renderer, const Token<Window> p_window, const v2ui& p_render_size, DatabaseConnection& p_database_connection,
                                              AssetDatabase& p_asset_database)
    {
        Span<int8> l_quad_blit_vert = p_asset_database.get_asset_blob(p_database_connection, HashSlice(slice_int8_build_rawstr("internal/quad_blit.vert")));
        Span<int8> l_quad_blit_frag = p_asset_database.get_asset_blob(p_database_connection, HashSlice(slice_int8_build_rawstr("internal/quad_blit.frag")));
        GPUPresent l_present = GPUPresent::allocate(
            p_gpu_context.instance, p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, WindowAllocator::get_window(p_window).handle, v3ui{p_render_size.x, p_render_size.y, 1},
            p_gpu_context.graphics_allocator.heap.renderpass_attachment_textures.get_vector(p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass).attachment_textures)
                .get(0),
            l_quad_blit_vert.slice, l_quad_blit_frag.slice);
        l_quad_blit_vert.free();
        l_quad_blit_frag.free();

        return l_present;
    };
};

struct EngineFreeFragments
{
    inline static void collision_free(Collision2& p_collision, CollisionMiddleware& p_collision_middleware)
    {
        p_collision_middleware.free(p_collision);
        p_collision.free();
    };

};

struct EngineStepFragments
{
    inline static int8 window_step(const Token<Window> p_window, GPUContext& p_gpu_context, GPUPresent& p_present)
    {
        Window& l_window = WindowAllocator::get_window(p_window);
        if (l_window.resize_event.ask)
        {
            l_window.consume_resize_event();
            p_present.resize(v3ui{l_window.client_width, l_window.client_height, 1}, p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator);
        }

        if (l_window.is_closing)
        {
            l_window.close();
            return 1;
        }

        return 0;
    };

    inline static void collision_step(Collision2& p_collision, CollisionMiddleware& p_collision_middleware, Scene& p_scene)
    {
        p_collision_middleware.step(p_collision, &p_scene);
        p_collision.step();
    };

    inline static void render_resource_step(D3Renderer& p_renderer, GPUContext& p_gpu_context, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database,
                                            RenderResourceAllocator2& p_render_resource_allocator, RenderMiddleWare& p_render_middleware, Scene& p_scene)
    {
        p_render_middleware.meshrenderer_component_unit.deallocation_step(p_renderer, p_gpu_context, p_render_resource_allocator);
        p_render_resource_allocator.deallocation_step(p_renderer, p_gpu_context);
        p_render_resource_allocator.allocation_step(p_renderer, p_gpu_context, p_database_connection, p_asset_database);
        p_render_middleware.meshrenderer_component_unit.allocation_step(p_renderer, p_gpu_context, p_render_resource_allocator, p_asset_database);

        p_render_middleware.step(p_renderer, p_gpu_context, &p_scene);
    };

    inline static void d3renderer_draw_present(EngineModuleCore& p_core, GPUContext& p_gpu_context, D3Renderer& p_renderer, GPUPresent& p_present)
    {
        p_renderer.buffer_step(p_gpu_context, p_core.clock.totaltime);
        p_gpu_context.buffer_step_and_submit();
        GraphicsBinder l_graphics_binder = p_gpu_context.creates_graphics_binder();
        l_graphics_binder.start();
        p_renderer.graphics_step(l_graphics_binder);
        p_present.graphics_step(l_graphics_binder);
        l_graphics_binder.end();
        p_gpu_context.submit_graphics_binder_and_notity_end(l_graphics_binder);
        p_present.present(p_gpu_context.graphics_end_semaphore);
        p_gpu_context.wait_for_completion();
    };

    inline static void d3renderer_draw_headless(EngineModuleCore& p_core, GPUContext& p_gpu_context, D3Renderer& p_renderer)
    {
        p_renderer.buffer_step(p_gpu_context, p_core.clock.totaltime);
        p_gpu_context.buffer_step_and_submit();
        GraphicsBinder l_graphics_binder = p_gpu_context.creates_graphics_binder();
        l_graphics_binder.start();
        p_renderer.graphics_step(l_graphics_binder);
        l_graphics_binder.end();
        p_gpu_context.submit_graphics_binder(l_graphics_binder);
        p_gpu_context.wait_for_completion();
    };
};

#define EgineModule_RendererComponents_on_removed(p_render_middleware, p_render_resource_allocator, p_node_component)                                                                                  \
    case MeshRendererComponent::Type:                                                                                                                                                                  \
    {                                                                                                                                                                                                  \
        MeshRendererComponentAsset_SceneCommunication::on_node_component_removed(p_render_middleware, p_render_resource_allocator, p_node_component);                                                  \
    }                                                                                                                                                                                                  \
    break;                                                                                                                                                                                             \
    case CameraComponent::Type:                                                                                                                                                                        \
    {                                                                                                                                                                                                  \
        CameraComponentAsset_SceneCommunication::on_node_component_removed(p_render_middleware, p_node_component);                                                                                     \
    }                                                                                                                                                                                                  \
    break;

#define EngineModuleCollision_BoxColliderComponent_on_removed(p_collision_middleware, p_collision, p_node_component)                                                                                   \
    case BoxColliderComponent::Type:                                                                                                                                                                   \
    {                                                                                                                                                                                                  \
        BoxColliderComponentAsset_SceneCommunication::on_node_component_removed(p_collision_middleware, p_collision, p_node_component);                                                                \
    }                                                                                                                                                                                                  \
    break;