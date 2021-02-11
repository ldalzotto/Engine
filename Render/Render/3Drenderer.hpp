#pragma once

namespace v2
{

	struct Camera
	{
		m44f view;
		m44f projection;
	};

	struct Vertex
	{
		v3f position;
		v2f uv;
	};

	struct ShaderIndex
	{
		uimax execution_order;
		Token(Shader) shader_index;
	};

	struct Mesh
	{
		Token(BufferGPU) gpu_memory;
		uimax vertex_count;
	};

	struct RenderableObject
	{
		Token(BufferHost) model;
		Mesh mesh;
	};

	struct D3RendererHeap
	{
		Pool <ShaderIndex> shaders;
		Pool <Material> materials;
		Pool <RenderableObject> renderable_objects;

		Vector <Token(ShaderIndex)> shaders_indexed;
		PoolOfVector <Token(Material)> shaders_to_materials;
		PoolOfVector <Token(RenderableObject)> material_to_renderable_objects;

		struct RenderableObject_ModelUpdateEvent
		{
			Token(RenderableObject) renderable_object;
			m44f model_matrix;
		};

		Vector <RenderableObject_ModelUpdateEvent> model_update_events;

		static D3RendererHeap allocate();

		void free();

		Token(ShaderIndex) push_shader(const ShaderIndex& p_shader);

		void remove_shader(const Token(ShaderIndex) p_shader);

		void link_shader_with_material(const Token(ShaderIndex) p_shader, const Token(Material) p_material);

		PoolOfVector<Token(Material) >::Element_ShadowVector get_materials_from_shader(const Token(ShaderIndex) p_shader);

		Token(Material) push_material(const Material& p_material);

		void remove_material(const Token(Material) p_material);

		Material& get_material(const Token(Material) p_material);

		void link_material_with_renderable_object(const Token(Material) p_material, const Token(RenderableObject) p_renderable_object);

		PoolOfVector<Token(RenderableObject) >::Element_ShadowVector get_renderableobjects_from_material(const Token(Material) p_material);

		Token(RenderableObject) allocate_renderable_object(const RenderableObject& p_renderable_object);

		void free_renderable_object(const Token(RenderableObject) p_rendereable_object);

		RenderableObject& get_renderableobject(const Token(RenderableObject) p_renderable_object);
	};

	struct ColorStep
	{
		Token(GraphicsPass) pass;
		Span<v4f> clear_values;

		Token(ShaderLayout) global_buffer_layout;
		Material global_material;

		struct AllocateInfo
		{
			v3ui render_target_dimensions;
		};

		static ColorStep allocate(GPUContext& p_gpu_context, const AllocateInfo& p_allocate_info);

		void free(GPUContext& p_gpu_context);

	};

	/*
		The D3Renderer is a structure that organize GPU graphics allocated data in a hierarchical way (Shader -> Material -> RenderableObject).
	*/
	struct D3Renderer
	{

		D3RendererHeap heap;
		ColorStep color_step;

		static D3Renderer allocate(GPUContext& p_gpu_context, const ColorStep::AllocateInfo& p_allocation_info);

		void free(GPUContext& p_gpu_context);

		void buffer_step(GPUContext& p_gpu_context);

		void graphics_step(GraphicsBinder& p_graphics_binder);
	};


	inline D3RendererHeap D3RendererHeap::allocate()
	{
		D3RendererHeap l_heap;

		l_heap.shaders = Pool<ShaderIndex>::allocate(0);
		l_heap.materials = Pool<Material>::allocate(0);
		l_heap.renderable_objects = Pool<RenderableObject>::allocate(0);
		l_heap.shaders_indexed = Vector<Token(ShaderIndex) >::allocate(0);
		l_heap.shaders_to_materials = PoolOfVector<Token(Material) >::allocate_default();
		l_heap.material_to_renderable_objects = PoolOfVector<Token(RenderableObject) >::allocate_default();
		l_heap.model_update_events = Vector<RenderableObject_ModelUpdateEvent>::allocate(0);

		return l_heap;
	};

	inline void D3RendererHeap::free()
	{
#if RENDER_BOUND_TEST
		assert_true(!this->shaders.has_allocated_elements());
		assert_true(!this->materials.has_allocated_elements());
		assert_true(!this->renderable_objects.has_allocated_elements());
		assert_true(this->shaders_indexed.empty());
		assert_true(!this->shaders_to_materials.has_allocated_elements());
		assert_true(!this->material_to_renderable_objects.has_allocated_elements());
		assert_true(this->model_update_events.empty());
#endif

		this->shaders.free();
		this->materials.free();
		this->renderable_objects.free();
		this->shaders_indexed.free();
		this->shaders_to_materials.free();
		this->material_to_renderable_objects.free();
		this->model_update_events.free();
	};

