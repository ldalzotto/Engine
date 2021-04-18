#pragma once

inline Engine SpawnEngine(const EngineConfiguration& p_configuration)
{
    return Engine::allocate(p_configuration);
};

inline void DestroyEngine(Engine& p_engine)
{
    p_engine.free();
};

inline uimax FrameCount(Engine& p_engine)
{
    return p_engine.clock.framecount;
};

inline float32 DeltaTime(Engine& p_engine)
{
    return p_engine.clock.deltatime;
};

inline Token<Node> CreateNode(Engine& p_engine, const transform& p_transform, const Token<Node> p_parent)
{
    return p_engine.scene.add_node(p_transform, p_parent);
};

inline Token<Node> CreateNode(Engine& p_engine, const transform& p_transform)
{
    return p_engine.scene.add_node(p_transform, Scene_const::root_node);
};

inline void RemoveNode(Engine& p_engine, const Token<Node> p_node)
{
    p_engine.scene.remove_node(p_engine.scene.get_node(p_node));
};

inline void NodeAddWorldRotation(Engine& p_engine, const Token<Node> p_node, const quat& p_delta_rotation)
{
    NTree<Node>::Resolve l_node = p_engine.scene.get_node(p_node);
    p_engine.scene.tree.add_worldrotation(l_node, p_delta_rotation);
};

inline CameraComponent& NodeAddCamera(Engine& p_engine, const Token<Node> p_node, const CameraComponent::Asset& p_camera_asset)
{
    p_engine.scene_middleware.render_middleware.allocate_camera_inline(p_camera_asset, p_node);
    p_engine.scene.add_node_component_by_value(p_node, CameraComponentAsset_SceneCommunication::build_nodecomponent());
    return p_engine.scene_middleware.render_middleware.camera_component;
};

inline Token<MeshRendererComponent> NodeAddMeshRenderer(Engine& p_engine, const Token<Node> p_node, const hash_t p_material_id, const hash_t p_mesh_id)
{
    Token<MeshRendererComponent> l_mesh_renderer = MeshRendererComponentComposition::allocate_meshrenderer_database_and_load_dependecies(
        p_engine.scene_middleware.render_middleware.meshrenderer_component_unit, p_engine.renderer_resource_allocator, p_engine.database_connection, p_engine.asset_database,
        MeshRendererComponent::DatabaseAllocationLoadDependenciesInput{p_material_id, p_mesh_id}, p_node);
    p_engine.scene.add_node_component_by_value(p_node, MeshRendererComponentAsset_SceneCommunication::build_nodecomponent(l_mesh_renderer));
    return l_mesh_renderer;
};

inline void NodeRemoveMeshRenderer(Engine& p_engine, const Token<Node> p_node)
{
    p_engine.scene.remove_node_component(p_node, MeshRendererComponent::Type);
};

inline Token<Material> MeshRenderer_GetMaterial(Engine& p_engine, const Token<MeshRendererComponent> p_mesh_renderer)
{
    p_engine.renderer_resource_allocator.material_unit.materials.pool.get(
        p_engine.scene_middleware.render_middleware.meshrenderer_component_unit.mesh_renderers.get(p_mesh_renderer).dependencies.material);
};

template <class ElementType> inline ElementType& Material_GetParameter(Engine& p_engine, const Token<Material> p_material, const uimax p_index)
{
    return p_engine.renderer.heap()
        .materials.get(p_material)
        .template get_buffer_host_parameter_memory_typed<ElementType>(p_engine.gpu_context.graphics_allocator, p_engine.gpu_context.buffer_memory.allocator, p_index);
};
