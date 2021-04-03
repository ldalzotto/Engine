#pragma once

struct EngineConfiguration
{
    Slice<int8> asset_database_path;
    int8 headless;
    v2ui render_size;
    int8 render_target_host_readable;
};

enum class EngineExternalStep : int8
{
    BEFORE_COLLISION = 0,
    AFTER_COLLISION = BEFORE_COLLISION + 1,
    BEFORE_UPDATE = AFTER_COLLISION + 1,
    END_OF_FRAME = BEFORE_UPDATE + 1
};

struct Engine
{
    int8 abort_condition;

    Clock clock;
    EngineLoop engine_loop;

    Collision2 collision;
    GPUContext gpu_context;
    D3Renderer renderer;

    Token(Window) window;
    GPUPresent present;

    RenderRessourceAllocator2 renderer_ressource_allocator;

    Scene scene;
    SceneMiddleware scene_middleware;

    DatabaseConnection database_connection;
    AssetDatabase asset_database;

    struct ComponentReleaser
    {
        Engine& engine;

        inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component)
        {
            g_on_node_component_removed(&this->engine.scene_middleware, this->engine.collision, this->engine.renderer, this->engine.gpu_context, this->engine.renderer_ressource_allocator,
                                        p_component);
        };
    };

    inline static Engine allocate(const EngineConfiguration& p_configuration)
    {
        Engine l_engine;
        l_engine.abort_condition = 0;
        l_engine.clock = Clock::allocate_default();
        l_engine.engine_loop = EngineLoop::allocate_default(1000000 / 60);
        l_engine.collision = Collision2::allocate();
        SliceN<GPUExtension, 1> tmp_gpu_extensions{GPUExtension::WINDOW_PRESENT};
        l_engine.gpu_context = GPUContext::allocate(slice_from_slicen(&tmp_gpu_extensions));
        l_engine.renderer_ressource_allocator = RenderRessourceAllocator2::allocate();
        l_engine.scene = Scene::allocate_default();
        l_engine.scene_middleware = SceneMiddleware::allocate_default();
        l_engine.database_connection = DatabaseConnection::allocate(p_configuration.asset_database_path);
        l_engine.asset_database = AssetDatabase::allocate(l_engine.database_connection);

        ColorStep::AllocateInfo l_colorstep_allocate_info{};
        l_colorstep_allocate_info.attachment_host_read = p_configuration.render_target_host_readable;
        if (!p_configuration.headless)
        {
            l_colorstep_allocate_info.render_target_dimensions = v3ui{p_configuration.render_size.x, p_configuration.render_size.y, 1};
            l_colorstep_allocate_info.color_attachment_sample = 1;
        }
        else
        {
            l_colorstep_allocate_info.render_target_dimensions = v3ui{8, 8, 1};
            l_colorstep_allocate_info.color_attachment_sample = 0;
        }

        l_engine.renderer = D3Renderer::allocate(l_engine.gpu_context, l_colorstep_allocate_info);

        if (!p_configuration.headless)
        {
            l_engine.window = WindowAllocator::allocate(p_configuration.render_size.x, p_configuration.render_size.y, slice_int8_build_rawstr(""));
            Span<int8> l_quad_blit_vert = l_engine.asset_database.get_asset_blob(l_engine.database_connection, HashSlice(slice_int8_build_rawstr("internal/quad_blit.vert")));
            Span<int8> l_quad_blit_frag = l_engine.asset_database.get_asset_blob(l_engine.database_connection, HashSlice(slice_int8_build_rawstr("internal/quad_blit.frag")));
            l_engine.present = GPUPresent::allocate(l_engine.gpu_context.instance, l_engine.gpu_context.buffer_memory, l_engine.gpu_context.graphics_allocator,
                                                    WindowAllocator::get_window(l_engine.window).handle, v3ui{p_configuration.render_size.x, p_configuration.render_size.y, 1},
                                                    l_engine.gpu_context.graphics_allocator.heap.renderpass_attachment_textures
                                                        .get_vector(l_engine.gpu_context.graphics_allocator.heap.graphics_pass.get(l_engine.renderer.color_step.pass).attachment_textures)
                                                        .get(0),
                                                    l_quad_blit_vert.slice, l_quad_blit_frag.slice);
            l_quad_blit_vert.free();
            l_quad_blit_frag.free();
        }
        else
        {
            l_engine.window = tk_bd(Window);
            l_engine.present = {0};
        }

        return l_engine;
    };

    inline void free()
    {
        ComponentReleaser l_component_releaser = ComponentReleaser{*this};
        this->scene.consume_component_events_stateful(l_component_releaser);
        this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_context, this->renderer_ressource_allocator, this->asset_database);
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        this->collision.free();
        this->renderer_ressource_allocator.free(this->renderer, this->gpu_context);
        this->renderer.free(this->gpu_context);
        this->present.free(this->gpu_context.instance, this->gpu_context.buffer_memory, this->gpu_context.graphics_allocator);
        this->gpu_context.free();
        WindowAllocator::free(this->window);
        this->scene.free();
    };

    inline void free_headless()
    {
        ComponentReleaser l_component_releaser = ComponentReleaser{*this};
        this->scene.consume_component_events_stateful(l_component_releaser);
        this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_context, this->renderer_ressource_allocator, this->asset_database);
        this->asset_database.free(this->database_connection);
        this->database_connection.free();
        this->collision.free();
        this->renderer_ressource_allocator.free(this->renderer, this->gpu_context);
        this->renderer.free(this->gpu_context);
        this->gpu_context.free();
        this->scene.free();
    };

    inline void close()
    {
        this->abort_condition = 1;
    };
};

