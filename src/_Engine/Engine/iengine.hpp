#pragma once

template <class _Engine> struct iEngine
{
    _Engine& engine;

    inline void free()
    {
        this->engine.free();
    };

    inline EngineModuleCore& get_core()
    {
        return this->engine.core;
    };

    inline Scene& get_scene()
    {
        return this->engine.scene;
    };

    inline D3RenderMiddleWare& get_render_middleware()
    {
        return this->engine.render_middleware;
    };

    inline RenderResourceAllocator2& get_render_resource_allocator()
    {
        return this->engine.renderer_resource_allocator;
    };

    inline DatabaseConnection& get_database_connection()
    {
        return this->engine.database_connection;
    };

    inline AssetDatabase& get_asset_database()
    {
        return this->engine.asset_database;
    };

    inline EWindow_Token get_window()
    {
        return this->engine.window;
    };

    inline void frame_before()
    {
        this->engine.frame_before();
    };

    inline void frame_after()
    {
        this->engine.frame_after();
    };

    inline EngineLoopState main_loop_forced_delta(const float32 p_delta)
    {
        return this->get_core().main_loop_forced_delta(p_delta);
    };

    template <class LoopFunc> inline void main_loop_forced_delta_typed(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        int8 l_running = 1;
        while (l_running)
        {
            switch (this->main_loop_forced_delta(p_delta))
            {
            case EngineLoopState::FRAME:
                this->frame_before();
                p_loop_func(this->get_core().clock.deltatime);
                this->frame_after();
                break;
            case EngineLoopState::ABORTED:
                l_running = 0;
                break;
            default:
                break;
            }
        }
    };

    inline EngineLoopState main_loop_non_blocking()
    {
        return this->get_core().main_loop_non_blocking();
    };

    template <class LoopFunc> inline void main_loop_blocking_typed(const LoopFunc& p_loop_func)
    {
        int8 l_running = 1;
        while (l_running)
        {
            switch (this->main_loop_non_blocking())
            {
            case EngineLoopState::FRAME:
            {
                this->frame_before();
                p_loop_func(this->get_core().clock.deltatime);
                this->frame_after();
            }
            break;
            case EngineLoopState::IDLE:
                this->get_core().engine_loop.block_until_next_update();
                break;
            case EngineLoopState::ABORTED:
                l_running = 0;
                break;
            }
        }
    };

    template <class LoopFunc> inline void single_frame_forced_delta_typed(const float32 p_delta, const LoopFunc& p_loop_func)
    {
        this->get_core().single_frame_forced_delta(p_delta);
        this->frame_before();
        p_loop_func(p_delta);
        this->frame_after();
    };

    inline uimax frame_count()
    {
        return this->get_core().clock.framecount;
    };

    inline float32 deltatime()
    {
        return this->get_core().clock.deltatime;
    };

    inline Node_Token create_node(const transform& p_transform, const Node_Token p_parent)
    {
        return this->get_scene().add_node(p_transform, p_parent);
    };

    inline Node_Token create_node(const transform& p_transform)
    {
        return this->get_scene().add_node(p_transform, Scene_const::root_node);
    };

    inline void remove_node(const Node_Token p_node)
    {
        Scene& l_scene = this->get_scene();
        l_scene.remove_node(l_scene.get_node(p_node));
    };

    inline v3f node_get_localposition(const Node_Token p_node)
    {
        Scene& l_scene = this->get_scene();
        return l_scene.tree.get_localposition(l_scene.get_node(p_node));
    };

    inline void node_add_worldrotation(const Node_Token p_node, const quat& p_delta_rotation)
    {
        Scene& l_scene = this->get_scene();
        NTree<Node>::Resolve l_node = l_scene.get_node(p_node);
        l_scene.tree.add_worldrotation(l_node, p_delta_rotation);
    };

    inline CameraComponent& node_add_camera(const Node_Token p_node, const CameraComponent::Asset& p_camera_asset)
    {
        D3RenderMiddleWare& l_render_middleware = this->get_render_middleware();
        Scene& l_scene = this->get_scene();
        l_render_middleware.allocate_camera_inline(p_camera_asset, p_node);
        l_scene.add_node_component_by_value(p_node, CameraComponentAsset_SceneCommunication::build_nodecomponent());
        return l_render_middleware.camera_component;
    };

    inline MeshRendererComponent::sToken node_add_meshrenderer(const Node_Token p_node, const hash_t p_material_id, const hash_t p_mesh_id)
    {
        MeshRendererComponent::sToken l_mesh_renderer = MeshRendererComponentComposition::allocate_meshrenderer_database_and_load_dependecies(
            this->get_render_middleware().meshrenderer_component_unit, this->get_render_resource_allocator(), this->get_database_connection(), this->get_asset_database(),
            MeshRendererComponent::DatabaseAllocationLoadDependenciesInput{p_material_id, p_mesh_id}, p_node);
        this->get_scene().add_node_component_by_value(p_node, MeshRendererComponentAsset_SceneCommunication::build_nodecomponent(l_mesh_renderer));
        return l_mesh_renderer;
    };

    inline void node_remove_meshrenderer(const Node_Token p_node)
    {
        this->get_scene().remove_node_component(p_node, MeshRendererComponent::Type);
    };
};