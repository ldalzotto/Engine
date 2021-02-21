#pragma once

namespace v2
{

	struct RenderRessourceHeader
	{
		//TODO -> precising if the ressource is allocated with database or inline ?
		int8 allocated;

		inline static RenderRessourceHeader build_default()
		{
			return RenderRessourceHeader{ 0 };
		};
	};

	struct ShaderModuleRessource
	{
		RenderRessourceHeader header;
		Token(ShaderModule) shader_module;

		inline static ShaderModuleRessource build_default()
		{
			return ShaderModuleRessource{
					RenderRessourceHeader::build_default(),
					tk_bd(ShaderModule)
			};
		};
	};

	struct MeshRessource
	{
		RenderRessourceHeader header;
		Token(Mesh) mesh;

		inline static MeshRessource build_default()
		{
			return MeshRessource{
					RenderRessourceHeader::build_default(),
					tk_bd(Mesh)
			};
		};
	};

	struct ShaderRessource
	{
		struct Dependencies
		{
			Token(ShaderModuleRessource) vertex_shader;
			Token(ShaderModuleRessource) fragment_shader;
		};

		RenderRessourceHeader header;
		Token(ShaderIndex) shader;

		inline static ShaderRessource build_default()
		{
			return ShaderRessource{
					RenderRessourceHeader::build_default(),
					tk_bd(ShaderIndex)
			};
		};
	};

	struct MaterialRessource
	{
		struct Dependencies
		{
			Token(ShaderRessource) shader;
		};

		RenderRessourceHeader header;
		Token(Material) material;

		inline static MaterialRessource build_default()
		{
			return MaterialRessource{
					RenderRessourceHeader::build_default(),
					tk_bd(Material)
			};
		};
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
		struct NestedDependencies
		{
			ShaderRessource::Dependencies shader_dependencies;
			MaterialRessource::Dependencies material_dependencies;
			Token(MaterialRessource) material;
			Token(MeshRessource) mesh;
		};

		static constexpr component_t Type = HashRaw_constexpr(STR(MeshRendererComponent));
		int8 force_update;
		Token(Node) scene_node;
		Token(RenderableObject) renderable_object;

		inline static MeshRendererComponent build(const Token(Node) p_scene_node)
		{
			return MeshRendererComponent
					{
							1, p_scene_node, tk_bd(RenderableObject)
					};
		};
	};


	enum class RessourceAllocationType
	{
		UNKNOWN = 0,
		DATABASE = 1,
		INLINE = 2
	};

	template<class RessourceDependencies_t, class RessourceAsset_t, class Ressource_t>
	struct RessourceAllocationEvent
	{
		RessourceAllocationType type;
		RessourceDependencies_t dependencies;
		union
		{
			uimax database_id;
			RessourceAsset_t asset;
		};
		Token(Ressource_t) ressource;

		inline static RessourceAllocationEvent<RessourceDependencies_t, RessourceAsset_t, Ressource_t> build_inline(const Token(Ressource_t) p_target_ressource_token, const RessourceDependencies_t& p_dependencies,
				const RessourceAsset_t& p_ressource_asset)
		{
			RessourceAllocationEvent<RessourceDependencies_t, RessourceAsset_t, Ressource_t> l_event;
			l_event.type = RessourceAllocationType::INLINE;
			l_event.dependencies = p_dependencies;
			l_event.asset = p_ressource_asset;
			l_event.ressource = p_target_ressource_token;
			return l_event;
		};
	};

	struct ShaderModuleRessourceAsset
	{
		Span<int8> compiled_shader;
	};

	struct MeshRessourceAsset
	{
		Span<Vertex> initial_vertices;
		Span<uint32> initial_indices;
	};

	struct ShaderRessourceAsset
	{
		Span<ShaderLayoutParameterType> specific_parameters;
		uimax execution_order;
		ShaderConfiguration shader_configuration;
	};

	struct MaterialRessourceAsset
	{
		//TODO adding parameters
	};

	struct RenderHeap
	{
		Pool<ShaderModuleRessource> shader_modules;
		Pool<MeshRessource> mesh;
		Pool<ShaderRessource> shaders;
		Pool<MaterialRessource> materials;
		CameraComponent camera;