struct EngineExternalStepCallback
{
    void* closure;
    typedef void (*cb_t)(EngineExternalStep, Engine&, void*);
    cb_t cb;
    inline void step(EngineExternalStep p_step, Engine& p_engine)
    {
        this->cb(p_step, p_engine, this->closure);
    };
};

struct EngineLoopFunctions
{
    template <class ExternalCallbackStep> inline static void new_frame(Engine& p_engine, ExternalCallbackStep& p_callback_step)
    {
        p_engine.clock.newframe();

        Window& l_window = WindowAllocator::get_window(p_engine.window);
        if (l_window.resize_event.ask)
        {
            l_window.consume_resize_event();
            p_engine.present.resize(v3ui{l_window.client_width, l_window.client_height, 1}, p_engine.gpu_context.buffer_memory, p_engine.gpu_context.graphics_allocator);
        }
        if (l_window.is_closing)
        {
            l_window.close();
            p_engine.abort_condition = 1;
        }
        // TODO -> input
    };

    inline static void new_frame_headless(Engine& p_engine)
    {
        p_engine.clock.newframe();
    };

    template <class ExternalCallbackStep> inline static void update(Engine& p_engine, const float32 p_delta, ExternalCallbackStep& p_callback_step)
    {
        p_engine.clock.newupdate(p_delta);

        p_callback_step.step(EngineExternalStep::BEFORE_COLLISION, p_engine);

        p_engine.collision.step();

        p_callback_step.step(EngineExternalStep::AFTER_COLLISION, p_engine);
        p_callback_step.step(EngineExternalStep::BEFORE_UPDATE, p_engine);

        p_engine.scene_middleware.deallocation_step(p_engine.renderer, p_engine.gpu_context, p_engine.renderer_ressource_allocator);
        p_engine.renderer_ressource_allocator.deallocation_step(p_engine.renderer, p_engine.gpu_context);
        p_engine.renderer_ressource_allocator.allocation_step(p_engine.renderer, p_engine.gpu_context, p_engine.database_connection, p_engine.asset_database);
        p_engine.scene_middleware.allocation_step(p_engine.renderer, p_engine.gpu_context, p_engine.renderer_ressource_allocator, p_engine.asset_database);

        p_engine.scene_middleware.step(&p_engine.scene, p_engine.collision, p_engine.renderer, p_engine.gpu_context);
    };

    inline static void render(Engine& p_engine)
    {
        p_engine.renderer.buffer_step(p_engine.gpu_context);
        p_engine.gpu_context.buffer_step_and_submit();
        GraphicsBinder l_graphics_binder = p_engine.gpu_context.creates_graphics_binder();
        l_graphics_binder.start();
        p_engine.renderer.graphics_step(l_graphics_binder);
        p_engine.present.graphics_step(l_graphics_binder);
        l_graphics_binder.end();
        p_engine.gpu_context.submit_graphics_binder_and_notity_end(l_graphics_binder);
        p_engine.present.present(p_engine.gpu_context.graphics_end_semaphore);
        p_engine.gpu_context.wait_for_completion();
    };

