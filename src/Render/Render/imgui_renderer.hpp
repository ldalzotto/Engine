#pragma once

#include "backends/imgui_impl_vulkan.h"
#include "imgui_internal.h"

// TODO -> this must me moved to it's own system.
// TODO -> this is not how we want to integrate the imgui library into the engine.
//         right know, imgui and the engine doesn't blend together. The vulkan renderer does not make use of our own renderer
//         and vertices are drawn out of the blue, there is no MeshRenderer in the scene !!
//         What we instead is to simulate the imgui library as if we would have created our own gui library. The gui module whould generare events that will be consumed by the
//         engine to create the mesh renderers and update it's vertex buffer.

// This struct acts as an interface between imgui and the engine.
// All coordinates are expressed in voewport space [-1.0f , 1.0f].
struct ImguiLib
{
    v3ui render_target_dimensions;

    inline void ShowDemoWindow()
    {
        ImGui::ShowDemoWindow(NULL);
    };
};

struct ImguiRenderer
{
    ImGuiContext* imgui_ctx;

    Token<GraphicsPass> graphics_pass;
    int8 fonts_initialized;
    Span<v4f> clear_values;

    inline static ImguiRenderer allocate(GPUContext& p_gpu_context, const RenderTargetInternal_Color_Depth& p_render_target, const RenderPassAttachment::ClearOp p_clear_op)
    {
        ImguiRenderer l_return;
        l_return.fonts_initialized = 0;
        GraphicsPassAllocationComposition::RenderPassAttachmentInput<1> l_graphicspass_allocation_input = {
            SliceN<Token<TextureGPU>, 1>{p_render_target.color}, SliceN<AttachmentType, 1>{AttachmentType::COLOR}, SliceN<RenderPassAttachment::ClearOp, 1>{p_clear_op}};
        l_return.graphics_pass =
            GraphicsPassAllocationComposition::allocate_renderpass_then_graphicspass(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_graphicspass_allocation_input);

        // SliceN<v4f, 2> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}, v4f{1.0f, 0.0f, 0.0f, 0.0f}};
        SliceN<v4f, 1> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}};
        l_return.clear_values = Span<v4f>::allocate_slice(slice_from_slicen(&tmp_clear_values));

        l_return.imgui_ctx = initialize_imgui(p_gpu_context, l_return.graphics_pass);

        return l_return;
    };

    inline static ImGuiContext* initialize_imgui(GPUContext& p_gpu_context, const Token<GraphicsPass> p_graphics_pass)
    {
        ImGuiContext* l_imgui_ctx = ImGui::CreateContext();

#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_imgui_ctx);
#endif

        ImGui::SetCurrentContext(ImGui::CreateContext());
        ImGui::StyleColorsClassic();

        ImGui_ImplVulkan_InitInfo l_imgui_info{};
        l_imgui_info.Instance = (VkInstance)p_gpu_context.instance.instance.tok;
        l_imgui_info.PhysicalDevice = (VkPhysicalDevice)p_gpu_context.instance.graphics_card.device.tok;
        l_imgui_info.Device = (gc_t)p_gpu_context.instance.logical_device.tok;
        l_imgui_info.QueueFamily = p_gpu_context.instance.graphics_card.graphics_queue_family.family;
        l_imgui_info.Queue = p_gpu_context.graphics_allocator.graphics_device.graphics_queue;
        l_imgui_info.DescriptorPool = p_gpu_context.graphics_allocator.graphics_device.shaderparameter_pool.descriptor_pool;
        l_imgui_info.Subpass = 0;
        l_imgui_info.MinImageCount = 2;
        l_imgui_info.ImageCount = 2;
        l_imgui_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        int8 l_success = ImGui_ImplVulkan_Init(&l_imgui_info, p_gpu_context.graphics_allocator.heap.graphics_pass.get(p_graphics_pass).render_pass.render_pass);
#if __DEBUG
        assert_true(l_success);
#endif
        return l_imgui_ctx;
    };

    inline void free(GPUContext& p_gpu_context)
    {
#if __MEMLEAK
        remove_ptr_to_tracked((int8*)this->imgui_ctx);
#endif

        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext(this->imgui_ctx);

        GraphicsPassAllocationComposition::free_graphicspass(p_gpu_context.graphics_allocator, this->graphics_pass);
        this->clear_values.free();
    };

    inline void buffer_step(BufferMemory& p_buffer_memory)
    {
        if (!this->fonts_initialized)
        {
            ImGui_ImplVulkan_CreateFontsTexture((VkCommandBuffer)p_buffer_memory.allocator.device.command_buffer.command_buffer.tok);
            this->fonts_initialized = 1;
        }
    };

    // TODO -> There is no obligation to call this just before the renderer. We can call it before the user defined engine loop function.
    inline ImguiLib graphics_step_begin(const RenderTargetInternal_Color_Depth& p_render_target)
    {
        ImGuiIO& l_io = ImGui::GetIO();
        l_io.DisplaySize = ImVec2((float32)p_render_target.dimensions.x, (float32)p_render_target.dimensions.y);
        l_io.DeltaTime = 1.0f / 60.0f;

        ImGui::NewFrame();

        return ImguiLib{p_render_target.dimensions};
    };

    inline void graphics_step(GraphicsBinder& p_graphics_binder)
    {
        ImGui::Render();
        ImDrawData* l_draw_data = ImGui::GetDrawData();

        p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.heap.graphics_pass.get(this->graphics_pass), this->clear_values.slice);
        ImGui_ImplVulkan_RenderDrawData(l_draw_data, (VkCommandBuffer)p_graphics_binder.graphics_allocator.graphics_device.command_buffer.command_buffer.tok);
        p_graphics_binder.end_render_pass();
    };
};
