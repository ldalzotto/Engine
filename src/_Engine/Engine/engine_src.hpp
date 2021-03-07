#pragma once

struct EngineConfiguration
{
    Slice<int8> asset_database_path;
    int8 headless;
    v2ui render_size;
};

struct Engine
{
    int8 abort_condition;

    Clock clock;
    EngineLoop engine_loop;

    Collision2 collision;
    v2::GPUContext gpu_context;
    v2::D3Renderer renderer;

    Token(Window) window;
    v2::GPUPresent present;

    v2::RenderRessourceAllocator2 renderer_ressource_allocator;

    v2::Scene scene;
    v2::SceneMiddleware scene_middleware;

    AssetDatabase asset_database;

    inline static Engine allocate(const EngineConfiguration& p_configuration)
    {
        Engine l_engine;
        l_engine.abort_condition = 0;
        l_engine.clock = Clock::allocate_default();
        l_engine.engine_loop = EngineLoop::allocate_default(1000000 / 60);
        l_engine.collision = Collision2::allocate();
        l_engine.gpu_context = v2::GPUContext::allocate(SliceN<v2::GPUExtension, 1>{v2::GPUExtension::WINDOW_PRESENT}.to_slice());
        l_engine.renderer = v2::D3Renderer::allocate(l_engine.gpu_context, v2::ColorStep::AllocateInfo{v3ui{8, 8, 1}, 0, 1});
        l_engine.renderer_ressource_allocator = v2::RenderRessourceAllocator2::allocate();
        l_engine.scene = v2::Scene::allocate_default();
        l_engine.scene_middleware = v2::SceneMiddleware::allocate_default();
        l_engine.asset_database = AssetDatabase::allocate(p_configuration.asset_database_path);

        if (!p_configuration.headless)
        {
            l_engine.window = WindowAllocator::allocate(p_configuration.render_size.x, p_configuration.render_size.y, slice_int8_build_rawstr(""));
            Span<int8> l_quad_blit_vert = l_engine.asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("~internal/quad_blit.vert")));
            Span<int8> l_quad_blit_frag = l_engine.asset_database.get_asset_blob(HashSlice(slice_int8_build_rawstr("~internal/quad_blit.frag")));
            l_engine.present = v2::GPUPresent::allocate(l_engine.gpu_context.instance, l_engine.gpu_context.buffer_memory, l_engine.gpu_context.graphics_allocator,
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

    void free();

    void free_headless();

    inline void close()
    {
        this->abort_condition = 1;
    };

    template <class ExternalCallbacksFn> void main_loop(ExternalCallbacksFn& p_callbacks);

    template <class ExternalCallbacksFn> void single_frame_forced_delta(const float32 p_delta, ExternalCallbacksFn& p_callbacks);

    template <class ExternalCallbacksFn> void single_frame_forced_delta_headless(const float32 p_delta, ExternalCallbacksFn& p_callbacks);

    template <class ExternalCallbacksFn> void single_frame(ExternalCallbacksFn& p_callbacks);
};

struct Engine_ComponentReleaser
{
    Engine& engine;

    inline void on_component_removed(v2::Scene* p_scene, const v2::NodeEntry& p_node, const v2::NodeComponent& p_component)
    {
        g_on_node_component_removed(&this->engine.scene_middleware, this->engine.collision, this->engine.renderer, this->engine.gpu_context, this->engine.renderer_ressource_allocator, p_component);
    };
};

inline void Engine::free()
{
    Engine_ComponentReleaser l_component_releaser = Engine_ComponentReleaser{*this};
    this->scene.consume_component_events_stateful(l_component_releaser);
    this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_context, this->renderer_ressource_allocator, this->asset_database);
    this->asset_database.free();
    this->collision.free();
    this->renderer_ressource_allocator.free(this->renderer, this->gpu_context);
    this->renderer.free(this->gpu_context);
    this->present.free(this->gpu_context.instance, this->gpu_context.buffer_memory, this->gpu_context.graphics_allocator);
    this->gpu_context.free();
    WindowAllocator::free(this->window);
    this->scene.free();
};

inline void Engine::free_headless()
{
    Engine_ComponentReleaser l_component_releaser = Engine_ComponentReleaser{*this};
    this->scene.consume_component_events_stateful(l_component_releaser);
    this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_context, this->renderer_ressource_allocator, this->asset_database);
    this->asset_database.free();
    this->collision.free();
    this->renderer_ressource_allocator.free(this->renderer, this->gpu_context);
    this->renderer.free(this->gpu_context);
    this->gpu_context.free();
    this->scene.free();
};

