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

#include "./communication_layer.hpp"
