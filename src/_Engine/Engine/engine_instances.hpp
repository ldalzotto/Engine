#pragma once

struct Engine_Scene_Collision
{
    EngineModuleCore core;
    Scene scene;

    Collision2 collision;
    CollisionMiddleware collision_middleware;

    struct OnComponentRemoved
    {
        Engine_Scene_Collision* thiz;

        inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component) const
        {
            switch (p_component.type)
            {
                EngineModuleCollision_BoxColliderComponent_on_removed(thiz->collision_middleware, thiz->collision, p_component);
            default:
                abort();
            }
        };
    };

    inline static Engine_Scene_Collision allocate(const EngineModuleCore::RuntimeConfiguration& p_core_configuration)
    {
        Engine_Scene_Collision l_return;
        l_return.core = EngineModuleCore::allocate(p_core_configuration);
        l_return.scene = Scene::allocate_default();
        l_return.collision = Collision2::allocate();
        l_return.collision_middleware = CollisionMiddleware::allocate_default();
        return l_return;
    };

    template <class LoopFunc> inline void main_loop_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->core.main_loop_forced_delta_no_event_poll(p_delta, [&](const float32 p_delta) {
            p_loop_func(p_delta);
            this->scene.consume_component_events_stateful(OnComponentRemoved{this});
            EngineStepFragments::collision_step(this->collision, this->collision_middleware, this->scene);
            this->scene.step();
        });
    };

    inline void free()
    {
        this->scene.free_and_consume_component_events_stateful(OnComponentRemoved{this});

        EngineFreeFragments::collision_free(this->collision, this->collision_middleware);
    };
};

struct Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present
{
    EngineModuleCore core;

    Scene scene;

    GPUContext gpu_context;

    DatabaseConnection database_connection;
    AssetDatabase asset_database;

    RenderResourceAllocator2 renderer_resource_allocator;
    D3RenderMiddleWare render_middleware;
    Renderer_3D renderer;

    Token<EWindow> window;

    GPUPresent present;

    struct OnComponentRemoved
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present* thiz;

        inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component) const
        {
            switch (p_component.type)
            {
                EgineModule_RendererComponents_on_removed(thiz->render_middleware, thiz->renderer_resource_allocator, p_component);
            default:
                abort();
            }
        };
    };

    struct RuntimeConfiguration
    {
        EngineModuleCore::RuntimeConfiguration core;
        Slice<int8> database_path;
        v2ui render_size;
        int8 render_target_host_readable;
    };

    inline static Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present allocate(const RuntimeConfiguration& p_runtime_configuration)
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present l_return;
        l_return.core = EngineModuleCore::allocate(p_runtime_configuration.core);
        l_return.scene = Scene::allocate_default();

        l_return.gpu_context = EngineAllocationFragments::gpucontext_allocate();
        l_return.database_connection = DatabaseConnection::allocate(p_runtime_configuration.database_path);
        l_return.asset_database = AssetDatabase::allocate(l_return.database_connection);
        l_return.renderer_resource_allocator = RenderResourceAllocator2::allocate();
        l_return.render_middleware = D3RenderMiddleWare::allocate();
        l_return.renderer = EngineAllocationFragments::d3renderer_allocate(l_return.gpu_context, p_runtime_configuration.render_size, p_runtime_configuration.render_target_host_readable);
        l_return.window = WindowAllocator::allocate(p_runtime_configuration.render_size.x, p_runtime_configuration.render_size.y, slice_int8_build_rawstr(""));
        l_return.present = EngineAllocationFragments::present_allocate(l_return.gpu_context, l_return.renderer.render_targets, l_return.window, p_runtime_configuration.render_size,
                                                                       l_return.database_connection, l_return.asset_database);

        return l_return;
    };

    template <class LoopFunc> struct UpdateFunc
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer_Window_Present* thiz;
        const LoopFunc& loop_func;
        inline void operator()(const float32 p_delta) const
        {
            thiz->core.abort_condition = EngineStepFragments::window_step(thiz->window, thiz->gpu_context, thiz->present);
            loop_func(p_delta);
            thiz->scene.consume_component_events_stateful(OnComponentRemoved{thiz});
            EngineStepFragments::render_resource_step(thiz->renderer.d3_renderer, thiz->renderer.render_targets, thiz->gpu_context, thiz->database_connection, thiz->asset_database,
                                                      thiz->renderer_resource_allocator, thiz->render_middleware, thiz->scene);
            EngineStepFragments::d3renderer_draw_present(thiz->core, thiz->gpu_context, thiz->renderer.d3_renderer, thiz->present);
            thiz->scene.step();
        };
    };

    template <class LoopFunc> inline void main_loop(const LoopFunc& p_loop_func)
    {
        this->core.main_loop(UpdateFunc<LoopFunc>{this, p_loop_func});
    };

    template <class LoopFunc> inline void main_loop_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->core.main_loop_forced_delta(p_delta, UpdateFunc<LoopFunc>{this, p_loop_func});
    };

    template <class LoopFunc> inline void single_frame_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->core.single_frame_forced_delta(p_delta, UpdateFunc<LoopFunc>{this, p_loop_func});
    };

    inline void free()
    {
        this->scene.free_and_consume_component_events_stateful(OnComponentRemoved{this});

        this->render_middleware.free(this->renderer.d3_renderer, this->gpu_context, this->asset_database, this->renderer_resource_allocator);
        this->renderer_resource_allocator.free(this->renderer.d3_renderer, this->gpu_context);
        this->renderer.free(this->gpu_context);

        this->asset_database.free(this->database_connection);
        this->database_connection.free();

        this->present.free(this->gpu_context.instance, this->gpu_context.buffer_memory, this->gpu_context.graphics_allocator);
        WindowAllocator::free(this->window);
        this->gpu_context.free();
    };
};

