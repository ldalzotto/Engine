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

    ImguiRenderer l_imgui_renderer = ImguiRenderer::allocate(l_gpu_context);
    l_imgui_renderer.initialize_imgui(l_gpu_context);

    l_gpu_context.buffer_step_and_submit();
    GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();
    l_imgui_renderer.create_font_texture(l_graphics_binder);
    l_imgui_renderer.render(l_graphics_binder);
    l_gpu_context.submit_graphics_binder(l_graphics_binder);
    l_gpu_context.wait_for_completion();

    l_gpu_context.buffer_step_and_submit();
    GraphicsBinder l_graphics_binder_2 = l_gpu_context.creates_graphics_binder();
    l_imgui_renderer.render(l_graphics_binder_2);
    l_gpu_context.submit_graphics_binder(l_graphics_binder_2);
    l_gpu_context.wait_for_completion();


#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_imgui_renderer.free(l_gpu_context);
    l_gpu_context.free();

    // l_imgui_renderer.render();
    memleak_ckeck();

    return 0;
};