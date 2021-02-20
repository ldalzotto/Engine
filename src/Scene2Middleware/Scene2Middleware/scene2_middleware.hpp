#pragma once

#include "Collision/collision.hpp"
#include "Render/render.hpp"
#include "Scene2/scene2.hpp"

#include "./Middlewares/collision.hpp"
#include "./Middlewares/render.hpp"

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
		RenderMiddleWare render_middleware;

		static SceneMiddleware allocate_default();

		void free(Scene* p_scene, Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context);

		void step(Scene* p_scene, Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context);
	};

	inline SceneMiddleware SceneMiddleware::allocate_default()
	{
		return SceneMiddleware{
				CollisionMiddleware::allocate_default(),
				RenderMiddleWare::allocate()
		};
	};

	inline void SceneMiddleware::free(Scene* p_scene, Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context)
	{
		this->collision_middleware.free(p_collision, p_scene);
		this->render_middleware.free(p_renderer, p_gpu_context, p_scene);
	};

	void SceneMiddleware::step(Scene* p_scene, Collision2& p_collision, D3Renderer& p_renderer, GPUContext& p_gpu_context)
	{
		this->collision_middleware.step(p_collision, p_scene);
		this->render_middleware.step(p_renderer, p_gpu_context, p_scene);
	};
}

#include "./communication_layer.hpp"
