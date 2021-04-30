#pragma once

struct EngineAPI_Internal
{
    inline static uimax FrameCount(EngineModuleCore& p_core)
    {
        return p_core.clock.framecount;
    };

    inline static float32 DeltaTime(EngineModuleCore& p_core)
    {
        return p_core.clock.deltatime;
    };

    inline static Token<Node> CreateNode(Scene& p_scene, const transform& p_transform, const Token<Node> p_parent)
    {
        return p_scene.add_node(p_transform, p_parent);
    };

    inline static Token<Node> CreateNode(Scene& p_scene, const transform& p_transform)
    {
        return p_scene.add_node(p_transform, Scene_const::root_node);
    };

    inline static void RemoveNode(Scene& p_scene, const Token<Node> p_node)
    {
        p_scene.remove_node(p_scene.get_node(p_node));
    };

    inline static void NodeAddWorldRotation(Scene& p_scene, const Token<Node> p_node, const quat& p_delta_rotation)
    {
        NTree<Node>::Resolve l_node = p_scene.get_node(p_node);
        p_scene.tree.add_worldrotation(l_node, p_delta_rotation);
    };

    inline static CameraComponent& NodeAddCamera(const Token<Node> p_node, const CameraComponent::Asset& p_camera_asset, RenderMiddleWare& p_render_middleware, Scene& p_scene)
    {
        p_render_middleware.allocate_camera_inline(p_camera_asset, p_node);
        p_scene.add_node_component_by_value(p_node, CameraComponentAsset_SceneCommunication::build_nodecomponent());
        return p_render_middleware.camera_component;
    };

    inline Token<MeshRendererComponent> static NodeAddMeshRenderer(const Token<Node> p_node, const hash_t p_material_id, const hash_t p_mesh_id, RenderMiddleWare& p_render_middleware,
                                                                   RenderResourceAllocator2& p_render_resource_allocator, DatabaseConnection& p_database_connection, AssetDatabase& p_asset_database,
                                                                   Scene& p_scene)
    {
        Token<MeshRendererComponent> l_mesh_renderer = MeshRendererComponentComposition::allocate_meshrenderer_database_and_load_dependecies(
            p_render_middleware.meshrenderer_component_unit, p_render_resource_allocator, p_database_connection, p_asset_database,
            MeshRendererComponent::DatabaseAllocationLoadDependenciesInput{p_material_id, p_mesh_id}, p_node);
        p_scene.add_node_component_by_value(p_node, MeshRendererComponentAsset_SceneCommunication::build_nodecomponent(l_mesh_renderer));
        return l_mesh_renderer;
    };

    inline static void NodeRemoveMeshRenderer(Scene& p_scene, const Token<Node> p_node)
    {
        p_scene.remove_node_component(p_node, MeshRendererComponent::Type);
    };
};

template <class EngineType> inline uimax FrameCount(EngineType& p_engine)
{
    return EngineAPI_Internal::FrameCount(p_engine.core);
};

template <class EngineType> inline float32 DeltaTime(EngineType& p_engine)
{
    return EngineAPI_Internal::DeltaTime(p_engine.core);
};

template <class EngineType> inline Token<Node> CreateNode(EngineType& p_engine, const transform& p_transform, const Token<Node> p_parent)
{
    return EngineAPI_Internal::CreateNode(p_engine.scene, p_transform, p_parent);
};

template <class EngineType> inline Token<Node> CreateNode(EngineType& p_engine, const transform& p_transform)
{
    return EngineAPI_Internal::CreateNode(p_engine.scene, p_transform);
};

template <class EngineType> inline void RemoveNode(EngineType& p_engine, const Token<Node> p_node)
{
    EngineAPI_Internal::RemoveNode(p_engine.scene, p_node);
};

template <class EngineType> inline void NodeAddWorldRotation(EngineType& p_engine, const Token<Node> p_node, const quat& p_delta_rotation)
{
    EngineAPI_Internal::NodeAddWorldRotation(p_engine.scene, p_node, p_delta_rotation);
};

template <class EngineType> inline CameraComponent& NodeAddCamera(EngineType& p_engine, const Token<Node> p_node, const CameraComponent::Asset& p_camera_asset)
{
    return EngineAPI_Internal::NodeAddCamera(p_node, p_camera_asset, p_engine.render_middleware, p_engine.scene);
};

template <class EngineType> inline Token<MeshRendererComponent> NodeAddMeshRenderer(EngineType& p_engine, const Token<Node> p_node, const hash_t p_material_id, const hash_t p_mesh_id)
{
    return EngineAPI_Internal::NodeAddMeshRenderer(p_node, p_material_id, p_mesh_id, p_engine.render_middleware, p_engine.renderer_resource_allocator, p_engine.database_connection,
                                                   p_engine.asset_database, p_engine.scene);
};

template <class EngineType> inline void NodeRemoveMeshRenderer(EngineType& p_engine, const Token<Node> p_node)
{
    EngineAPI_Internal::NodeRemoveMeshRenderer(p_engine.scene, p_node);
};