#include "./ImguiIntegration/imgui_integration.hpp"

#define RENDER_DOC_DEBUG 1
#ifdef RENDER_DOC_DEBUG
#include "../../src/Test/Renderdoc/renderdoc_app.h"
RENDERDOC_API_1_1_0* rdoc_api = NULL;
#endif

int main()
{
#ifdef RENDER_DOC_DEBUG
    HMODULE mod = GetModuleHandleA("renderdoc.dll");

    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_0, (void**)&rdoc_api);
    assert_true(ret == 1);
#endif

    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    RenderTargetInternal_Color_Depth::AllocateInfo l_render_target_allocate_info{};
    l_render_target_allocate_info.render_target_dimensions = v3ui{200, 200, 1};
    RenderTargetInternal_Color_Depth l_render_target = RenderTargetInternal_Color_Depth::allocate(l_gpu_context, l_render_target_allocate_info);
    ImguiRenderer l_imgui_renderer = ImguiRenderer::allocate(l_gpu_context, l_render_target);

    BufferStepExecutionFlow::buffer_step_begin(l_gpu_context.buffer_memory);
    l_imgui_renderer.buffer_step(l_gpu_context.buffer_memory);
    BufferStep::step(l_gpu_context.buffer_memory.allocator, l_gpu_context.buffer_memory.events);
    BufferStepExecutionFlow::buffer_step_submit(l_gpu_context);

    ImguiLib l_imgui = l_imgui_renderer.graphics_step_begin(l_render_target);

    l_imgui.ShowDemoWindow();

    GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();
    l_imgui_renderer.graphics_step_end(l_graphics_binder);

    l_gpu_context.submit_graphics_binder(l_graphics_binder);
    l_gpu_context.wait_for_completion();

    l_gpu_context.buffer_step_submit();

    l_imgui = l_imgui_renderer.graphics_step_begin(l_render_target);

    l_imgui.ShowDemoWindow();

    GraphicsBinder l_graphics_binder_2 = l_gpu_context.build_graphics_binder();
    l_imgui_renderer.graphics_step_end(l_graphics_binder_2);
    l_gpu_context.submit_graphics_binder(l_graphics_binder_2);

    l_gpu_context.wait_for_completion();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_imgui_renderer.free(l_gpu_context);
    l_render_target.free(l_gpu_context);
    l_gpu_context.free();

    // l_imgui_renderer.render();
    memleak_ckeck();

    return 0;
};