struct Engine_Scene_GPU_AssetDatabase_D3Renderer_Imgui_Window_Present
{
    EngineModuleCore core;

    Scene scene;

    GPUContext gpu_context;

    DatabaseConnection database_connection;
    AssetDatabase asset_database;

    RenderResourceAllocator2 renderer_resource_allocator;
    D3RenderMiddleWare render_middleware;
    Renderer_3D_imgui renderer;

    Token<EWindow> window;

    GPUPresent present;

    struct OnComponentRemoved
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer_Imgui_Window_Present* thiz;

        inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component) const
        {
            switch (p_component.type)
            {
                EgineModule_RendererComponents_on_removed(thiz->render_middleware, thiz->renderer_resource_allocator, p_component);
            default:
                abort();
            }
        };
    };

    struct RuntimeConfiguration
    {
        EngineModuleCore::RuntimeConfiguration core;
        Slice<int8> database_path;
        v2ui render_size;
        int8 render_target_host_readable;
    };

    inline static Engine_Scene_GPU_AssetDatabase_D3Renderer_Imgui_Window_Present allocate(const RuntimeConfiguration& p_runtime_configuration)
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer_Imgui_Window_Present l_return;
        l_return.core = EngineModuleCore::allocate(p_runtime_configuration.core);
        l_return.scene = Scene::allocate_default();

        l_return.gpu_context = EngineAllocationFragments::gpucontext_allocate();
        l_return.database_connection = DatabaseConnection::allocate(p_runtime_configuration.database_path);
        l_return.asset_database = AssetDatabase::allocate(l_return.database_connection);
        l_return.renderer_resource_allocator = RenderResourceAllocator2::allocate();
        l_return.render_middleware = D3RenderMiddleWare::allocate();
        l_return.renderer = EngineAllocationFragments::d3renderer_imgui_allocate(l_return.gpu_context, p_runtime_configuration.render_size, p_runtime_configuration.render_target_host_readable);
        l_return.window = WindowAllocator::allocate(p_runtime_configuration.render_size.x, p_runtime_configuration.render_size.y, slice_int8_build_rawstr(""));
        l_return.present = EngineAllocationFragments::present_allocate(l_return.gpu_context, l_return.renderer.render_targets, l_return.window, p_runtime_configuration.render_size,
                                                                       l_return.database_connection, l_return.asset_database);

        return l_return;
    };

    template <class LoopFunc, class GuiFunc> struct UpdateFunc
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer_Imgui_Window_Present* thiz;
        const LoopFunc& loop_func;
        const GuiFunc& gui_func;
        inline void operator()(const float32 p_delta) const
        {
            thiz->core.abort_condition = EngineStepFragments::window_step(thiz->window, thiz->gpu_context, thiz->present);
            loop_func(p_delta);
            thiz->scene.consume_component_events_stateful(OnComponentRemoved{thiz});
            EngineStepFragments::render_resource_step(thiz->renderer.d3_renderer, thiz->renderer.render_targets, thiz->gpu_context, thiz->database_connection, thiz->asset_database,
                                                      thiz->renderer_resource_allocator, thiz->render_middleware, thiz->scene);
            EngineStepFragments::d3renderer_imgui_draw_present(thiz->core, thiz->gpu_context, thiz->renderer.d3_renderer, thiz->renderer.imgui_renderer, thiz->renderer.render_targets, thiz->present,
                                                               this->gui_func);
            thiz->scene.step();
        };
    };

    template <class LoopFunc, class GuiFunc> inline void main_loop(const LoopFunc& p_loop_func, const GuiFunc& p_gui_func)
    {
        this->core.main_loop(UpdateFunc<LoopFunc, GuiFunc>{this, p_loop_func, p_gui_func});
    };

    template <class LoopFunc, class GuiFunc> inline void main_loop_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func, const GuiFunc& p_gui_func)
    {
        this->core.main_loop_forced_delta(p_delta, UpdateFunc<LoopFunc, GuiFunc>{this, p_loop_func, p_gui_func});
    };

    template <class LoopFunc, class GuiFunc> inline void single_frame_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func, const GuiFunc& p_gui_func)
    {
        this->core.single_frame_forced_delta(p_delta, UpdateFunc<LoopFunc, GuiFunc>{this, p_loop_func, p_gui_func});
    };

    inline void free()
    {
        this->scene.free_and_consume_component_events_stateful(OnComponentRemoved{this});

        this->render_middleware.free(this->renderer.d3_renderer, this->gpu_context, this->asset_database, this->renderer_resource_allocator);
        this->renderer_resource_allocator.free(this->renderer.d3_renderer, this->gpu_context);
        this->renderer.free(this->gpu_context);

        this->asset_database.free(this->database_connection);
        this->database_connection.free();

        this->present.free(this->gpu_context.instance, this->gpu_context.buffer_memory, this->gpu_context.graphics_allocator);
        this->gpu_context.free();
        WindowAllocator::free(this->window);
    };
};

