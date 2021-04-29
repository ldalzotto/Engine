#pragma once

#include "Collision/collision.hpp"
#include "Render/render.hpp"
#include "Scene2/scene2.hpp"
#include "AssetResource/asset_ressource.hpp"

#include "./Middlewares/collision.hpp"
#include "./Middlewares/render.hpp"

/*
    The SceneMiddleware is the indirection layer between the Scene and engine Systems.
    When we want to add behavior to a Node of the Scene, we do it by allocating a NodeComponent.
    This NodeComponent holds a reference of a resource (for example, a BoxCollider, a MeshRenderer ...).
    The Middlewares are responsible of this allocation and the update of the associated system.
*/
//TODO -> remove this ? YES :)
struct SceneMiddleware
{
    CollisionMiddleware collision_middleware;
    RenderMiddleWare render_middleware;

    static SceneMiddleware allocate_default();

    void free(Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context, RenderResourceAllocator2& p_render_resource_allocator, AssetDatabase& p_asset_database);

};

inline SceneMiddleware SceneMiddleware::allocate_default()
{
    return SceneMiddleware{CollisionMiddleware::allocate_default(), RenderMiddleWare::allocate()};
};

inline void SceneMiddleware::free(Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context, RenderResourceAllocator2& p_render_resource_allocator,
                                  AssetDatabase& p_asset_database)
{
    this->collision_middleware.free(p_collision);
    this->render_middleware.free(p_renderer, p_gpu_context, p_asset_database, p_render_resource_allocator);
};

#include "./communication_layer.hpp"
