
#include "Render/render.hpp"

namespace v2
{

	inline void test()
	{
		GPUContext l_ctx = GPUContext::allocate();
		D3Renderer l_renderer = D3Renderer::allocate(l_ctx, ColorStep::AllocateInfo{ v3ui{ 8, 8, 1 }});





		const int8* p_vertex_litteral =
				MULTILINE(\
                    #version 450 \n

						layout(location = 0) in vec3 pos; \n
						layout(location = 1) in vec2 uv; \n

						void main()\n
						{ \n
								gl_Position = vec4(pos.xyz, 0.5f);\n
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






		Token(ShaderIndex) l_shader = l_renderer.heap.push_shader(ShaderIndex{});
		Token(Material) l_material = l_renderer.heap.push_material(Material{});
		Token(RenderableObject) l_renderable_object = l_renderer.heap.allocate_renderable_object(RenderableObject{});

		l_renderer.heap.link_shader_with_material(l_shader, l_material);
		l_renderer.heap.link_material_with_renderable_object(l_material, l_renderable_object);









		l_renderer.buffer_step(l_ctx);
		l_ctx.buffer_allocator.step();

		GraphicsBinder l_binder = GraphicsBinder::build(l_ctx.buffer_allocator, l_ctx.graphics_allocator);
		l_binder.start();
		l_renderer.graphics_step(l_binder);
		l_binder.end();


		l_renderer.free(l_ctx);
		l_ctx.free();
	};
};

int main()
{
	v2::test();

	memleak_ckeck();
};