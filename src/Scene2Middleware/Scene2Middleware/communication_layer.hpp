#pragma once

namespace v2
{
namespace BoxColliderComponentAsset_SceneCommunication
{
inline static BoxColliderComponentAsset from_json(JSONDeserializer& p_json_deserialiazer)
{
    BoxColliderComponentAsset l_return;
    JSONDeserializer l_deserializer;
    p_json_deserialiazer.next_object("half_extend", &l_deserializer);
    l_return.half_extend = MathJSONDeserialization::_v3f(&l_deserializer);
    l_deserializer.free();
    return l_return;
};

inline static void to_json(const BoxColliderComponentAsset& p_component, JSONSerializer* in_out_json_serializer)
{
    in_out_json_serializer->start_object(slice_int8_build_rawstr("half_extend"));
    MathJSONSerialization::_v3f(p_component.half_extend, in_out_json_serializer);
    in_out_json_serializer->end_object();
};

inline static NodeComponent construct_nodecomponent(const Token(BoxColliderComponent) p_ressource)
{
    return NodeComponent{BoxColliderComponent::Type, tk_v(p_ressource)};
};

inline static BoxColliderComponentAsset desconstruct_nodecomponent(SceneMiddleware& p_scene_middleware, Collision2& p_collision, const NodeComponent& p_node_component)
{
    return BoxColliderComponentAsset{p_scene_middleware.collision_middleware.allocator.box_collider_get_world_half_extend(p_collision, tk_b(BoxColliderComponent, p_node_component.resource))};
};

inline static void on_node_component_removed(SceneMiddleware* p_scene_middleware, Collision2& p_collision, const NodeComponent& p_node_component)
{
    p_scene_middleware->collision_middleware.allocator.free_box_collider_component(p_collision, tk_b(BoxColliderComponent, p_node_component.resource));
};
}; // namespace BoxColliderComponentAsset_SceneCommunication

namespace CameraComponentAsset_SceneCommunication
{

inline static NodeComponent construct_nodecomponent()
{
    return NodeComponent{CameraComponent::Type, tokent_build_default()};
};

inline static CameraComponent::Asset deconstruct_nodecomponent(SceneMiddleware& p_scene_middleware, D3Renderer& p_renderer)
{
    return p_scene_middleware.render_middleware.allocator.camera_component.asset;
};

inline static void on_node_component_removed(RenderMiddleWare& p_render_middleware, const NodeComponent& p_node_component)
{
    p_render_middleware.allocator.free_camera();
};

}; // namespace CameraComponentAsset_SceneCommunication

namespace MeshRendererComponentAsset_SceneCommunication
{
inline static NodeComponent construct_nodecomponent(const Token(MeshRendererComponent) p_component)
{
    return NodeComponent{MeshRendererComponent::Type, tk_v(p_component)};
};

inline static void on_node_component_removed(RenderMiddleWare& p_render_middleware, const NodeComponent& p_node_component)
{
    RenderRessourceAllocator2Composition::free_meshrenderer_with_dependencies(p_render_middleware.allocator, tk_b(v2::MeshRendererComponent, p_node_component.resource));
};
}; // namespace MeshRendererComponentAsset_SceneCommunication

// global
inline void on_node_component_removed(SceneMiddleware* p_scene_middleware, Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context, const NodeComponent& p_node_component)
{
    switch (p_node_component.type)
    {

    case BoxColliderComponent::Type:
    {
        BoxColliderComponentAsset_SceneCommunication::on_node_component_removed(p_scene_middleware, p_collision, p_node_component);
    }
    break;
    case v2::MeshRendererComponent::Type:
    {
        MeshRendererComponentAsset_SceneCommunication::on_node_component_removed(p_scene_middleware->render_middleware, p_node_component);
    }
    break;
    case v2::CameraComponent::Type:
    {
        CameraComponentAsset_SceneCommunication::on_node_component_removed(p_scene_middleware->render_middleware, p_node_component);
    }
    break;
    default:
        abort();
        break;
    }
};

} // namespace v2