	inline Token(ShaderIndex) D3RendererHeap::push_shader(const ShaderIndex& p_shader)
	{
		uimax l_insertion_index = 0;
		for (loop(i, 0, this->shaders_indexed.Size))
		{
			if (this->shaders.get(this->shaders_indexed.get(i)).execution_order >= l_insertion_index)
			{
				break;
			};
		};

		Token(ShaderIndex) l_shader = this->shaders.alloc_element(p_shader);
		this->shaders_to_materials.alloc_vector();

		this->shaders_indexed.insert_element_at_always(l_shader, l_insertion_index);
		return l_shader;
	};

	inline void D3RendererHeap::remove_shader(const Token(ShaderIndex) p_shader)
	{
		ShaderIndex& l_shader = this->shaders.get(p_shader);

		Slice<Token(Material) > l_linked_materials = this->shaders_to_materials.get_vector(tk_bf(Slice<Token(Material)>, l_shader.shader_index));
		for (loop(j, 0, l_linked_materials.Size))
		{
			this->remove_material(l_linked_materials.get(j));
		}
		this->shaders_to_materials.release_vector(tk_bf(Slice<Token(Material)>, l_shader.shader_index));
		this->shaders.release_element(p_shader);
	};

	inline void D3RendererHeap::link_shader_with_material(const Token(ShaderIndex) p_shader, const Token(Material) p_material)
	{
		this->shaders_to_materials.element_push_back_element(tk_bf(Slice<Token(Material)>, p_shader), p_material);
	};

	inline PoolOfVector<Token(Material) >::Element_ShadowVector D3RendererHeap::get_materials_from_shader(const Token(ShaderIndex) p_shader)
	{
		return this->shaders_to_materials.get_element_as_shadow_vector(tk_bf(Slice<Token(Material)>, p_shader));
	};

	inline Token(Material) D3RendererHeap::push_material(const Material& p_material)
	{
		this->material_to_renderable_objects.alloc_vector();
		return this->materials.alloc_element(p_material);
	};

	inline void D3RendererHeap::remove_material(const Token(Material) p_material)
	{
		this->material_to_renderable_objects.release_vector(tk_bf(Slice<Token(RenderableObject)>, p_material));
		this->materials.release_element(p_material);
	};

	inline Material& D3RendererHeap::get_material(const Token(Material) p_material)
	{
		return this->materials.get(p_material);
	};

	inline void D3RendererHeap::link_material_with_renderable_object(const Token(Material) p_material, const Token(RenderableObject) p_renderable_object)
	{
		this->material_to_renderable_objects.element_push_back_element(tk_bf(Slice<Token(RenderableObject)>, p_material), p_renderable_object);
	};

	inline PoolOfVector<Token(RenderableObject) >::Element_ShadowVector D3RendererHeap::get_renderableobjects_from_material(const Token(Material) p_material)
	{
		return this->material_to_renderable_objects.get_element_as_shadow_vector(tk_bf(Slice<Token(RenderableObject)>, p_material));
	};

	inline Token(RenderableObject) D3RendererHeap::allocate_renderable_object(const RenderableObject& p_renderable_object)
	{
		return this->renderable_objects.alloc_element(p_renderable_object);
	};

	inline void D3RendererHeap::free_renderable_object(const Token(RenderableObject) p_rendereable_object)
	{
		this->renderable_objects.release_element(p_rendereable_object);
	};

	inline RenderableObject& D3RendererHeap::get_renderableobject(const Token(RenderableObject) p_renderable_object)
	{
		return this->renderable_objects.get(p_renderable_object);
	};