		inline static RenderHeap allocate()
		{
			return RenderHeap{
					Pool<ShaderModuleRessource>::allocate(0),
					Pool<MeshRessource>::allocate(0),
					Pool<ShaderRessource>::allocate(0),
					Pool<MaterialRessource>::allocate(0)
			};
		};

		inline void free()
		{
#if RENDER_BOUND_TEST
			assert_true(!this->shader_modules.has_allocated_elements());
			assert_true(!this->mesh.has_allocated_elements());
			assert_true(!this->shaders.has_allocated_elements());
			assert_true(!this->materials.has_allocated_elements());
#endif
			this->shader_modules.free();
			this->mesh.free();
			this->shaders.free();
			this->materials.free();
		};
	};

	struct RenderRessourceAllocator2
	{
		RenderHeap heap;

		struct MeshRendererAllocationEvent
		{
			Token(MeshRendererComponent) mesh_renderer;
		};

		Vector<RessourceAllocationEvent<int8, ShaderModuleRessourceAsset, ShaderModuleRessource>> shadermodule_allocation_events;
		Vector<RessourceAllocationEvent<int8, MeshRessourceAsset, MeshRessource>> mesh_allocation_events;
		Vector<RessourceAllocationEvent<ShaderRessource::Dependencies, ShaderRessourceAsset, ShaderRessource>> shader_allocation_events;
		Vector<RessourceAllocationEvent<MaterialRessource::Dependencies, MaterialRessourceAsset, MaterialRessource>> material_allocation_events;
		Vector<MeshRendererAllocationEvent> mesh_renderer_allocation_events;

		PoolIndexed<MeshRendererComponent> mesh_renderers;
		PoolIndexed<MeshRendererComponent::NestedDependencies> mesh_renderers_dependencies;

		inline static RenderRessourceAllocator2 allocate()
		{
			return RenderRessourceAllocator2{
					RenderHeap::allocate(),
					Vector<RessourceAllocationEvent<int8, ShaderModuleRessourceAsset, ShaderModuleRessource>>::allocate(0),
					Vector<RessourceAllocationEvent<int8, MeshRessourceAsset, MeshRessource>>::allocate(0),
					Vector<RessourceAllocationEvent<ShaderRessource::Dependencies, ShaderRessourceAsset, ShaderRessource>>::allocate(0),
					Vector<RessourceAllocationEvent<MaterialRessource::Dependencies, MaterialRessourceAsset, MaterialRessource>>::allocate(0),
					Vector<MeshRendererAllocationEvent>::allocate(0),
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
			assert_true(!this->mesh_renderers.has_allocated_elements());
			assert_true(!this->mesh_renderers_dependencies.has_allocated_elements());
#endif

			this->shadermodule_allocation_events.free();
			this->mesh_allocation_events.free();
			this->shader_allocation_events.free();
			this->material_allocation_events.free();
			this->mesh_renderer_allocation_events.free();
			this->mesh_renderers.free();
			this->mesh_renderers_dependencies.free();
		};

		inline void step(D3Renderer& p_renderer, GPUContext& p_gpu_context)
		{
			for (loop_reverse(i, 0, this->shadermodule_allocation_events.Size))
			{
				auto& l_event = this->shadermodule_allocation_events.get(i);
				ShaderModuleRessource& l_ressource = this->heap.shader_modules.get(l_event.ressource);
				l_ressource.shader_module = p_gpu_context.graphics_allocator.allocate_shader_module(l_event.asset.compiled_shader.slice);
				l_ressource.header.allocated = 1;
				l_event.asset.compiled_shader.free();
				this->shadermodule_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->mesh_allocation_events.Size))
			{
				auto& l_event = this->mesh_allocation_events.get(i);
				MeshRessource& l_ressource = this->heap.mesh.get(l_event.ressource);
				l_ressource.mesh = D3RendererAllocatorComposition::allocate_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_event.asset.initial_vertices.slice, l_event.asset.initial_indices.slice);
				l_ressource.header.allocated = 1;
				l_event.asset.initial_vertices.free();
				l_event.asset.initial_indices.free();
				this->mesh_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->shader_allocation_events.Size))
			{
				auto& l_event = this->shader_allocation_events.get(i);
				ShaderRessource& l_ressource = this->heap.shaders.get(l_event.ressource);

				ShaderModuleRessource& l_vertex_shader = this->heap.shader_modules.get(l_event.dependencies.vertex_shader);
				ShaderModuleRessource& l_fragment_shader = this->heap.shader_modules.get(l_event.dependencies.fragment_shader);

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
				MaterialRessource& l_ressource = this->heap.materials.get(l_event.ressource);

				ShaderRessource& l_shader = this->heap.shaders.get(l_event.dependencies.shader);

				l_ressource.material = p_renderer.allocator.allocate_material(Material::allocate_empty(p_gpu_context.graphics_allocator, 1));
				p_renderer.allocator.heap.link_shader_with_material(l_shader.shader, l_ressource.material);
				l_ressource.header.allocated = 1;
				this->material_allocation_events.pop_back();
			}