    inline static void render_headless(Engine& p_engine)
    {
        p_engine.renderer.buffer_step(p_engine.gpu_context);
        p_engine.gpu_context.buffer_step_and_submit();
        GraphicsBinder l_graphics_binder = p_engine.gpu_context.creates_graphics_binder();
        p_engine.renderer.graphics_step(l_graphics_binder);
        p_engine.gpu_context.submit_graphics_binder(l_graphics_binder);
        p_engine.gpu_context.wait_for_completion();
    };

    template <class ExternalCallbackStep> inline static void end_of_frame(Engine& p_engine, ExternalCallbackStep& p_callback_step)
    {
        Engine::ComponentReleaser l_component_releaser = Engine::ComponentReleaser{p_engine};
        p_engine.scene.consume_component_events_stateful(l_component_releaser);
        p_engine.scene.step();
        p_callback_step.step(EngineExternalStep::END_OF_FRAME, p_engine);
    };
};

struct EngineRunner
{

    template <class ExternalCallbackStep> inline static void single_frame_forced_delta(Engine& p_engine, const float32 p_delta, ExternalCallbackStep& p_callback_step)
    {
        AppNativeEvent::poll_events();
        p_engine.engine_loop.update_forced_delta(p_delta);
        EngineLoopFunctions::new_frame(p_engine, p_callback_step);
        EngineLoopFunctions::update(p_engine, p_delta, p_callback_step);
        EngineLoopFunctions::render(p_engine);
        EngineLoopFunctions::end_of_frame(p_engine, p_callback_step);
    };

    template <class ExternalCallbackStep> inline static void single_frame_forced_delta_headless(Engine& p_engine, const float32 p_delta, ExternalCallbackStep& p_callback_step)
    {
        AppNativeEvent::poll_events();
        p_engine.engine_loop.update_forced_delta(p_delta);
        EngineLoopFunctions::new_frame_headless(p_engine);
        EngineLoopFunctions::update(p_engine, p_delta, p_callback_step);
        EngineLoopFunctions::render_headless(p_engine);
        EngineLoopFunctions::end_of_frame(p_engine, p_callback_step);
    };

    template <class ExternalCallbackStep> inline static int8 single_frame_no_block(Engine& p_engine, ExternalCallbackStep& p_callback_step)
    {
        AppNativeEvent::poll_events();
        float32 l_delta;
        if (p_engine.engine_loop.update(&l_delta))
        {
            EngineRunner::single_frame_forced_delta(p_engine, l_delta, p_callback_step);
            return 1;
        };
        return 0;
    };

    template <class ExternalCallbackStep> inline static int8 single_frame(Engine& p_engine, ExternalCallbackStep& p_callback_step)
    {
        AppNativeEvent::poll_events();
        float32 l_delta;
        if (p_engine.engine_loop.update_thread_block(&l_delta))
        {
            EngineRunner::single_frame_forced_delta(p_engine, l_delta, p_callback_step);
            return 1;
        };
        return 0;
    };

    template <class ExternalCallbackStep> inline static void main_loop(Engine& p_engine, ExternalCallbackStep& p_callback_step)
    {
        while (!p_engine.abort_condition)
        {
            EngineRunner::single_frame(p_engine, p_callback_step);
        }
    };

    template <class ExternalCallbackStep> inline static void main_loop_forced_delta(Engine& p_engine, ExternalCallbackStep& p_callback_step, float32* in_delta)
    {
        while (!p_engine.abort_condition)
        {
            EngineRunner::single_frame_forced_delta(p_engine, *in_delta, p_callback_step);
        }
    };

    template <class ExternalCallbackStep> inline static void main_loop_headless_forced_delta(Engine& p_engine, ExternalCallbackStep& p_callback_step, float32* in_delta)
    {
        while (!p_engine.abort_condition)
        {
            EngineRunner::single_frame_forced_delta_headless(p_engine, *in_delta, p_callback_step);
        }
    };
};