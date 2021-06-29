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

    inline void frame_before(){
        // There is no window, so nothing before
    };

    inline void frame_after()
    {
        this->scene.consume_component_events_stateful(OnComponentRemoved{this});
        EngineStepFragments::collision_step(this->collision, this->collision_middleware, this->scene);
        this->scene.step();
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

    EWindow_Token window;

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

    inline void frame_before()
    {
        this->core.abort_condition = EngineStepFragments::window_step(this->window, this->gpu_context, this->present);
    };

    inline void frame_after()
    {
        this->scene.consume_component_events_stateful(OnComponentRemoved{this});
        EngineStepFragments::render_resource_step(this->renderer.d3_renderer, this->renderer.render_targets, this->gpu_context, this->database_connection, this->asset_database,
                                                  this->renderer_resource_allocator, this->render_middleware, this->scene);
        EngineStepFragments::d3renderer_draw_present(this->core, this->gpu_context, this->renderer.d3_renderer, this->present);
        this->scene.step();
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

    inline void frame_before()
    {
        // no window
    };

    inline void frame_after()
    {
        this->scene.consume_component_events_stateful(OnComponentRemoved{this});
        EngineStepFragments::render_resource_step(this->renderer.d3_renderer, this->renderer.render_targets, this->gpu_context, this->database_connection, this->asset_database,
            this->renderer_resource_allocator, this->render_middleware, this->scene);
        EngineStepFragments::d3renderer_draw_headless(this->core, this->gpu_context, this->renderer.d3_renderer);
        this->scene.step();
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