			for (loop_reverse(i, 0, this->mesh_renderer_allocation_events.Size))
			{
				auto& l_event = this->mesh_renderer_allocation_events.get(i);
				MeshRendererComponent::NestedDependencies& l_dependencies = this->mesh_renderers_dependencies.get(tk_bf(MeshRendererComponent::NestedDependencies, l_event.mesh_renderer));
				MeshRendererComponent& l_mesh_renderer = this->mesh_renderers.get(l_event.mesh_renderer);

				l_mesh_renderer.renderable_object = D3RendererAllocatorComposition::allocate_renderable_object_with_buffers(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator,
						this->heap.mesh.get(l_dependencies.mesh).mesh);
				p_renderer.allocator.heap.link_material_with_renderable_object(this->heap.materials.get(l_dependencies.material).material,
						l_mesh_renderer.renderable_object);

				this->mesh_renderer_allocation_events.pop_back();
			}
		};

		inline Token(ShaderModuleRessource) allocate_shadermodule_inline(const ShaderModuleRessourceAsset& p_shadermodule_ressource_asset)
		{
			auto l_event = RessourceAllocationEvent<int8, ShaderModuleRessourceAsset, ShaderModuleRessource>::build_inline(
					this->heap.shader_modules.alloc_element(ShaderModuleRessource::build_default()), 0, p_shadermodule_ressource_asset);
			this->shadermodule_allocation_events.push_back_element(l_event);
			return l_event.ressource;
		};

		inline Token(ShaderRessource) allocate_shader_inline(const ShaderRessourceAsset& p_shader_ressource_asset, const ShaderRessource::Dependencies& p_dependencies)
		{
			auto l_event = RessourceAllocationEvent<ShaderRessource::Dependencies, ShaderRessourceAsset, ShaderRessource>::build_inline(this->heap.shaders.alloc_element(ShaderRessource::build_default()), p_dependencies,
					p_shader_ressource_asset);
			this->shader_allocation_events.push_back_element(l_event);
			return l_event.ressource;
		};

		inline Token(MeshRessource) allocate_mesh_inline(const MeshRessourceAsset& p_mesh_ressource_asset)
		{
			auto l_event = RessourceAllocationEvent<int8, MeshRessourceAsset, MeshRessource>::build_inline(this->heap.mesh.alloc_element(MeshRessource::build_default()), 0,
					p_mesh_ressource_asset);
			this->mesh_allocation_events.push_back_element(l_event);
			return l_event.ressource;
		};

		inline Token(MaterialRessource) allocate_material_inline(const MaterialRessourceAsset& p_material_ressource_asset, const MaterialRessource::Dependencies& p_dependencies)
		{
			auto l_event = RessourceAllocationEvent<MaterialRessource::Dependencies, MaterialRessourceAsset, MaterialRessource>::build_inline(this->heap.materials.alloc_element(MaterialRessource::build_default()), p_dependencies,
					p_material_ressource_asset);
			this->material_allocation_events.push_back_element(l_event);
			return l_event.ressource;
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
				const ShaderModuleRessourceAsset& p_vertex_shader, const ShaderModuleRessourceAsset& p_fragment_shader,
				const ShaderRessourceAsset& p_shader, const MaterialRessourceAsset& p_material, const MeshRessourceAsset& p_mesh, const Token(Node) p_scene_node)
		{
			MeshRendererComponent::NestedDependencies l_mesh_renderer_depencies;

			l_mesh_renderer_depencies.shader_dependencies.vertex_shader = p_render_ressource_allocator.allocate_shadermodule_inline(p_vertex_shader);
			l_mesh_renderer_depencies.shader_dependencies.fragment_shader = p_render_ressource_allocator.allocate_shadermodule_inline(p_fragment_shader);
			l_mesh_renderer_depencies.material_dependencies.shader = p_render_ressource_allocator.allocate_shader_inline(p_shader, l_mesh_renderer_depencies.shader_dependencies);
			l_mesh_renderer_depencies.material = p_render_ressource_allocator.allocate_material_inline(p_material, l_mesh_renderer_depencies.material_dependencies);
			l_mesh_renderer_depencies.mesh = p_render_ressource_allocator.allocate_mesh_inline(p_mesh);

			return p_render_ressource_allocator.allocate_meshrenderer_inline(l_mesh_renderer_depencies, p_scene_node);
		};

		inline static void free_meshrenderer(RenderRessourceAllocator2& p_render_ressource_allocator, D3Renderer& p_renderer, GPUContext& p_gpu_context, const Token(MeshRendererComponent) p_mesh_renderer)
		{
			MeshRendererComponent& l_mesh_renderer = p_render_ressource_allocator.mesh_renderers.get(p_mesh_renderer);
			MeshRendererComponent::NestedDependencies l_mesh_renderer_dependencies = p_render_ressource_allocator.mesh_renderers_dependencies.get(tk_bf(MeshRendererComponent::NestedDependencies, p_mesh_renderer));
			{
				MeshRessource& l_mesh_ressource = p_render_ressource_allocator.heap.mesh.get(l_mesh_renderer_dependencies.mesh);
				if (l_mesh_ressource.header.allocated)
				{
					D3RendererAllocatorComposition::free_mesh_with_buffers(p_gpu_context.buffer_memory, p_renderer.allocator, l_mesh_ressource.mesh);
				}
				p_render_ressource_allocator.heap.mesh.release_element(l_mesh_renderer_dependencies.mesh);


				MaterialRessource& l_material_ressource = p_render_ressource_allocator.heap.materials.get(l_mesh_renderer_dependencies.material);

				if (l_material_ressource.header.allocated)
				{
					p_renderer.allocator.heap.unlink_material_with_renderable_object(l_material_ressource.material, l_mesh_renderer.renderable_object);

					D3RendererAllocatorComposition::free_material_with_parameters(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_material_ressource.material);

					ShaderRessource& l_shader_ressource = p_render_ressource_allocator.heap.shaders.get(l_mesh_renderer_dependencies.material_dependencies.shader);
					if (l_shader_ressource.header.allocated)
					{
						p_renderer.allocator.heap.unlink_shader_with_material(l_shader_ressource.shader, l_material_ressource.material);
						D3RendererAllocatorComposition::free_shader_with_shaderlayout(p_gpu_context.graphics_allocator, p_renderer.allocator, l_shader_ressource.shader);

						ShaderModuleRessource& l_vertex_shader = p_render_ressource_allocator.heap.shader_modules.get(l_mesh_renderer_dependencies.shader_dependencies.vertex_shader);
						if (l_vertex_shader.header.allocated)
						{
							p_gpu_context.graphics_allocator.free_shader_module(l_vertex_shader.shader_module);
						}
						p_render_ressource_allocator.heap.shader_modules.release_element(l_mesh_renderer_dependencies.shader_dependencies.vertex_shader);

						ShaderModuleRessource& l_fragement_shader = p_render_ressource_allocator.heap.shader_modules.get(l_mesh_renderer_dependencies.shader_dependencies.fragment_shader);
						if (l_fragement_shader.header.allocated)
						{
							p_gpu_context.graphics_allocator.free_shader_module(l_fragement_shader.shader_module);
						}
						p_render_ressource_allocator.heap.shader_modules.release_element(l_mesh_renderer_dependencies.shader_dependencies.fragment_shader);
					}

					p_render_ressource_allocator.heap.shaders.release_element(l_mesh_renderer_dependencies.material_dependencies.shader);

				}

				p_render_ressource_allocator.heap.materials.release_element(l_mesh_renderer_dependencies.material);

				D3RendererAllocatorComposition::free_renderable_object_with_buffers(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, p_renderer.allocator, l_mesh_renderer.renderable_object);
			}

			p_render_ressource_allocator.mesh_renderers.release_element(p_mesh_renderer);
			p_render_ressource_allocator.mesh_renderers_dependencies.release_element(tk_bf(MeshRendererComponent::NestedDependencies, p_mesh_renderer));
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