#pragma once

namespace BoxColliderComponentAsset_SceneCommunication
{
inline static NodeComponent build_nodecomponent(const Token<BoxColliderComponent> p_component)
{
    return NodeComponent{BoxColliderComponent::Type, token_value(p_component)};
};

inline static BoxColliderComponentAsset desconstruct_nodecomponent(CollisionMiddleware& p_collision_middleware, Collision2& p_collision, const NodeComponent& p_node_component)
{
    return BoxColliderComponentAsset{p_collision_middleware.allocator.box_collider_get_world_half_extend(p_collision, token_build<BoxColliderComponent>(p_node_component.resource))};
};

inline static void on_node_component_removed(CollisionMiddleware& p_collision_middleware, Collision2& p_collision, const NodeComponent& p_node_component)
{
    p_collision_middleware.allocator.free_box_collider_component(p_collision, token_build<BoxColliderComponent>(p_node_component.resource));
};
}; // namespace BoxColliderComponentAsset_SceneCommunication

namespace CameraComponentAsset_SceneCommunication
{

inline static NodeComponent build_nodecomponent()
{
    return NodeComponent{CameraComponent::Type, tokent_build_default()};
};

inline static CameraComponent::Asset deconstruct_nodecomponent(D3RenderMiddleWare& p_render_middleware, D3Renderer& p_renderer)
{
    return p_render_middleware.camera_component.asset;
};

inline static void on_node_component_removed(D3RenderMiddleWare& p_render_middleware, const NodeComponent& p_node_component)
{
    p_render_middleware.free_camera();
};

}; // namespace CameraComponentAsset_SceneCommunication

namespace MeshRendererComponentAsset_SceneCommunication
{
inline static NodeComponent build_nodecomponent(const Token<MeshRendererComponent> p_component)
{
    return NodeComponent{MeshRendererComponent::Type, token_value(p_component)};
};

inline static void on_node_component_removed(D3RenderMiddleWare& p_render_middleware, RenderResourceAllocator2& p_render_resource_allocator, const NodeComponent& p_node_component)
{
    MeshRendererComponentComposition::free_meshrenderer_with_dependencies(p_render_middleware.meshrenderer_component_unit, p_render_resource_allocator,
                                                                          token_build<MeshRendererComponent>(p_node_component.resource));
};
}; // namespace MeshRendererComponentAsset_SceneCommunication

#define EgineModule_RendererComponents_on_removed(p_render_middleware, p_render_resource_allocator, p_node_component)                                                                                  \
    case MeshRendererComponent::Type:                                                                                                                                                                  \
    {                                                                                                                                                                                                  \
        MeshRendererComponentAsset_SceneCommunication::on_node_component_removed((p_render_middleware), (p_render_resource_allocator), (p_node_component));                                                  \
    }                                                                                                                                                                                                  \
    break;                                                                                                                                                                                             \
    case CameraComponent::Type:                                                                                                                                                                        \
    {                                                                                                                                                                                                  \
        CameraComponentAsset_SceneCommunication::on_node_component_removed((p_render_middleware), (p_node_component));                                                                                     \
    }                                                                                                                                                                                                  \
    break;

#define EngineModuleCollision_BoxColliderComponent_on_removed(p_collision_middleware, p_collision, p_node_component)                                                                                   \
    case BoxColliderComponent::Type:                                                                                                                                                                   \
    {                                                                                                                                                                                                  \
        BoxColliderComponentAsset_SceneCommunication::on_node_component_removed((p_collision_middleware), (p_collision), (p_node_component));                                                                \
    }                                                                                                                                                                                                  \
    break;
