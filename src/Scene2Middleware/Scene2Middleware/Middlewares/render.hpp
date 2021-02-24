#pragma once

namespace v2
{
	struct RenderHeap
	{
		PoolHashedCounted<hash_t, ShaderModuleRessource> shader_modules_v2;
		PoolHashedCounted<hash_t, MeshRessource> mesh_v2;
		PoolHashedCounted<hash_t, ShaderRessource> shaders_v3;
		PoolHashedCounted<hash_t, MaterialRessource> materials;
		CameraComponent camera;

		inline static RenderHeap allocate()
		{
			return RenderHeap{
					PoolHashedCounted<hash_t, ShaderModuleRessource>::allocate_default(),
					PoolHashedCounted<hash_t, MeshRessource>::allocate_default(),
					PoolHashedCounted<hash_t, ShaderRessource>::allocate_default(),
					PoolHashedCounted<hash_t, MaterialRessource>::allocate_default()
			};
		};

		inline void free()
		{
#if RENDER_BOUND_TEST
			assert_true(this->shaders_v3.empty());
			assert_true(this->mesh_v2.empty());
			assert_true(this->shaders_v3.empty());
			assert_true(this->materials.empty());
#endif
			this->shader_modules_v2.free();
			this->mesh_v2.free();
			this->shaders_v3.free();
			this->materials.free();
		};
	};

	struct RenderRessourceAllocator2
	{
		RenderHeap heap;

		struct MeshRendererAllocationEvent
		{
			Token(MeshRendererComponent)RessourceAllocationEvent_member_allocated_ressource;
		};

		Vector<ShaderModuleRessource::AllocationEvent> shadermodule_allocation_events;
		Vector<MeshRessource::AllocationEvent> mesh_allocation_events;
		Vector<ShaderRessource::AllocationEvent> shader_allocation_events;
		Vector<MaterialRessource::AllocationEvent> material_allocation_events;
		Vector<MeshRendererAllocationEvent> mesh_renderer_allocation_events;

		Vector<ShaderModuleRessource::FreeEvent> shadermodule_free_events;
		Vector<MeshRessource::FreeEvent> mesh_free_events;
		Vector<ShaderRessource::FreeEvent> shader_free_events;
		Vector<MaterialRessource::FreeEvent> material_free_events;
		Vector<MeshRendererComponent::FreeEvent> meshrenderer_free_events;

		PoolIndexed<MeshRendererComponent> mesh_renderers;
		PoolIndexed<MeshRendererComponent::NestedDependencies> mesh_renderers_dependencies;

		inline static RenderRessourceAllocator2 allocate()
		{
			return RenderRessourceAllocator2{
					RenderHeap::allocate(),
					Vector<ShaderModuleRessource::AllocationEvent>::allocate(0),
					Vector<MeshRessource::AllocationEvent>::allocate(0),
					Vector<ShaderRessource::AllocationEvent>::allocate(0),
					Vector<MaterialRessource::AllocationEvent>::allocate(0),
					Vector<MeshRendererAllocationEvent>::allocate(0),
					Vector<ShaderModuleRessource::FreeEvent>::allocate(0),
					Vector<MeshRessource::FreeEvent>::allocate(0),
					Vector<ShaderRessource::FreeEvent>::allocate(0),
					Vector<MaterialRessource::FreeEvent>::allocate(0),
					Vector<MeshRendererComponent::FreeEvent>::allocate(0),
					PoolIndexed<MeshRendererComponent>::allocate_default(),
					PoolIndexed<MeshRendererComponent::NestedDependencies>::allocate_default()
			};
		};

		inline void free()
		{
			this->heap.free();

#if RENDER_BOUND_TEST
			assert_true(this->shadermodule_allocation_events.empty());
			assert_true(this->mesh_allocation_events.empty());
			assert_true(this->shader_allocation_events.empty());
			assert_true(this->material_allocation_events.empty());
			assert_true(this->mesh_renderer_allocation_events.empty());
			assert_true(this->shadermodule_free_events.empty());
			assert_true(this->mesh_free_events.empty());
			assert_true(this->shader_free_events.empty());
			assert_true(this->material_free_events.empty());
			assert_true(this->meshrenderer_free_events.empty());
			assert_true(!this->mesh_renderers.has_allocated_elements());
			assert_true(!this->mesh_renderers_dependencies.has_allocated_elements());
#endif

			this->shadermodule_allocation_events.free();
			this->mesh_allocation_events.free();
			this->shader_allocation_events.free();
			this->material_allocation_events.free();
			this->mesh_renderer_allocation_events.free();
			this->shadermodule_free_events.free();
			this->mesh_free_events.free();
			this->shader_free_events.free();
			this->material_free_events.free();
			this->meshrenderer_free_events.free();
			this->mesh_renderers.free();
			this->mesh_renderers_dependencies.free();
		};

