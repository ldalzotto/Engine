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
		Token(GraphicsPass) color_pass;
		Span<v4f> color_pass_clear_values;

		Vector <Token(Shader)> ordered_shaders;
		Pool <Material> materials;
		Pool <RenderableObject> renderable_objects;


		VectorOfVector <Token(Material)> shaders_to_materials;
		PoolOfVector <Token(RenderableObject)> material_to_renderable_objects;

		struct AllocateInfo
		{
			v3ui render_target_dimensions;
		};

		static D3RendererHeap allocate(GPUContext& p_gpu_context, const AllocateInfo& p_allocate_info);

		VectorOfVector<Token(Material) >::Element_ShadowVector get_materials_from_shader_index(const uimax p_shader_index);

		Material& get_material(const Token(Material) p_material);

		PoolOfVector<Token(RenderableObject) >::Element_ShadowVector get_renderableobjects_from_material(const Token(Material) p_material);

		RenderableObject& get_renderableobject(const Token(RenderableObject) p_renderable_object);
	};

	/*
		The D3Renderer is a structure that organize GPU graphics allocated data in a hierarchical way (Shader -> Material -> RenderableObject).
	 	//TODO -> refactoring the GPU memory.hpp to allow material modification with the ShaderLayout only.
	 	//TODO -> adding the global material
	 	//TODO -> update RenderableObject model GPU data
		//TODO -> adding a bind to a index buffer (stored in the Mesh object)
		//TODO -> adding a draw_indexed variant
	*/
	struct D3Renderer
	{

		D3RendererHeap heap;

		static D3Renderer allocate();

		void free();

		void graphics_step(GraphicsBinder& p_graphics_binder);
	};

	D3RendererHeap D3RendererHeap::allocate(GPUContext& p_gpu_context, const D3RendererHeap::AllocateInfo& p_allocate_info)
	{
		RenderPassAttachment l_attachments[2] = {
				RenderPassAttachment{
						AttachmentType::COLOR,
						ImageFormat::build_color_2d(p_allocate_info.render_target_dimensions, ImageUsageFlag::SHADER_COLOR_ATTACHMENT)
				},
				RenderPassAttachment{
						AttachmentType::DEPTH,
						ImageFormat::build_color_2d(p_allocate_info.render_target_dimensions, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)
				}
		};

		D3RendererHeap l_heap;
		l_heap.color_pass = p_gpu_context.graphics_allocator.allocate_graphicspass<2>(p_gpu_context.buffer_allocator, l_attachments);
		return l_heap;
	};

	inline void D3Renderer::graphics_step(GraphicsBinder& p_graphics_binder)
	{
		//TODO -> update RenderableObject model GPU data

		GraphicsBinder l_binder = GraphicsBinder::build(p_graphics_binder.buffer_allocator,p_graphics_binder.graphics_allocator);
		l_binder.begin_render_pass(p_graphics_binder.graphics_allocator.get_graphics_pass(this->heap.color_pass), this->heap.color_pass_clear_values.slice);

		for (loop(i, 0, this->heap.ordered_shaders.Size))
		{
			Token(Shader) l_shader_token = this->heap.ordered_shaders.get(i);
			l_binder.bind_shader(p_graphics_binder.graphics_allocator.get_shader(l_shader_token));

			auto l_materials = this->heap.get_materials_from_shader_index(i);
			for (loop(j, 0, l_materials.get_size()))
			{
				Token(Material) l_material = l_materials.get(j);
				l_binder.bind_material(this->heap.get_material(l_material));

				auto l_renderable_objects = this->heap.get_renderableobjects_from_material(l_material);

				for (loop(k, 0, l_renderable_objects.get_size()))
				{
					Token(RenderableObject) l_renderable_object_token = l_renderable_objects.get(k);

					Mesh& l_mesh = this->heap.get_renderableobject(l_renderable_object_token).mesh;

					//TODO -> adding a bind to a index buffer (stored in the Mesh object)
					//TODO -> adding a draw_indexed variant
					l_binder.bind_vertex_buffer_gpu(p_graphics_binder.buffer_allocator.get_buffergpu(l_mesh.gpu_memory));
					l_binder.draw(l_mesh.vertex_count);
				}

				l_binder.pop_material_bind(this->heap.get_material(l_material));
			}
		}

		l_binder.end_render_pass();
	};

};