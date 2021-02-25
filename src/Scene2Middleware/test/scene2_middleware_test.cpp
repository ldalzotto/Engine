

#include "Scene2Middleware/scene2_middleware.hpp"
#include "shader_compiler.hpp"


namespace v2
{

	inline void ressource_componsition_test()
	{
		struct RessourceTest
		{
			RessourceIdentifiedHeader header;

			struct Asset
			{
				uint8 free_called;

				inline static Asset build_default()
				{
					return Asset{ 0 };
				};

				inline void free()
				{
					this->free_called = 1;
				};
			};

			struct AllocateEvent
			{
				Token(RessourceTest) allocated_ressource;
				Asset asset;
			};

			struct FreeEvent
			{

			};
		};

		PoolHashedCounted<hash_t, RessourceTest> hashed_counted_ressources = PoolHashedCounted<hash_t, RessourceTest>::allocate_default();
		Vector<RessourceTest::AllocateEvent> ressource_allocation_events = Vector<RessourceTest::AllocateEvent>::allocate(0);
		Vector<RessourceTest::FreeEvent> ressource_free_events = Vector<RessourceTest::FreeEvent>::allocate(0);

		hash_t l_ressource_id = 10;

		{
			RessourceComposition::allocate_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, l_ressource_id, [](const hash_t p_key)
			{
				return RessourceTest{ RessourceIdentifiedHeader::build_with_id(p_key) };
			}, [](const Token(RessourceTest) p_allocated_ressource_token)
			{
				return RessourceTest::AllocateEvent{ p_allocated_ressource_token, RessourceTest::Asset::build_default() };
			});

			assert_true(ressource_allocation_events.Size == 1);
			assert_true(tk_eq(ressource_allocation_events.get(0).allocated_ressource, tk_b(RessourceTest, 0)));
			assert_true(hashed_counted_ressources.has_key_nothashed(l_ressource_id));
			assert_true(hashed_counted_ressources.CountMap.get_value_nothashed(10)->counter == 1);
		}

		auto l_allocate_event_builder = [](const Token(RessourceTest) p_allocated_ressource_token)
		{
			return RessourceTest::AllocateEvent{ p_allocated_ressource_token, RessourceTest::Asset::build_default() };
		};
		auto l_free_event_builder = [](const Token(RessourceTest) p_allocated_ressource_token)
		{
			return RessourceTest::FreeEvent{};
		};

		Token(RessourceTest) l_allocated_ressource;
		{
			l_allocated_ressource = RessourceComposition::allocate_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, l_ressource_id, [](const hash_t p_key)
			{
				return RessourceTest{};
			}, l_allocate_event_builder);

