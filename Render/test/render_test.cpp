
#include "Render/render.hpp"

namespace v2
{
	inline void test()
	{
		GPUContext l_ctx = GPUContext::allocate();
		D3Renderer l_renderer = D3Renderer::allocate(l_ctx, ColorStep::AllocateInfo{ v3ui{ 8, 8, 1 }});
		GraphicsBinder l_binder = GraphicsBinder::build(l_ctx.buffer_allocator, l_ctx.graphics_allocator);
		l_binder.start();
		l_renderer.buffer_step(l_ctx);
		l_renderer.graphics_step(l_binder);
		l_binder.end();
		l_renderer.free(l_ctx);
	};
};

int main()
{
	v2::test();
};