		inline void step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
		{
			for (loop_reverse(i, 0, this->meshrenderer_free_events.Size))
			{
				auto& l_event = this->meshrenderer_free_events.get(i);
				MeshRendererComponent& l_mesh_renderer = this->mesh_renderers.get(l_event.component);
				MaterialRessource& l_linked_material = this->heap.materials.pool.get(l_event.linked_material);
				p_renderer.allocator.heap.unlink_material_with_renderable_object(l_linked_material.material, l_mesh_renderer.renderable_object);
				D3RendererAllocatorComposition::free_renderable_object_with_buffers(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_mesh_renderer.renderable_object);
				this->mesh_renderers.release_element(l_event.component);
				this->mesh_renderers_dependencies.release_element(tk_bf(MeshRendererComponent::NestedDependencies, l_event.component));
				this->meshrenderer_free_events.pop_back();
			}

			for (loop_reverse(i, 0, this->material_free_events.Size))
			{
				auto& l_event = this->material_free_events.get(i);
				MaterialRessource& l_ressource = this->heap.materials.pool.get(l_event.ressource);
				ShaderRessource& l_linked_shader = this->heap.shaders_v3.pool.get(l_event.dependencies.shader);
				p_renderer.allocator.heap.unlink_shader_with_material(l_linked_shader.shader, l_ressource.material);
				D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_ressource.material);
				this->heap.materials.pool.release_element(l_event.ressource);
				this->material_free_events.pop_back();
			}

			for (loop_reverse(i, 0, this->shader_free_events.Size))
			{
				auto& l_event = this->shader_free_events.get(i);
				ShaderRessource& l_ressource = this->heap.shaders_v3.pool.get(l_event.ressource);
				D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, l_ressource.shader);
				this->heap.shaders_v3.pool.release_element(l_event.ressource);
				this->shader_free_events.pop_back();
			}

