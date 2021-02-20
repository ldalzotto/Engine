#pragma once

namespace v2
{
	struct RenderRessourceHeader
	{
		//TODO -> thinking of a way to handle ressource deallocation event cleaning.
		// Maybe inspired by the collision system ?
		int8 allocated;
	};

	struct ShaderModuleRessource
	{
		RenderRessourceHeader header;
		uimax id;
		Token(ShaderModule) shader_module;
	};

	struct MeshRessource
	{
		RenderRessourceHeader header;
		uimax id;
		Token(Mesh) mesh;
	};

	struct ShaderRessource
	{
		RenderRessourceHeader header;
		uimax id;
		Token(ShaderModuleRessource) vertex_shader;
		Token(ShaderModuleRessource) fragment_shader;
		Token(ShaderIndex) shader;
	};

	struct MaterialRessource
	{
		RenderRessourceHeader header;
		Token(ShaderRessource) shader;
		Token(Material) material;
	};

	struct RenderableObjectRessource
	{
		RenderRessourceHeader header;
		Token(MeshRessource) mesh;
		Token(MaterialRessource) material;
		Token(RenderableObject) renderable_object;
	};

	struct CameraComponentAsset
	{
		float32 Near;
		float32 Far;
		float32 Fov;
	};

	struct CameraComponent
	{
		static constexpr component_t Type = HashRaw_constexpr(STR(CameraComponent));
		int8 force_update;
		Token(Node) scene_node;
	};


	struct MeshRendererComponent
	{
		static constexpr component_t Type = HashRaw_constexpr(STR(MeshRendererComponent));
		int8 force_update;
		Token(Node) scene_node;
		Token(RenderableObjectRessource) renderable_object;
	};

	struct RenderHeap
	{
		Pool<ShaderModuleRessource> shader_modules;
		Pool<MeshRessource> mesh;
		Pool<ShaderRessource> shaders;
		Pool<RenderableObjectRessource> renderable_objects;
		Pool<MaterialRessource> materials;
		CameraComponent camera;

		inline static RenderHeap allocate()
		{
			return RenderHeap{
					Pool<ShaderModuleRessource>::allocate(0),
					Pool<MeshRessource>::allocate(0),
					Pool<ShaderRessource>::allocate(0),
					Pool<RenderableObjectRessource>::allocate(0),
					Pool<MaterialRessource>::allocate(0)
			};
		};

		inline void free()
		{
#if RENDER_BOUND_TEST
			assert_true(!this->shader_modules.has_allocated_elements());
			assert_true(!this->mesh.has_allocated_elements());
			assert_true(!this->shaders.has_allocated_elements());
			assert_true(!this->renderable_objects.has_allocated_elements());
			assert_true(!this->materials.has_allocated_elements());
#endif
			this->shader_modules.free();
			this->mesh.free();
			this->shaders.free();
			this->renderable_objects.free();
			this->materials.free();
		};
	};

	//TODO -> adding reference removal when free
	struct RenderRessourceAllocator
	{
		RenderHeap heap;

		struct ShaderModuleRessource_AllocationEvent
		{
			Span<int8> shader_module_compiled;
			Token(ShaderModuleRessource) shader_module_ressource;
		};

		Vector<ShaderModuleRessource_AllocationEvent> shadermoduleressource_events;

		struct MeshRessource_AllocationEvent
		{
			Span<Vertex> initial_vertices;
			Span<uint32> initial_indices;
			Token(MeshRessource) mesh_ressource;
		};

		Vector<MeshRessource_AllocationEvent> meshressource_events;

		struct ShaderRessource_AllocationEvent
		{
			Token(ShaderModuleRessource) vertex_shader;
			Token(ShaderModuleRessource) fragment_shader;
			Span<ShaderLayoutParameterType> specific_parameters;
			uimax execution_order;
			ShaderConfiguration shader_configuration;
			Token(ShaderRessource) shader_ressource;
		};

		Vector<ShaderRessource_AllocationEvent> shaderressource_events;

		struct RenderableObject_AllocationEvent
		{
			Token(MeshRessource) mesh;
			Token(MaterialRessource) material;
			Token(RenderableObjectRessource) renderable_object;
		};

		Vector<RenderableObject_AllocationEvent> renderableobject_events;