template <class ExternalCallbacksFn> inline void Engine::main_loop(ExternalCallbacksFn& p_callbacks)
{
    while (!this->abort_condition)
    {
        this->single_frame(p_callbacks);
    }
    WindowAllocator::get_window(this->window).close();
};

struct EngineLoopFunctions
{
    inline static void new_frame(Engine& p_engine)
    {
        p_engine.clock.newframe();

        Window& l_window = WindowAllocator::get_window(p_engine.window);
        if (l_window.resize_event.ask)
        {
            l_window.consume_resize_event();
            p_engine.present.resize(v3ui{l_window.client_width, l_window.client_height, 1}, p_engine.gpu_context.buffer_memory, p_engine.gpu_context.graphics_allocator);
            p_engine.abort_condition = 1;
        }
        if(l_window.is_closing)
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

    template <class ExternalCallbacksFn> inline static void update(Engine& p_engine, const float32 p_delta, ExternalCallbacksFn& p_external_callbacks)
    {
        p_engine.clock.newupdate(p_delta);

        p_external_callbacks.before_collision(p_engine);

        p_engine.collision.step();

        p_external_callbacks.after_collision(p_engine);
        p_external_callbacks.before_update(p_engine);

        p_engine.scene_middleware.deallocation_step(p_engine.renderer, p_engine.gpu_context, p_engine.renderer_ressource_allocator);
        p_engine.renderer_ressource_allocator.deallocation_step(p_engine.renderer, p_engine.gpu_context);
        p_engine.renderer_ressource_allocator.allocation_step(p_engine.renderer, p_engine.gpu_context, p_engine.asset_database);
        p_engine.scene_middleware.allocation_step(p_engine.renderer, p_engine.gpu_context, p_engine.renderer_ressource_allocator, p_engine.asset_database);

        p_engine.scene_middleware.step(&p_engine.scene, p_engine.collision, p_engine.renderer, p_engine.gpu_context);
    };

    inline static void render(Engine& p_engine)
    {
        p_engine.renderer.buffer_step(p_engine.gpu_context);
        p_engine.gpu_context.buffer_step_and_submit();
        v2::GraphicsBinder l_graphics_binder = p_engine.gpu_context.creates_graphics_binder();
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
        v2::GraphicsBinder l_graphics_binder = p_engine.gpu_context.creates_graphics_binder();
        p_engine.renderer.graphics_step(l_graphics_binder);
        p_engine.gpu_context.submit_graphics_binder(l_graphics_binder);
        p_engine.gpu_context.wait_for_completion();
    };

    inline static void end_of_frame(Engine& p_engine)
    {
        Engine_ComponentReleaser l_component_releaser = Engine_ComponentReleaser{p_engine};
        p_engine.scene.consume_component_events_stateful(l_component_releaser);
        p_engine.scene.step();
    };
};

template <class ExternalCallbacksFn> inline void Engine::single_frame_forced_delta(const float32 p_delta, ExternalCallbacksFn& p_callbacks)
{
    AppNativeEvent::poll_events();
    if (this->engine_loop.update_forced_delta(p_delta))
    {
        EngineLoopFunctions::new_frame(*this);
        EngineLoopFunctions::update(*this, p_delta, p_callbacks);
        EngineLoopFunctions::render(*this);
        EngineLoopFunctions::end_of_frame(*this);
    }
};

template <class ExternalCallbacksFn> inline void Engine::single_frame_forced_delta_headless(const float32 p_delta, ExternalCallbacksFn& p_callbacks)
{
    AppNativeEvent::poll_events();
    if (this->engine_loop.update_forced_delta(p_delta))
    {
        EngineLoopFunctions::new_frame_headless(*this);
        EngineLoopFunctions::update(*this, p_delta, p_callbacks);
        EngineLoopFunctions::render_headless(*this);
        EngineLoopFunctions::end_of_frame(*this);
    }
};

template <class ExternalCallbacksFn> inline void Engine::single_frame(ExternalCallbacksFn& p_callbacks)
{
    AppNativeEvent::poll_events();
    float32 l_delta;
    if (this->engine_loop.update(&l_delta))
    {
        this->single_frame_forced_delta(l_delta, p_callbacks);
    };
};
