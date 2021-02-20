#pragma once

namespace v2
{
	struct CameraComponentAsset
	{
		float32 Near;
		float32 Far;
		float32 Fov;
	};

	struct CameraComponent
	{
		static constexpr component_t Type = HashRaw_constexpr(STR(CameraComponent));

		Token(Node) scene_node;
	};

	struct RenderHeap
	{
		CameraComponent camera;

		struct ShaderModule
		{
			uimax id;
			Token(ShaderModule) shader_module;
		};

		Pool<RenderHeap::ShaderModule> shader_modules;
	};

	struct RenderAllocator
	{
		RenderHeap heap;
		int8 camera_allocated;

		inline static RenderAllocator allocate()
		{
			return RenderAllocator{
					RenderHeap{},
					0
			};
		};

		inline void allocate_camera(const CameraComponent& p_camera)
		{
			this->heap.camera = p_camera;
			this->camera_allocated = 1;
		};

		inline void free_camera()
		{
			this->camera_allocated = 0;
		};
	};

	struct RenderEvents
	{
		struct CameraAllocated
		{
			Token(Node) scene_node;
			CameraComponentAsset camera;
		};

		Vector<CameraAllocated> camera_allocated;

		struct ShaderModuleAllocation
		{
			uimax id;
			Span<int8> compiled_shader;
		};

		Vector<ShaderModuleAllocation> shader_modules;

		inline static RenderEvents allocate()
		{
			return RenderEvents{
					Vector<CameraAllocated>::allocate(0),
					Vector<ShaderModuleAllocation>::allocate(0)
			};
		};

		inline void free()
		{
			this->camera_allocated.free();
			this->shader_modules.free();
		};
	};

	struct RenderMiddleWare
	{
		RenderAllocator allocator;
		RenderEvents events;

		inline static RenderMiddleWare allocate()
		{
			return RenderMiddleWare{
					RenderAllocator::allocate(), RenderEvents::allocate()
			};
		};

		inline void free(D3Renderer& p_renderer, GPUContext& p_gpu_context, Scene* p_scene)
		{
			this->step(p_renderer, p_gpu_context, p_scene);

			this->events.free();
		};

		inline void step(D3Renderer& p_renderer, GPUContext& p_gpu_context, Scene* p_scene)
		{
			for (loop(i, 0, this->events.camera_allocated.Size))
			{
				RenderEvents::CameraAllocated& l_event = this->events.camera_allocated.get(i);
				NodeEntry l_node = p_scene->get_node(l_event.scene_node);
				this->allocator.heap.camera = CameraComponent{ l_event.scene_node };
				Slice<Camera> l_camera_slice = p_renderer.color_step.get_camera(p_gpu_context);
				l_camera_slice.get(0).projection = m44f::perspective(l_event.camera.Fov, (float32)p_renderer.color_step.render_target_dimensions.x / p_renderer.color_step.render_target_dimensions.y, l_event.camera.Near, l_event.camera.Far);
				//TODO -> having a set_camera_projection raw values in the Render module
			}
			this->events.camera_allocated.clear();

			/*
			for(loop(i, 0, this->events.shader_modules.Size))
			{
				auto& l_event = this->events.shader_modules.get(i);
				Token(ShaderModule) l_shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_event.compiled_shader.slice);
				// RenderHeap::ShaderModule l_heap_shader_module = {l_event.id, l_shader_module};
			}
			*/

			if (this->allocator.camera_allocated)
			{
				NodeEntry l_node = p_scene->get_node(this->allocator.heap.camera.scene_node);
				if (l_node.Element->state.haschanged_thisframe)
				{
					Slice<Camera> l_camera_slice = p_renderer.color_step.get_camera(p_gpu_context);
					m44f l_local_to_world = p_scene->tree.get_localtoworld(l_node);
					l_camera_slice.get(0).view = m44f::view(p_scene->tree.get_worldposition(l_node), l_local_to_world.Forward.Vec3, l_local_to_world.Up.Vec3);
					//TODO -> push view matrix
					//TODO -> having a set_camera_view in the Render module
				}
			}
		};
	};
};