			for (loop_reverse(i, 0, this->mesh_free_events.Size))
			{
				auto& l_event = this->mesh_free_events.get(i);
				MeshRessource& l_ressource = this->heap.mesh_v2.pool.get(l_event.ressource);
				D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_ressource.mesh);
				this->heap.mesh_v2.pool.release_element(l_event.ressource);
				this->mesh_free_events.pop_back();
			}

			for (loop_reverse(i, 0, this->shadermodule_free_events.Size))
			{
				auto& l_event = this->shadermodule_free_events.get(i);
				ShaderModuleRessource& l_ressource = this->heap.shader_modules_v2.pool.get(l_event.ressource);
				p_gpu_context.graphics_allocator.free_shader_module(l_ressource.shader_module);
				this->heap.shader_modules_v2.pool.release_element(l_event.ressource);
				this->shadermodule_free_events.pop_back();
			}

			for (loop_reverse(i, 0, this->shadermodule_allocation_events.Size))
			{
				auto& l_event = this->shadermodule_allocation_events.get(i);
				ShaderModuleRessource& l_ressource = this->heap.shader_modules_v2.pool.get(l_event.allocated_ressource);
				l_ressource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_event.asset.compiled_shader.slice);
				l_ressource.header.allocated = 1;
				l_event.asset.compiled_shader.free();
				this->shadermodule_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->mesh_allocation_events.Size))
			{
				auto& l_event = this->mesh_allocation_events.get(i);
				MeshRessource& l_ressource = this->heap.mesh_v2.pool.get(l_event.allocated_ressource);
				l_ressource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_event.asset.initial_vertices.slice, l_event.asset.initial_indices.slice);
				l_ressource.header.allocated = 1;
				l_event.asset.initial_vertices.free();
				l_event.asset.initial_indices.free();
				this->mesh_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->shader_allocation_events.Size))
			{
				auto& l_event = this->shader_allocation_events.get(i);
				ShaderRessource& l_ressource = this->heap.shaders_v3.pool.get(l_event.allocated_ressource);

				ShaderModuleRessource& l_vertex_shader = this->heap.shader_modules_v2.pool.get(l_event.dependencies.vertex_shader);
				ShaderModuleRessource& l_fragment_shader = this->heap.shader_modules_v2.pool.get(l_event.dependencies.fragment_shader);

				l_ressource.shader = D3RendererAllocatorComposition::allocate_colorstep_shader_with_shaderlayout(
						p_gpu_context.graphics_allocator, p_renderer.allocator, l_event.asset.specific_parameters.slice, l_event.asset.execution_order,
						p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass),
						l_event.asset.shader_configuration,
						p_gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.shader_module),
						p_gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.shader_module)
				);
				l_event.asset.specific_parameters.free();
				l_ressource.header.allocated = 1;
				this->shader_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->material_allocation_events.Size))
			{
				auto& l_event = this->material_allocation_events.get(i);
				MaterialRessource& l_ressource = this->heap.materials.pool.get(l_event.allocated_ressource);

				ShaderRessource& l_shader = this->heap.shaders_v3.pool.get(l_event.dependencies.shader);

				l_ressource.material = p_renderer.allocator.allocate_material(Material::allocate_empty(p_gpu_context.graphics_allocator, 1));
				p_renderer.allocator.heap.link_shader_with_material(l_shader.shader, l_ressource.material);
				l_ressource.header.allocated = 1;
				this->material_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->mesh_renderer_allocation_events.Size))
			{
				auto& l_event = this->mesh_renderer_allocation_events.get(i);
				MeshRendererComponent::NestedDependencies& l_dependencies = this->mesh_renderers_dependencies.get(tk_bf(MeshRendererComponent::NestedDependencies, l_event.allocated_ressource));
				MeshRendererComponent& l_mesh_renderer = this->mesh_renderers.get(l_event.allocated_ressource);

				l_mesh_renderer.renderable_object = D3RendererAllocatorComposition::allocate_renderable_object_with_buffers(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator,
						this->heap.mesh_v2.pool.get(l_dependencies.mesh).mesh);
				p_renderer.allocator.heap.link_material_with_renderable_object(this->heap.materials.pool.get(l_dependencies.material).material,
						l_mesh_renderer.renderable_object);
				l_mesh_renderer.allocated = 1;

				this->mesh_renderer_allocation_events.pop_back();
			}
		};

		inline Token(ShaderModuleRessource) allocate_shadermodule_inline(const hash_t p_shadermodule_id, const ShaderModuleRessource::Asset& p_shadermodule_ressource_asset)
		{
			return RessourceComposition::allocate_ressource_composition_explicit(this->heap.shader_modules_v2, this->shadermodule_allocation_events, p_shadermodule_id, ShaderModuleRessource::build_from_id,
					[&p_shadermodule_ressource_asset](const Token(ShaderModuleRessource) p_allocated_ressource)
					{
						return ShaderModuleRessource::AllocationEvent::build_inline(p_shadermodule_ressource_asset, p_allocated_ressource);
					});
		};

		inline void free_shadermodule(const ShaderModuleRessource& p_shader_module)
		{
			RessourceComposition::free_ressource_composition(
					this->heap.shader_modules_v2, this->shadermodule_allocation_events, this->shadermodule_free_events, p_shader_module.header,
					ShaderModuleRessource::FreeEvent::build_from_token
			);
		};

		inline Token(ShaderRessource) allocate_shader_v2_inline(const hash_t p_shader_id, const ShaderRessource::Asset& p_shader_ressource_asset, const ShaderRessource::Dependencies& p_dependencies)
		{
			return RessourceComposition::allocate_ressource_composition_explicit(
					this->heap.shaders_v3, this->shader_allocation_events, p_shader_id, ShaderRessource::build_from_id, [&p_shader_ressource_asset, &p_dependencies](const Token(ShaderRessource) p_allocated_ressource)
					{
						return ShaderRessource::AllocationEvent::build_inline(p_dependencies, p_shader_ressource_asset, p_allocated_ressource);
					}
			);
		};

		inline void free_shader(const ShaderRessource& p_shader)
		{
			RessourceComposition::free_ressource_composition_explicit(
					this->heap.shaders_v3, this->shader_allocation_events, this->shader_free_events, p_shader.header, ShaderRessource::FreeEvent::build_from_token, RessourceComposition::AllocationEventFoundSlot::FreeAsset{}
			);
		};

		inline Token(MeshRessource) allocate_mesh_inline(const hash_t p_mesh_id, const MeshRessource::Asset& p_mesh_ressource_asset)
		{
			return RessourceComposition::allocate_ressource_composition_explicit(
					this->heap.mesh_v2, this->mesh_allocation_events, p_mesh_id, MeshRessource::build_from_id, [&p_mesh_ressource_asset](const Token(MeshRessource) p_allocated_ressource)
					{
						return MeshRessource::AllocationEvent::build_inline(p_mesh_ressource_asset, p_allocated_ressource);
					}
			);
		};

		inline void free_mesh(const MeshRessource& p_mesh)
		{
			RessourceComposition::free_ressource_composition_explicit(
					this->heap.mesh_v2, this->mesh_allocation_events, this->mesh_free_events, p_mesh.header, MeshRessource::FreeEvent::build_from_token, RessourceComposition::AllocationEventFoundSlot::FreeAsset{}
			);
		};

		inline Token(MaterialRessource) allocate_material_inline(const hash_t p_material_id, const MaterialRessource::Asset& p_material_ressource_asset, const MaterialRessource::Dependencies& p_dependencies)
		{
			return RessourceComposition::allocate_ressource_composition_explicit(
					this->heap.materials, this->material_allocation_events, p_material_id, MaterialRessource::build_from_id, [&p_material_ressource_asset, &p_dependencies](const Token(MaterialRessource) p_allocated_ressource)
					{
						return MaterialRessource::AllocationEvent::build_inline(p_dependencies, p_material_ressource_asset, p_allocated_ressource);
					}
			);
		};

		inline void free_material(const Token(MaterialRessource) p_material_token, const MaterialRessource& p_material, const MaterialRessource::Dependencies& p_dependencies)
		{
			RessourceComposition::free_ressource_composition(
					this->heap.materials, this->material_allocation_events, this->material_free_events, p_material.header, [&p_dependencies](Token(MaterialRessource) p_removed_token)
					{ return MaterialRessource::FreeEvent{ p_removed_token, p_dependencies }; }
			);
		};

		inline Token(MeshRendererComponent) allocate_meshrenderer_inline(const MeshRendererComponent::NestedDependencies& p_dependencies, const Token(Node) p_scene_node)
		{
			Token(MeshRendererComponent) l_mesh_renderer = this->mesh_renderers.alloc_element(MeshRendererComponent::build(p_scene_node));
			this->mesh_renderers_dependencies.alloc_element(p_dependencies);
			this->mesh_renderer_allocation_events.push_back_element(MeshRendererAllocationEvent{ l_mesh_renderer });
			return l_mesh_renderer;
		};
	};

	struct RenderRessourceAllocator2Composition
	{
		inline static Token(MeshRendererComponent) allocate_meshrenderer_inline_with_dependencies(RenderRessourceAllocator2& p_render_ressource_allocator,
				const hash_t p_vertex_shader_id, const ShaderModuleRessource::Asset& p_vertex_shader, const hash_t p_fragment_shader_id, const ShaderModuleRessource::Asset& p_fragment_shader,
				const hash_t p_shader_id, const ShaderRessource::Asset& p_shader, const hash_t p_material_id, const MaterialRessource::Asset& p_material, const hash_t p_mesh_id, const MeshRessource::Asset& p_mesh,
				const Token(Node) p_scene_node)
		{
			MeshRendererComponent::NestedDependencies l_mesh_renderer_depencies;
			l_mesh_renderer_depencies.shader_dependencies.vertex_shader = p_render_ressource_allocator.allocate_shadermodule_inline(p_vertex_shader_id, p_vertex_shader);
			l_mesh_renderer_depencies.shader_dependencies.fragment_shader = p_render_ressource_allocator.allocate_shadermodule_inline(p_fragment_shader_id, p_fragment_shader);
			l_mesh_renderer_depencies.material_dependencies.shader = p_render_ressource_allocator.allocate_shader_v2_inline(p_shader_id, p_shader, l_mesh_renderer_depencies.shader_dependencies);
			l_mesh_renderer_depencies.material = p_render_ressource_allocator.allocate_material_inline(p_material_id, p_material, l_mesh_renderer_depencies.material_dependencies);
			l_mesh_renderer_depencies.mesh = p_render_ressource_allocator.allocate_mesh_inline(p_mesh_id, p_mesh);
			return p_render_ressource_allocator.allocate_meshrenderer_inline(l_mesh_renderer_depencies, p_scene_node);
		};

		inline static void free_meshrenderer(RenderRessourceAllocator2& p_render_ressource_allocator, const Token(MeshRendererComponent) p_mesh_renderer)
		{
			MeshRendererComponent& l_mesh_renderer = p_render_ressource_allocator.mesh_renderers.get(p_mesh_renderer);
			MeshRendererComponent::NestedDependencies l_mesh_renderer_dependencies = p_render_ressource_allocator.mesh_renderers_dependencies.get(tk_bf(MeshRendererComponent::NestedDependencies, p_mesh_renderer));

			MaterialRessource& l_material_ressource = p_render_ressource_allocator.heap.materials.pool.get(l_mesh_renderer_dependencies.material);
			ShaderRessource& l_shader_ressource = p_render_ressource_allocator.heap.shaders_v3.pool.get(l_mesh_renderer_dependencies.material_dependencies.shader);
			MeshRessource& l_mesh_ressource = p_render_ressource_allocator.heap.mesh_v2.pool.get(l_mesh_renderer_dependencies.mesh);
			ShaderModuleRessource& l_vertex_module_ressource = p_render_ressource_allocator.heap.shader_modules_v2.pool.get(l_mesh_renderer_dependencies.shader_dependencies.vertex_shader);
			ShaderModuleRessource& l_fragment_module_ressource = p_render_ressource_allocator.heap.shader_modules_v2.pool.get(l_mesh_renderer_dependencies.shader_dependencies.fragment_shader);


			if (l_mesh_renderer.allocated)
			{
				p_render_ressource_allocator.meshrenderer_free_events.push_back_element(MeshRendererComponent::FreeEvent{ p_mesh_renderer, l_mesh_renderer_dependencies.material });
			}
			else
			{
				RessourceComposition::remove_reference_from_allocation_events(p_render_ressource_allocator.mesh_renderer_allocation_events, p_mesh_renderer);
				p_render_ressource_allocator.mesh_renderers.release_element(p_mesh_renderer);
				p_render_ressource_allocator.mesh_renderers_dependencies.release_element(tk_bf(MeshRendererComponent::NestedDependencies, p_mesh_renderer));
			}

			p_render_ressource_allocator.free_material(l_mesh_renderer_dependencies.material, l_material_ressource, l_mesh_renderer_dependencies.material_dependencies);
			p_render_ressource_allocator.free_shader(l_shader_ressource);
			p_render_ressource_allocator.free_mesh(l_mesh_ressource);
			p_render_ressource_allocator.free_shadermodule(l_vertex_module_ressource);
			p_render_ressource_allocator.free_shadermodule(l_fragment_module_ressource);
		};

	};

	struct RenderMiddleWare
	{
		RenderRessourceAllocator2 allocator;

		inline static RenderMiddleWare allocate()
		{
			return RenderMiddleWare{
					RenderRessourceAllocator2::allocate()
			};
		};

		inline void free(D3Renderer& p_renderer, GPUContext& p_gpu_context, Scene* p_scene)
		{
			this->step(p_renderer, p_gpu_context, p_scene);

			this->allocator.free();
		};

		inline void step(D3Renderer& p_renderer, GPUContext& p_gpu_context, Scene* p_scene)
		{
			this->allocator.step(p_renderer, p_gpu_context);

			for (loop(i, 0, this->allocator.mesh_renderers.Indices.Size))
			{
				MeshRendererComponent& l_mesh_renderer = this->allocator.mesh_renderers.get(this->allocator.mesh_renderers.Indices.get(i));
				NodeEntry l_node = p_scene->get_node(l_mesh_renderer.scene_node);
				if (l_mesh_renderer.force_update || l_node.Element->state.haschanged_thisframe)
				{
					p_renderer.heap().push_modelupdateevent(D3RendererHeap::RenderableObject_ModelUpdateEvent{
							l_mesh_renderer.renderable_object,
							p_scene->tree.get_localtoworld(l_node)
					});

					l_mesh_renderer.force_update = 0;
				}
			}

			/*
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
			*/
			/*
			for(loop(i, 0, this->events.shader_modules.Size))
			{
				auto& l_event = this->events.shader_modules.get(i);
				Token(ShaderModule) l_shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_event.compiled_shader.slice);
				// RenderHeap::ShaderModule l_heap_shader_module = {l_event.id, l_shader_module};
			}
			*/
			/*
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
			*/
		};
	};

};