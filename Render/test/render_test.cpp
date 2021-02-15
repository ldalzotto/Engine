
#include "Render/render.hpp"

// #define RENDER_DOC_DEBUG

#ifdef RENDER_DOC_DEBUG

#include "TestCommon/renderdoc_app.h"

RENDERDOC_API_1_1_0* rdoc_api = NULL;

#endif


namespace v2
{

	inline void test()
	{
		GPUContext l_ctx = GPUContext::allocate();
		D3Renderer l_renderer = D3Renderer::allocate(l_ctx, ColorStep::AllocateInfo{ v3ui{ 8, 8, 1 }});

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_ctx.buffer_allocator.device.device, NULL);
#endif

		const int8* p_vertex_litteral =
				MULTILINE(\
                    #version 450 \n

						layout(location = 0) in vec3 pos; \n

						struct Camera \n
				{ \n
						mat4 view; \n
						mat4 projection; \n
				}; \n

						layout(set = 0, binding = 0) uniform camera { Camera cam; }; \n
						layout(set = 2, binding = 0) uniform model { mat4 mod; }; \n

						void main()\n
				{ \n
						gl_Position = cam.projection * (cam.view * (mod * vec4(pos.xyz, 1.0f)));\n
				}\n
				);

		const int8* p_fragment_litteral =
				MULTILINE(\
                    #version 450\n

						layout(location = 0) out vec4 outColor;\n

						layout(set = 1, binding = 0) uniform color { vec3 col; }; \n

						void main()\n
				{ \n
						outColor = vec4(col.xyz, 1.0f);\n
				}\n
				);

		ShaderCompiled l_vertex_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
		ShaderCompiled l_fragment_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

		Token(ShaderLayout) l_shader_layout;

		{
			ShaderLayoutParameterType l_layout_parameter_types[3] = { ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX };
			Span<ShaderLayoutParameterType> l_layout_parameter_types_span = Span<ShaderLayoutParameterType>::allocate_array<3>(l_layout_parameter_types);
			ShaderLayout::VertexInputParameter l_layout_verex_input[1] = { ShaderLayout::VertexInputParameter{ PrimitiveSerializedTypes::Type::FLOAT32_3, 0 }};
			Span<ShaderLayout::VertexInputParameter> l_layout_verex_input_span = Span<ShaderLayout::VertexInputParameter>::allocate_array<1>(l_layout_verex_input);
			l_shader_layout = l_ctx.graphics_allocator.allocate_shader_layout(l_layout_parameter_types_span, l_layout_verex_input_span, sizeof(v3f));
		}

		Token(ShaderModule) l_vertex_shader_module = l_ctx.graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
		Token(ShaderModule) l_fragment_shader_module = l_ctx.graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

		l_vertex_shader_compiled.free();
		l_fragment_shader_compiled.free();


		ShaderAllocateInfo l_shader_allocate_info{
				l_ctx.graphics_allocator.get_graphics_pass(l_renderer.color_step.pass),
				ShaderConfiguration{ 1, ShaderConfiguration::CompareOp::GreaterOrEqual },
				l_ctx.graphics_allocator.get_shader_layout(l_shader_layout),
				l_ctx.graphics_allocator.get_shader_module(l_vertex_shader_module),
				l_ctx.graphics_allocator.get_shader_module(l_fragment_shader_module)
		};

		Token(Shader) l_graphics_shader = l_ctx.graphics_allocator.allocate_shader(l_shader_allocate_info);
		Token(ShaderIndex) l_shader = l_renderer.heap.push_shader(ShaderIndex{ 0, l_graphics_shader });


		Material l_red_material =Material::allocate_empty(l_ctx.graphics_allocator, 1);
		l_red_material.add_and_allocate_buffer_host_parameter_typed(l_ctx.graphics_allocator, l_ctx.buffer_allocator, l_ctx.graphics_allocator.get_shader_layout(l_shader_layout),
				v3f{ 1.0f, 0.0f, 0.0f });