		struct Material_AllocationEvent
		{
			Token(ShaderRessource) shader;
			Token(MaterialRessource) material;
		};

		Vector<Material_AllocationEvent> material_events;

		PoolIndexed<MeshRendererComponent> meshrenderers;

		inline static RenderRessourceAllocator allocate()
		{
			return RenderRessourceAllocator{
					RenderHeap::allocate(),
					Vector<ShaderModuleRessource_AllocationEvent>::allocate(0),
					Vector<MeshRessource_AllocationEvent>::allocate(0),
					Vector<ShaderRessource_AllocationEvent>::allocate(0),
					Vector<RenderableObject_AllocationEvent>::allocate(0),
					Vector<Material_AllocationEvent>::allocate(0),
					PoolIndexed<MeshRendererComponent>::allocate_default()
			};
		};

		inline void free()
		{
			this->heap.free();

#if RENDER_BOUND_TEST
			assert_true(!this->meshrenderers.has_allocated_elements());
#endif

			this->shadermoduleressource_events.free();
			this->meshressource_events.free();
			this->shaderressource_events.free();
			this->renderableobject_events.free();
			this->material_events.free();

			this->meshrenderers.free();
		};

		inline void step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
		{
			for (loop_reverse(i, 0, this->shadermoduleressource_events.Size))
			{
				auto& l_event = this->shadermoduleressource_events.get(i);
				ShaderModuleRessource& l_ressource = this->heap.shader_modules.get(l_event.shader_module_ressource);
				l_ressource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_event.shader_module_compiled.slice);
				l_ressource.header.allocated = 1;
				l_event.shader_module_compiled.free();
				this->shadermoduleressource_events.pop_back();
			}

			for (loop_reverse(i, 0, this->meshressource_events.Size))
			{
				auto& l_event = this->meshressource_events.get(i);
				MeshRessource& l_ressource = this->heap.mesh.get(l_event.mesh_ressource);
				l_ressource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_event.initial_vertices.slice, l_event.initial_indices.slice);
				l_ressource.header.allocated = 1;
				l_event.initial_vertices.free();
				l_event.initial_indices.free();
				this->meshressource_events.pop_back();
			}