struct Engine_Scene_GPU_AssetDatabase_D3Renderer
{
    EngineModuleCore core;

    Scene scene;

    GPUContext gpu_context;

    DatabaseConnection database_connection;
    AssetDatabase asset_database;

    RenderResourceAllocator2 renderer_resource_allocator;
    D3RenderMiddleWare render_middleware;
    Renderer_3D renderer;

    struct OnComponentRemoved
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer* thiz;

        inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component) const
        {
            switch (p_component.type)
            {
                EgineModule_RendererComponents_on_removed(thiz->render_middleware, thiz->renderer_resource_allocator, p_component);
            default:
                abort();
            }
        };
    };

    struct RuntimeConfiguration
    {
        EngineModuleCore::RuntimeConfiguration core;
        Slice<int8> database_path;
        v2ui render_size;
        int8 render_target_host_readable;
    };

    inline static Engine_Scene_GPU_AssetDatabase_D3Renderer allocate(const RuntimeConfiguration& p_runtime_configuration)
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer l_return;
        l_return.core = EngineModuleCore::allocate(p_runtime_configuration.core);
        l_return.scene = Scene::allocate_default();

        l_return.gpu_context = EngineAllocationFragments::gpucontext_allocate();
        l_return.database_connection = DatabaseConnection::allocate(p_runtime_configuration.database_path);
        l_return.asset_database = AssetDatabase::allocate(l_return.database_connection);
        l_return.renderer_resource_allocator = RenderResourceAllocator2::allocate();
        l_return.render_middleware = D3RenderMiddleWare::allocate();
        l_return.renderer = EngineAllocationFragments::d3renderer_allocate_headless(l_return.gpu_context, p_runtime_configuration.render_size, p_runtime_configuration.render_target_host_readable);

        return l_return;
    };

    template <class LoopFunc> struct UpdateFunc
    {
        Engine_Scene_GPU_AssetDatabase_D3Renderer* thiz;
        const LoopFunc& loop_func;
        inline void operator()(const float32 p_delta) const
        {
            loop_func(p_delta);
            thiz->scene.consume_component_events_stateful(OnComponentRemoved{thiz});
            EngineStepFragments::render_resource_step(thiz->renderer.d3_renderer, thiz->renderer.render_targets, thiz->gpu_context, thiz->database_connection, thiz->asset_database,
                                                      thiz->renderer_resource_allocator, thiz->render_middleware, thiz->scene);
            EngineStepFragments::d3renderer_draw_headless(thiz->core, thiz->gpu_context, thiz->renderer.d3_renderer);
            thiz->scene.step();
        };
    };

    template <class LoopFunc> inline void main_loop_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->core.main_loop_forced_delta_no_event_poll(p_delta, UpdateFunc<LoopFunc>{this, p_loop_func});
    };

    template <class LoopFunc> inline void single_frame_forced_delta(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->core.single_frame_forced_delta_no_event_poll(p_delta, UpdateFunc<LoopFunc>{this, p_loop_func});
    };

    inline void free()
    {
        this->scene.free_and_consume_component_events_stateful(OnComponentRemoved{this});

        this->render_middleware.free(this->renderer.d3_renderer, this->gpu_context, this->asset_database, this->renderer_resource_allocator);
        this->renderer_resource_allocator.free(this->renderer.d3_renderer, this->gpu_context);
        this->renderer.free(this->gpu_context);

        this->asset_database.free(this->database_connection);
        this->database_connection.free();

        this->gpu_context.free();
    };
};