		Material l_green_material =Material::allocate_empty(l_ctx.graphics_allocator, 1);
		l_green_material.add_and_allocate_buffer_host_parameter_typed(l_ctx.graphics_allocator, l_ctx.buffer_allocator, l_ctx.graphics_allocator.get_shader_layout(l_shader_layout),
				v3f{ 0.0f, 1.0f, 0.0f });

		Token(Material) l_red_material_token = l_renderer.heap.push_material(l_red_material);
		Token(Material) l_green_material_token = l_renderer.heap.push_material(l_green_material);

		l_renderer.heap.link_shader_with_material(l_shader, l_red_material_token);
		l_renderer.heap.link_shader_with_material(l_shader, l_green_material_token);

		Token(RenderableObject) l_obj_1,l_obj_2, l_obj_3, l_obj_4;

		{
			Vertex l_vertices[3] = { Vertex{ v3f{ 0.0f, 0.0f, 0.0f }, v2f{ 0.0f, 0.0f }}, Vertex{ v3f{ 1.0f, 1.0f, 1.0f }, v2f{ 1.0f, 1.0f }}, Vertex{ v3f{ 2.0f, 2.0f, 2.0f }, v2f{ 2.0f, 2.0f }}};
			l_obj_1 = l_renderer.allocate_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, Slice<Vertex>::build_memory_elementnb(l_vertices, 3));
			l_obj_2 = l_renderer.allocate_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, Slice<Vertex>::build_memory_elementnb(l_vertices, 3));
			l_obj_3 = l_renderer.allocate_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, Slice<Vertex>::build_memory_elementnb(l_vertices, 3));
			l_obj_4 = l_renderer.allocate_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, Slice<Vertex>::build_memory_elementnb(l_vertices, 3));
		}

		l_renderer.heap.link_material_with_renderable_object(l_red_material_token, l_obj_1);
		l_renderer.heap.link_material_with_renderable_object(l_red_material_token, l_obj_2);
		l_renderer.heap.link_material_with_renderable_object(l_green_material_token, l_obj_3);
		l_renderer.heap.link_material_with_renderable_object(l_green_material_token, l_obj_4);


		l_renderer.color_step.set_camera(l_ctx, Camera{ m44f_const::IDENTITY, m44f_const::IDENTITY });

		l_renderer.buffer_step(l_ctx);
		l_ctx.buffer_allocator.step();

		l_ctx.buffer_step_and_submit();

		GraphicsBinder l_binder = GraphicsBinder::build(l_ctx.buffer_allocator, l_ctx.graphics_allocator);
		l_binder.start();
		l_renderer.graphics_step(l_binder);
		l_binder.end();

		l_ctx.submit_graphics_binder(l_binder);
		l_ctx.wait_for_completion();

		l_renderer.heap.remove_shader(l_shader);
		l_renderer.free_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, l_obj_1);
		l_renderer.free_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, l_obj_2);
		l_renderer.free_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, l_obj_3);
		l_renderer.free_renderable_object(l_ctx.buffer_allocator, l_ctx.graphics_allocator, l_obj_4);

		l_ctx.graphics_allocator.free_shader(l_graphics_shader);
		l_green_material.free(l_ctx.graphics_allocator, l_ctx.buffer_allocator);
		l_red_material.free(l_ctx.graphics_allocator, l_ctx.buffer_allocator);

		l_ctx.graphics_allocator.free_shader_layout(l_shader_layout);

		l_ctx.graphics_allocator.free_shader_module(l_vertex_shader_module);
		l_ctx.graphics_allocator.free_shader_module(l_fragment_shader_module);

#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_ctx.buffer_allocator.device.device, NULL);
#endif

		l_renderer.free(l_ctx);
		l_ctx.free();
	};
};

int main()
{
#ifdef RENDER_DOC_DEBUG
	HMODULE mod = GetModuleHandleA("renderdoc.dll");

	pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
	int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_0, (void**)&rdoc_api);
	assert_true(ret == 1);
#endif

	v2::test();

	memleak_ckeck();
};
