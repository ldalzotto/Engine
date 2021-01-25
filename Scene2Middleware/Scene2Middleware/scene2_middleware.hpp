#pragma once

#include "Collision/collision.hpp"
#include "Scene2/scene2.hpp"

#include "./Middlewares/collision.hpp"

namespace v2
{
    /*
        The SceneMiddleware is the indirection layer between the Scene and engine Systems.
        When we want to add behavior to a Node of the Scene, we do it by allocating a NodeComponent.
        This NodeComponent holds a reference of a ressource (for example, a BoxCollider, a MeshRenderer ...).
        The Middlewares are responsible of this allocation and the update of the associated system.
    */
    struct SceneMiddleware
    {
        CollisionMiddleware collision_middleware;

        static SceneMiddleware allocate_default(Collision2* p_collision);
        void free();

        void step(Scene* p_scene);
    };

    inline SceneMiddleware SceneMiddleware::allocate_default(Collision2* p_collision)
    {
        return SceneMiddleware{
            CollisionMiddleware::allocate_default(p_collision)
        };
    };

    inline void SceneMiddleware::free()
    {
        this->collision_middleware.free();
    };

    void SceneMiddleware::step(Scene* p_scene)
    {
        this->collision_middleware.step(p_scene);
    };
}

#include "./communication_layer.hpp"