			for (loop_reverse(i, 0, this->shaderressource_events.Size))
			{
				auto& l_event = this->shaderressource_events.get(i);
				ShaderRessource& l_ressource = this->heap.shaders.get(l_event.shader_ressource);

				ShaderModuleRessource& l_vertex_shader = this->heap.shader_modules.get(l_event.vertex_shader);
				ShaderModuleRessource& l_fragment_shader = this->heap.shader_modules.get(l_event.fragment_shader);

				if (l_vertex_shader.header.allocated && l_fragment_shader.header.allocated)
				{
					l_ressource.shader = D3RendererAllocatorComposition::allocate_colorstep_shader_with_shaderlayout(
							p_gpu_context.graphics_allocator, p_renderer.allocator, l_event.specific_parameters.slice, l_event.execution_order,
							p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_renderer.color_step.pass),
							l_event.shader_configuration,
							p_gpu_context.graphics_allocator.heap.shader_modules.get(l_vertex_shader.shader_module),
							p_gpu_context.graphics_allocator.heap.shader_modules.get(l_fragment_shader.shader_module)
					);
					l_event.specific_parameters.free();
					l_ressource.header.allocated = 1;
					this->shaderressource_events.pop_back();
				}
			}

			for (loop_reverse(i, 0, this->material_events.Size))
			{
				auto& l_event = this->material_events.get(i);
				MaterialRessource& l_ressource = this->heap.materials.get(l_event.material);

				ShaderRessource& l_shader = this->heap.shaders.get(l_event.shader);
				if (l_shader.header.allocated)
				{
					l_ressource.material = p_renderer.allocator.allocate_material(Material::allocate_empty(p_gpu_context.graphics_allocator, 1));
					p_renderer.allocator.heap.link_shader_with_material(l_shader.shader, l_ressource.material);
					l_ressource.header.allocated = 1;
					this->material_events.pop_back();
				}
			}

			for (loop_reverse(i, 0, this->renderableobject_events.Size))
			{
				auto& l_event = this->renderableobject_events.get(i);
				RenderableObjectRessource& l_ressource = this->heap.renderable_objects.get(l_event.renderable_object);

				MeshRessource& l_mesh = this->heap.mesh.get(l_event.mesh);
				MaterialRessource& l_material = this->heap.materials.get(l_event.material);

				if (l_mesh.header.allocated && l_material.header.allocated)
				{
					l_ressource.renderable_object = D3RendererAllocatorComposition::allocate_renderable_object_with_buffers(
							p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_mesh.mesh
					);

					p_renderer.allocator.heap.link_material_with_renderable_object(l_material.material, l_ressource.renderable_object);

					l_ressource.header.allocated = 1;
					this->renderableobject_events.pop_back();
				}
			}

		};

		inline Token(ShaderModuleRessource) allocate_shader_module(const Span<int8>& p_shader_module_compiled)
		{
			ShaderModuleRessource l_sahder_module = ShaderModuleRessource{ RenderRessourceHeader{ 0 }, 0, tk_bd(ShaderModule)};
			ShaderModuleRessource_AllocationEvent l_event = ShaderModuleRessource_AllocationEvent{ p_shader_module_compiled, this->heap.shader_modules.alloc_element(l_sahder_module) };
			this->shadermoduleressource_events.push_back_element(l_event);
			return l_event.shader_module_ressource;
		};

		inline Token(MeshRessource) allocate_mesh(const Span<Vertex>& p_initial_vertices, const Span<uint32>& p_initial_indices)
		{
			MeshRessource l_mesh = MeshRessource{ RenderRessourceHeader{ 0 }, 0, tk_bd(Mesh)};
			MeshRessource_AllocationEvent l_event = MeshRessource_AllocationEvent{ p_initial_vertices, p_initial_indices, this->heap.mesh.alloc_element(l_mesh) };
			this->meshressource_events.push_back_element(l_event);
			return l_event.mesh_ressource;
		};

		inline Token(ShaderRessource) allocate_shader(const Token(ShaderModuleRessource) p_vertex_shader, const Token(ShaderModuleRessource) p_fragment_shader,
				Span<ShaderLayoutParameterType> p_specific_parameters, const uimax execution_order, const ShaderConfiguration& p_shader_configuration)
		{
			Token(ShaderRessource) l_shader = this->heap.shaders.alloc_element(ShaderRessource{ RenderRessourceHeader{ 0 }, 0, p_vertex_shader, p_fragment_shader, tk_bd(ShaderIndex)});
			this->shaderressource_events.push_back_element(
					ShaderRessource_AllocationEvent{
							p_vertex_shader, p_fragment_shader, p_specific_parameters, execution_order, p_shader_configuration,
							l_shader
					}
			);
			return l_shader;
		};

		inline Token(MaterialRessource) allocate_material(const Token(ShaderRessource) p_shader)
		{
			Token(MaterialRessource) l_material = this->heap.materials.alloc_element(MaterialRessource{ RenderRessourceHeader{ 0 }, p_shader, tk_bd(Material)});
			this->material_events.push_back_element(Material_AllocationEvent{
					p_shader, l_material
			});
			return l_material;
		};

		inline Token(RenderableObjectRessource) allocate_renderable_object(const Token(MeshRessource) p_mesh, const Token(MaterialRessource) p_material)
		{
			Token(RenderableObjectRessource) l_renderable_object = this->heap.renderable_objects.alloc_element(RenderableObjectRessource{ RenderRessourceHeader{ 0 }, p_mesh, p_material, tk_bd(RenderableObject)});
			this->renderableobject_events.push_back_element(
					RenderableObject_AllocationEvent{
							p_mesh, p_material, l_renderable_object
					}
			);
			return l_renderable_object;
		};

		inline Token(MeshRendererComponent) allocate_meshrenderer_component(const Token(Node) p_scene_node, const Span<int8>& p_vertex_shader_module_compiled, const Span<int8>& p_fragment_shader_module_compiled,
				Span<ShaderLayoutParameterType> p_specific_parameters, const uimax execution_order, const ShaderConfiguration& p_shader_configuration,
				const Span<Vertex>& p_initial_vertices, const Span<uint32>& p_initial_indices)
		{
			Token(ShaderModuleRessource) l_vertex_shader = this->allocate_shader_module(p_vertex_shader_module_compiled);
			Token(ShaderModuleRessource) l_fragment_shader = this->allocate_shader_module(p_fragment_shader_module_compiled);
			Token(ShaderRessource) l_shader = this->allocate_shader(l_vertex_shader, l_fragment_shader, p_specific_parameters, execution_order, p_shader_configuration);
			Token(MaterialRessource) l_material = this->allocate_material(l_shader);
			Token(MeshRessource) l_mesh = this->allocate_mesh(p_initial_vertices, p_initial_indices);
			Token(RenderableObjectRessource) l_renderable_object = this->allocate_renderable_object(l_mesh, l_material);
			return this->meshrenderers.alloc_element(MeshRendererComponent{ 1, p_scene_node, l_renderable_object });
		};

		inline void free_meshrenderer_component(D3Renderer& p_renderer, GPUContext& p_gpu_context, const Token(MeshRendererComponent) p_mesh_renderer)
		{
			MeshRendererComponent& l_mesh_renderer = this->meshrenderers.get(p_mesh_renderer);

			{
				RenderableObjectRessource& l_renderable_object_ressource = this->heap.renderable_objects.get(l_mesh_renderer.renderable_object);
				if (l_renderable_object_ressource.header.allocated)
				{
					MeshRessource& l_mesh_ressource = this->heap.mesh.get(l_renderable_object_ressource.mesh);
					if (l_mesh_ressource.header.allocated)
					{
						D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_mesh_ressource.mesh);
					}
					this->heap.mesh.release_element(l_renderable_object_ressource.mesh);

					MaterialRessource& l_material_ressource = this->heap.materials.get(l_renderable_object_ressource.material);
					if (l_material_ressource.header.allocated)
					{
						D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_material_ressource.material);

						ShaderRessource& l_shader_ressource = this->heap.shaders.get(l_material_ressource.shader);
						if (l_shader_ressource.header.allocated)
						{
							D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, l_shader_ressource.shader);

							ShaderModuleRessource& l_vertex_shader = this->heap.shader_modules.get(l_shader_ressource.vertex_shader);
							if (l_vertex_shader.header.allocated)
							{
								p_gpu_context.graphics_allocator.free_shader_module(l_vertex_shader.shader_module);
							}
							this->heap.shader_modules.release_element(l_shader_ressource.vertex_shader);
							ShaderModuleRessource& l_fragement_shader = this->heap.shader_modules.get(l_shader_ressource.fragment_shader);
							if (l_fragement_shader.header.allocated)
							{
								p_gpu_context.graphics_allocator.free_shader_module(l_fragement_shader.shader_module);
							}
							this->heap.shader_modules.release_element(l_shader_ressource.fragment_shader);
						}
						this->heap.shaders.release_element(l_material_ressource.shader);
					}
					this->heap.materials.release_element(l_renderable_object_ressource.material);

					D3RendererAllocatorComposition::free_renderable_object_with_buffers(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_renderable_object_ressource.renderable_object);
				}


				this->heap.renderable_objects.release_element(l_mesh_renderer.renderable_object);
			}

			this->meshrenderers.release_element(p_mesh_renderer);
		};

		inline void allocate_camera_component(const Token(Node) p_scene_node, const CameraComponentAsset& p_camera_component_asset)
		{
			//TODO
		};

	};

	struct RenderMiddleWare
	{
		RenderRessourceAllocator allocator;

		inline static RenderMiddleWare allocate()
		{
			return RenderMiddleWare{
					RenderRessourceAllocator::allocate()
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

			for (loop(i, 0, this->allocator.meshrenderers.Indices.Size))
			{
				MeshRendererComponent& l_mesh_renderer = this->allocator.meshrenderers.get(this->allocator.meshrenderers.Indices.get(i));
				NodeEntry l_node = p_scene->get_node(l_mesh_renderer.scene_node);
				if (l_mesh_renderer.force_update || l_node.Element->state.haschanged_thisframe)
				{
					p_renderer.heap().push_modelupdateevent(D3RendererHeap::RenderableObject_ModelUpdateEvent{
							this->allocator.heap.renderable_objects.get(l_mesh_renderer.renderable_object).renderable_object,
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