	inline ColorStep ColorStep::allocate(GPUContext& p_gpu_context, const AllocateInfo& p_allocate_info)
	{
		ColorStep l_step;

		Span<ShaderLayoutParameterType> l_global_buffer_parameters = Span<ShaderLayoutParameterType>::allocate(1);
		l_global_buffer_parameters.get(0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX;
		Span<ShaderLayout::VertexInputParameter> l_global_buffer_vertices_parameters = Span<ShaderLayout::VertexInputParameter>::build(NULL, 0);


		RenderPassAttachment l_attachments[2] = {
				RenderPassAttachment{
						AttachmentType::COLOR,
						ImageFormat::build_color_2d(p_allocate_info.render_target_dimensions, ImageUsageFlag::SHADER_COLOR_ATTACHMENT)
				},
				RenderPassAttachment{
						AttachmentType::DEPTH,
						ImageFormat::build_depth_2d(p_allocate_info.render_target_dimensions, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)
				}
		};

		v4f l_clears[2] = { v4f{ 0.0f, 0.0f, 0.0f, 0.0f }, v4f{ 0.0f, 0.0f, 0.0f, 0.0f }};
		l_step.clear_values = Span<v4f>::allocate_array<2>(l_clears);
		l_step.pass = p_gpu_context.graphics_allocator.allocate_graphicspass<2>(p_gpu_context.buffer_allocator, l_attachments);
		l_step.global_buffer_layout = p_gpu_context.graphics_allocator.allocate_shader_layout(l_global_buffer_parameters, l_global_buffer_vertices_parameters, 0);

		Camera l_empty_camera{};
		l_step.global_material = Material::allocate_empty(p_gpu_context.graphics_allocator, 0);
		l_step.global_material.add_and_allocate_buffer_host_parameter_typed(p_gpu_context.graphics_allocator, p_gpu_context.buffer_allocator,
				p_gpu_context.graphics_allocator.get_shader_layout(l_step.global_buffer_layout), l_empty_camera);

		return l_step;
	};

	inline void ColorStep::free(GPUContext& p_gpu_context)
	{
		this->clear_values.free();
		p_gpu_context.graphics_allocator.free_shader_layout(this->global_buffer_layout);
		p_gpu_context.graphics_allocator.free_graphicspass(p_gpu_context.buffer_allocator, this->pass);
		this->global_material.free(p_gpu_context.graphics_allocator, p_gpu_context.buffer_allocator);
	};

	inline D3Renderer D3Renderer::allocate(GPUContext& p_gpu_context, const ColorStep::AllocateInfo& p_allocation_info)
	{
		return D3Renderer{
				D3RendererHeap::allocate(),
				ColorStep::allocate(p_gpu_context, p_allocation_info)
		};
	};

	inline void D3Renderer::free(GPUContext& p_gpu_context)
	{
		this->heap.free();
		this->color_step.free(p_gpu_context);
	};

	inline void D3Renderer::buffer_step(GPUContext& p_gpu_context)
	{
		for (loop(i, 0, this->heap.model_update_events.Size))
		{
			auto& l_event = this->heap.model_update_events.get(i);
			slice_memcpy(
					p_gpu_context.buffer_allocator.get_bufferhost(this->heap.get_renderableobject(l_event.renderable_object).model).get_mapped_memory(),
					Slice<m44f>::build_asint8_memory_singleelement(&l_event.model_matrix)
			);
		};

		this->heap.model_update_events.clear();
	};

	inline void D3Renderer::graphics_step(GraphicsBinder& p_graphics_binder)
	{
		p_graphics_binder.bind_shader_layout(p_graphics_binder.graphics_allocator.get_shader_layout(this->color_step.global_buffer_layout));
		p_graphics_binder.bind_material(this->color_step.global_material);

		p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.get_graphics_pass(this->color_step.pass), this->color_step.clear_values.slice);

		for (loop(i, 0, this->heap.shaders_indexed.Size))
		{
			Token(ShaderIndex) l_shader_token = this->heap.shaders_indexed.get(i);
			ShaderIndex& l_shader_index = this->heap.shaders.get(l_shader_token);
			p_graphics_binder.bind_shader(p_graphics_binder.graphics_allocator.get_shader(l_shader_index.shader_index));

			auto l_materials = this->heap.get_materials_from_shader(l_shader_token);
			for (loop(j, 0, l_materials.get_size()))
			{
				Token(Material) l_material = l_materials.get(j);
				p_graphics_binder.bind_material(this->heap.get_material(l_material));

				auto l_renderable_objects = this->heap.get_renderableobjects_from_material(l_material);

				for (loop(k, 0, l_renderable_objects.get_size()))
				{
					Token(RenderableObject) l_renderable_object_token = l_renderable_objects.get(k);

					Mesh& l_mesh = this->heap.get_renderableobject(l_renderable_object_token).mesh;

					//TODO -> adding a bind to a index buffer (stored in the Mesh object)
					//TODO -> adding a draw_indexed variant
					p_graphics_binder.bind_vertex_buffer_gpu(p_graphics_binder.buffer_allocator.get_buffergpu(l_mesh.gpu_memory));
					p_graphics_binder.draw(l_mesh.vertex_count);
				}

				p_graphics_binder.pop_material_bind(this->heap.get_material(l_material));
			}
		}

		p_graphics_binder.end_render_pass();

		p_graphics_binder.pop_material_bind(this->color_step.global_material);
	};

};