			assert_true(ressource_allocation_events.Size == 1);
			assert_true(tk_eq(ressource_allocation_events.get(0).allocated_ressource, l_allocated_ressource));
			assert_true(hashed_counted_ressources.has_key_nothashed(l_ressource_id));
			assert_true(hashed_counted_ressources.CountMap.get_value_nothashed(10)->counter == 2);
		}

		{
			RessourceTest& l_ressource = hashed_counted_ressources.pool.get(l_allocated_ressource);
			RessourceComposition::free_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, ressource_free_events, l_ressource.header,
					l_free_event_builder, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});

			assert_true(ressource_free_events.Size == 0);
			assert_true(hashed_counted_ressources.CountMap.get_value_nothashed(10)->counter == 1);
		}


		//If the ressource has not been already allocated, then the allocation event is removed, but no free event is generated
		{
			RessourceTest& l_ressource = hashed_counted_ressources.pool.get(l_allocated_ressource);
			RessourceComposition::free_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, ressource_free_events, l_ressource.header,
					l_free_event_builder, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});

			assert_true(ressource_allocation_events.Size == 0);
			assert_true(ressource_free_events.Size == 0);
			assert_true(!hashed_counted_ressources.has_key_nothashed(10));
		}

		//If the ressource has already been allocated, then a free vent is generated
		{
			assert_true(ressource_free_events.Size == 0);

			l_allocated_ressource = RessourceComposition::allocate_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, l_ressource_id, [](const hash_t p_key)
			{
				return RessourceTest{ RessourceIdentifiedHeader::build_with_id(p_key) };
			}, l_allocate_event_builder);

			assert_true(ressource_allocation_events.Size == 1);

			RessourceTest& l_ressource = hashed_counted_ressources.pool.get(l_allocated_ressource);
			l_ressource.header.allocated = 1;

			ressource_allocation_events.clear();

			RessourceComposition::free_ressource_composition_explicit(hashed_counted_ressources, ressource_allocation_events, ressource_free_events, l_ressource.header,
					l_free_event_builder, RessourceComposition::AllocationEventFoundSlot::FreeAsset{});

			assert_true(ressource_free_events.Size == 1);

		}

		ressource_free_events.free();
		ressource_allocation_events.free();
		hashed_counted_ressources.free();
	};


	struct ComponentReleaser2
	{
		Collision2& collision;
		D3Renderer& renderer;
		GPUContext& gpu_ctx;
		SceneMiddleware* scene_middleware;

		inline void on_component_removed(Scene* p_scene, const NodeEntry& p_node, const NodeComponent& p_component)
		{
			on_node_component_removed(this->scene_middleware, this->collision, this->renderer, this->gpu_ctx, p_component);
		};

	};

	struct Scene2MiddlewareContext
	{
		Scene scene;
		Collision2 collision;
		GPUContext gpu_ctx;
		D3Renderer renderer;
		SceneMiddleware scene_middleware;

		inline static Scene2MiddlewareContext allocate()
		{
			Scene l_scene = Scene::allocate_default();
			Collision2 l_collision = Collision2::allocate();
			GPUContext l_gpu_ctx = GPUContext::allocate();
			D3Renderer l_renderer = D3Renderer::allocate(l_gpu_ctx, ColorStep::AllocateInfo{ v3ui{ 8, 8, 1 }, 0 });
			SceneMiddleware l_scene_middleware = SceneMiddleware::allocate_default();
			return Scene2MiddlewareContext{
					l_scene, l_collision, l_gpu_ctx, l_renderer, l_scene_middleware
			};
		};

		inline void free(ComponentReleaser2& p_component_releaser)
		{
			this->scene.consume_component_events_stateful(p_component_releaser);
			this->scene_middleware.free(&this->scene, this->collision, this->renderer, this->gpu_ctx);
			this->collision.free();
			this->renderer.free(this->gpu_ctx);
			this->gpu_ctx.free();
			this->scene.free();
		};
	};

	inline void collision_middleware_component_allocation()
	{
		Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
		ComponentReleaser2 l_component_releaser = ComponentReleaser2{ l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, &l_ctx.scene_middleware };;
		/*
			We allocate the BoxCollider component with the "deferred" path.
			  - The BoxColliderComponentAsset is descontructible even if the CollisionMiddleware step is not called.
			We step the scene_middleware.
			  - The BoxColliderComponentAsset is still descontructible.
			  - But there is no more pending BoxCollider ressource allocation in the collision_middleware.
			The component associated to the BoxCollider component is removed.
			We consume the scene deletion exent.
		*/
		{
			v3f l_half_extend = { 1.0f, 2.0f, 3.0f };
			Token(Node) l_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(BoxColliderComponent) l_box_collider_component = l_ctx.scene_middleware.collision_middleware.allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{ l_half_extend });

			NodeComponent l_box_collider_node_component = BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component);
			l_ctx.scene.add_node_component_by_value(l_node, l_box_collider_node_component);
			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);

			{
				BoxColliderComponentAsset l_box_collider_component_asset =
						BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_ctx.scene_middleware, l_ctx.collision, l_box_collider_node_component);
				assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
				assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 1);
			}
			{
				l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);
				BoxColliderComponentAsset l_box_collider_component_asset =
						BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_ctx.scene_middleware, l_ctx.collision, l_box_collider_node_component);
				assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
				assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 0);
			}

			l_ctx.scene.remove_node_component_typed<BoxColliderComponent>(l_node);
			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);

		}


		{
			v3f l_half_extend = { 1.0f, 2.0f, 3.0f };
			Token(Node) l_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(BoxColliderComponent) l_box_collider_component = l_ctx.scene_middleware.collision_middleware.allocator.allocate_box_collider_component(l_ctx.collision, l_node, BoxColliderComponentAsset{ l_half_extend });

			NodeComponent l_box_collider_node_component = BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component);
			l_ctx.scene.add_node_component_by_value(l_node, l_box_collider_node_component);
			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);

			{
				BoxColliderComponentAsset l_box_collider_component_asset = BoxColliderComponentAsset_SceneCommunication::desconstruct_nodecomponent(l_ctx.scene_middleware, l_ctx.collision, l_box_collider_node_component);
				assert_true(l_box_collider_component_asset.half_extend == l_half_extend);
				assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_colliders_waiting_for_allocation.Size == 0);
			}

			l_ctx.scene.remove_node_component_typed<BoxColliderComponent>(l_node);
			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(l_component_releaser);
		}

		l_ctx.free(l_component_releaser);
	};

	inline void collision_middleware_queuing_for_calculation()
	{
		Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
		ComponentReleaser2 component_releaser = ComponentReleaser2{ l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, &l_ctx.scene_middleware };

		{
			v3f l_half_extend = { 1.0f, 2.0f, 3.0f };
			Token(Node) l_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(BoxColliderComponent) l_box_collider_component = l_ctx.scene_middleware.collision_middleware.allocator.allocate_box_collider_component_deferred(l_node, BoxColliderComponentAsset{ l_half_extend });
			l_ctx.scene.add_node_component_by_value(l_node, BoxColliderComponentAsset_SceneCommunication::construct_nodecomponent(l_box_collider_component));

			assert_true(!l_ctx.scene_middleware.collision_middleware.allocator.box_collider_is_queued_for_detection(l_ctx.collision, l_box_collider_component));

			l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);

			assert_true(l_ctx.scene_middleware.collision_middleware.allocator.box_collider_is_queued_for_detection(l_ctx.collision, l_box_collider_component));

			l_ctx.scene.remove_node_component_typed<BoxColliderComponent>(l_node);
		}

		l_ctx.free(component_releaser);
	};

	inline void render_middleware_inline_allocation()
	{
		Scene2MiddlewareContext l_ctx = Scene2MiddlewareContext::allocate();
		ComponentReleaser2 component_releaser = ComponentReleaser2{ l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx, &l_ctx.scene_middleware };
		{
			Token(Node) l_node_1 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(Node) l_node_2 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			Token(Node) l_node_3 = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);

			const int8* p_vertex_litteral =
					MULTILINE(\
                #version 450 \n

							layout(location = 0) in vec3 pos; \n
							layout(location = 1) in vec2 uv; \n

							struct Camera \n
					{ \n
							mat4 view; \n
							mat4 projection; \n
					}; \n

							layout(set = 0, binding = 0) uniform camera { Camera cam; }; \n
							layout(set = 1, binding = 0) uniform model { mat4 mod; }; \n

							void main()\n
					{ \n
							gl_Position = cam.projection * (cam.view * (mod * vec4(pos.xyz, 1.0f)));\n
					}\n
					);

			const int8* p_fragment_litteral =
					MULTILINE(\
                #version 450\n

							layout(location = 0) out vec4 outColor;\n

							void main()\n
					{ \n
							outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n
					}\n
					);

			ShaderCompiled l_vertex_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
			ShaderCompiled l_fragment_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

			Span<int8> l_compiled_vertex = Span<int8>::allocate_slice(l_vertex_shader_compiled.get_compiled_binary());
			Span<int8> l_compiled_fragment = Span<int8>::allocate_slice(l_fragment_shader_compiled.get_compiled_binary());

			l_vertex_shader_compiled.free();
			l_fragment_shader_compiled.free();

			Span<ShaderLayoutParameterType> l_shader_parameter_layout = Span<ShaderLayoutParameterType>::allocate_slice(SliceN<ShaderLayoutParameterType, 2>{
					ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::TEXTURE_FRAGMENT
			}.to_slice());

			v3f l_positions[8] = {
					v3f{ -1.0f, -1.0f, 1.0f },
					v3f{ -1.0f, 1.0f, 1.0f },
					v3f{ -1.0f, -1.0f, -1.0f },
					v3f{ -1.0f, 1.0f, -1.0f },
					v3f{ 1.0f, -1.0f, 1.0f },
					v3f{ 1.0f, 1.0f, 1.0f },
					v3f{ 1.0f, -1.0f, -1.0f },
					v3f{ 1.0f, 1.0f, -1.0f }
			};

			v2f l_uvs[14] = {
					v2f{ 0.625f, 0.0f },
					v2f{ 0.375f, 0.25f },
					v2f{ 0.375f, 0.0f },
					v2f{ 0.625f, 0.25f },
					v2f{ 0.375f, 0.5f },
					v2f{ 0.625f, 0.5f },
					v2f{ 0.375f, 0.75f },
					v2f{ 0.625f, 0.75f },
					v2f{ 0.375f, 1.00f },
					v2f{ 0.125f, 0.75f },
					v2f{ 0.125f, 0.50f },
					v2f{ 0.875f, 0.50f },
					v2f{ 0.625f, 1.00f },
					v2f{ 0.875f, 0.75f }
			};

			Vertex l_vertices[14] = {
					Vertex{ l_positions[1], l_uvs[0] },
					Vertex{ l_positions[2], l_uvs[1] },
					Vertex{ l_positions[0], l_uvs[2] },
					Vertex{ l_positions[3], l_uvs[3] },
					Vertex{ l_positions[6], l_uvs[4] },
					Vertex{ l_positions[7], l_uvs[5] },
					Vertex{ l_positions[4], l_uvs[6] },
					Vertex{ l_positions[5], l_uvs[7] },
					Vertex{ l_positions[0], l_uvs[8] },
					Vertex{ l_positions[0], l_uvs[9] },
					Vertex{ l_positions[2], l_uvs[10] },
					Vertex{ l_positions[3], l_uvs[11] },
					Vertex{ l_positions[1], l_uvs[12] },
					Vertex{ l_positions[1], l_uvs[13] }
			};
			uint32 l_indices[14 * 3] = {
					0, 1, 2,
					3, 4, 1,
					5, 6, 4,
					7, 8, 6,
					4, 9, 10,
					11, 7, 5,
					0, 3, 1,
					3, 5, 4,
					5, 7, 6,
					7, 12, 8,
					4, 6, 9,
					11, 13, 7
			};

			Span<Vertex> l_vertices_span = Span<Vertex>::allocate_slice(Slice<Vertex>::build_memory_elementnb(l_vertices, 14));
			Span<uint32> l_indices_span = Span<uint32>::allocate_slice(Slice<uint32>::build_memory_elementnb(l_indices, 14 * 3));

			hash_t l_vertex_shader_id = 12;
			ShaderModuleRessource::Asset l_vertex_shader = ShaderModuleRessource::Asset{ l_compiled_vertex };

			hash_t l_fragment_shader_id = 14;
			ShaderModuleRessource::Asset l_fragment_shader = ShaderModuleRessource::Asset{ l_compiled_fragment };

			hash_t l_shader_asset_id = 1482658;
			ShaderRessource::Asset l_shader_asset = ShaderRessource::Asset{
					l_shader_parameter_layout,
					0,
					ShaderConfiguration{ 1, ShaderConfiguration::CompareOp::LessOrEqual }
			};

			hash_t l_mesh_id = 1486;
			MeshRessource::Asset l_mesh_asset = MeshRessource::Asset{
					l_vertices_span, l_indices_span
			};

			Token(MeshRendererComponent) l_mesh_renderer = RenderRessourceAllocator2Composition::allocate_meshrenderer_inline_with_dependencies(l_ctx.scene_middleware.render_middleware.allocator,
					ShaderModuleRessource::InlineAllocationInput{ l_vertex_shader_id, l_vertex_shader },
					ShaderModuleRessource::InlineAllocationInput{ l_fragment_shader_id, l_fragment_shader },
					ShaderRessource::InlineAllocationInput{ l_shader_asset_id, l_shader_asset },
					MaterialRessource::InlineRessourceInput{ 0 },
					MeshRessource::InlineAllocationInput{ l_mesh_id, l_mesh_asset },
					l_node_1
			);
			l_ctx.scene.add_node_component_by_value(l_node_1, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer));

			hash_t l_material_texture_id = 14874879;
			Span<int8> l_material_texture_span = Span<int8>::allocate(8 * 8 * 4);
			TextureRessource::Asset l_material_texture_asset = TextureRessource::Asset{
					v3ui{ 8, 8, 1 },
					l_material_texture_span
			};


			MaterialRessource::Asset l_material_asset;
			{
				Span<int8> l_material_parameter_temp = Span<int8>::allocate(10);
				auto l_obj = MaterialRessource::Asset::ParameterType::OBJECT;
				auto l_tex = MaterialRessource::Asset::ParameterType::TEXTURE;
				l_material_asset.parameters = VaryingVector::allocate_default();
				l_material_asset.parameters.push_back_2(Slice<MaterialRessource::Asset::ParameterType>::build_asint8_memory_singleelement(&l_obj), l_material_parameter_temp.slice);
				l_material_asset.parameters.push_back_2(Slice<MaterialRessource::Asset::ParameterType>::build_asint8_memory_singleelement(&l_tex), Slice<hash_t>::build_asint8_memory_singleelement(&l_material_texture_id));
				l_material_parameter_temp.free();
			}


			Token(MeshRendererComponent) l_mesh_renderer_2 = RenderRessourceAllocator2Composition::allocate_meshrenderer_inline_with_dependencies(l_ctx.scene_middleware.render_middleware.allocator,
					ShaderModuleRessource::InlineAllocationInput{ l_vertex_shader_id, l_vertex_shader },
					ShaderModuleRessource::InlineAllocationInput{ l_fragment_shader_id, l_fragment_shader },
					ShaderRessource::InlineAllocationInput{ l_shader_asset_id, l_shader_asset },
					MaterialRessource::InlineRessourceInput{ 1, l_material_asset,
															 SliceN<TextureRessource::InlineRessourceInput, 1>{
																	 TextureRessource::InlineRessourceInput{
																			 l_material_texture_id,
																			 l_material_texture_asset
																	 }
															 }.to_slice() },
					MeshRessource::InlineAllocationInput{ l_mesh_id, l_mesh_asset },
					l_node_2
			);
			l_ctx.scene.add_node_component_by_value(l_node_2, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer_2));


			Token(Node) l_camera_node = l_ctx.scene.add_node(transform_const::ORIGIN, Scene_const::root_node);
			l_ctx.scene_middleware.render_middleware.allocator.allocate_camera_inline(
					CameraComponent::Asset{ 10.0f, 2.0f, 100.0f }, l_camera_node
			);
			l_ctx.scene.add_node_component_by_value(l_camera_node, CameraComponentAsset_SceneCommunication::construct_nodecomponent());


			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);
			l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);

			/*
				l_node_1 and l_node_2 have a MeshRenderer component attached.
			 	They use the same Shader but have different Materials.
			 	They use the same Mesh.
			 	They are allocated and a step is executing -> render ressources will be allocated.
			*/
			{
				{
					MeshRendererComponent& l_mesh_renderer_ressource = l_ctx.scene_middleware.render_middleware.allocator.mesh_renderers.get(l_mesh_renderer);
					assert_true(l_mesh_renderer_ressource.allocated);
					assert_true(tk_eq(l_mesh_renderer_ressource.scene_node, l_node_1));
					MaterialRessource& l_material = l_ctx.scene_middleware.render_middleware.allocator.heap.materials.pool.get(l_mesh_renderer_ressource.dependencies.material);
					assert_true(l_material.header.id == 0);
					assert_true(l_material.header.allocated == 1);
					ShaderRessource& l_shader = l_ctx.scene_middleware.render_middleware.allocator.heap.shaders_v3.pool.get(l_material.dependencies.shader);
					assert_true(l_shader.header.id == l_shader_asset_id);
					assert_true(l_shader.header.allocated == 1);
					MeshRessource& l_mesh = l_ctx.scene_middleware.render_middleware.allocator.heap.mesh_v2.pool.get(l_mesh_renderer_ressource.dependencies.mesh);
					assert_true(l_mesh.header.id == l_mesh_id);
					assert_true(l_mesh.header.allocated == 1);
				}
				{
					MeshRendererComponent& l_mesh_renderer_ressource_2 = l_ctx.scene_middleware.render_middleware.allocator.mesh_renderers.get(l_mesh_renderer_2);
					assert_true(l_mesh_renderer_ressource_2.allocated);
					assert_true(tk_eq(l_mesh_renderer_ressource_2.scene_node, l_node_2));
					MaterialRessource& l_material = l_ctx.scene_middleware.render_middleware.allocator.heap.materials.pool.get(l_mesh_renderer_ressource_2.dependencies.material);
					assert_true(l_material.header.id == 1);
					assert_true(l_material.header.allocated == 1);
					ShaderRessource& l_shader = l_ctx.scene_middleware.render_middleware.allocator.heap.shaders_v3.pool.get(l_material.dependencies.shader);
					assert_true(l_shader.header.id == l_shader_asset_id);
					assert_true(l_shader.header.allocated == 1);
					MeshRessource& l_mesh = l_ctx.scene_middleware.render_middleware.allocator.heap.mesh_v2.pool.get(l_mesh_renderer_ressource_2.dependencies.mesh);
					assert_true(l_mesh.header.id == l_mesh_id);
					assert_true(l_mesh.header.allocated == 1);
				}
			}

			Token(MeshRendererComponent) l_mesh_renderer_3 = RenderRessourceAllocator2Composition::allocate_meshrenderer_inline_with_dependencies(l_ctx.scene_middleware.render_middleware.allocator,
					ShaderModuleRessource::InlineAllocationInput{ l_vertex_shader_id, l_vertex_shader },
					ShaderModuleRessource::InlineAllocationInput{ l_fragment_shader_id, l_fragment_shader },
					ShaderRessource::InlineAllocationInput{ l_shader_asset_id, l_shader_asset },
					MaterialRessource::InlineRessourceInput{ 0 },
					MeshRessource::InlineAllocationInput{ l_mesh_id, l_mesh_asset },
					l_node_3
			);

			l_ctx.scene.add_node_component_by_value(l_node_3, MeshRendererComponentAsset_SceneCommunication::construct_nodecomponent(l_mesh_renderer_3));
			l_ctx.scene.remove_node(l_ctx.scene.get_node(l_node_3));

			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);

			/*
				We have created a mesh_renderer but deleted on the same frame.
			 	No render module allocation must occurs.
			 	The MeshRenderer has been removed from the RenderRessourceAllocator.
			*/
			{
				assert_true(l_ctx.scene_middleware.render_middleware.allocator.material_allocation_events.Size == 0);
				assert_true(l_ctx.scene_middleware.render_middleware.allocator.mesh_renderer_allocation_events.Size == 0);
				assert_true(l_ctx.scene_middleware.render_middleware.allocator.mesh_renderers.Memory.is_element_free(l_mesh_renderer_3));
			}

			l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);

			l_ctx.scene.remove_node(l_ctx.scene.get_node(l_node_2));

			l_ctx.scene.consume_component_events_stateful<ComponentReleaser2>(component_releaser);
			l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);

			// We removed the l_node_2. The l_node_1 is still here and common ressources still allocated
			{
				assert_true(l_ctx.scene_middleware.render_middleware.allocator.mesh_renderers.Memory.is_element_free(l_mesh_renderer_2));

				MeshRendererComponent& l_mesh_renderer_ressource = l_ctx.scene_middleware.render_middleware.allocator.mesh_renderers.get(l_mesh_renderer);
				assert_true(l_mesh_renderer_ressource.allocated);
				assert_true(tk_eq(l_mesh_renderer_ressource.scene_node, l_node_1));
				MaterialRessource& l_material = l_ctx.scene_middleware.render_middleware.allocator.heap.materials.pool.get(l_mesh_renderer_ressource.dependencies.material);
				assert_true(l_material.header.id == 0);
				assert_true(l_material.header.allocated == 1);
				ShaderRessource& l_shader = l_ctx.scene_middleware.render_middleware.allocator.heap.shaders_v3.pool.get(l_material.dependencies.shader);
				assert_true(l_shader.header.id == l_shader_asset_id);
				assert_true(l_shader.header.allocated == 1);
				MeshRessource& l_mesh = l_ctx.scene_middleware.render_middleware.allocator.heap.mesh_v2.pool.get(l_mesh_renderer_ressource.dependencies.mesh);
				assert_true(l_mesh.header.id == l_mesh_id);
				assert_true(l_mesh.header.allocated == 1);
			}

			l_ctx.scene.remove_node(l_ctx.scene.get_node(l_node_1));
			l_ctx.scene.remove_node(l_ctx.scene.get_node(l_camera_node));
		}

		l_ctx.scene_middleware.step(&l_ctx.scene, l_ctx.collision, l_ctx.renderer, l_ctx.gpu_ctx);

		l_ctx.free(component_releaser);
	};
};

int main()
{
	v2::ressource_componsition_test();
	v2::collision_middleware_component_allocation();
	v2::collision_middleware_queuing_for_calculation();
	v2::render_middleware_inline_allocation();
	memleak_ckeck();
}