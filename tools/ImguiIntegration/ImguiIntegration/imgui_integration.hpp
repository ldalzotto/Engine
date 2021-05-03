#pragma once

#include "Engine/engine.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "imgui_internal.h"

/*
    // TODO -> write test with the default window
*/

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
    Span<v4f> clear_values;

    inline static ImguiRenderer allocate(GPUContext& p_gpu_context, const RenderTargetInternal_Color_Depth& p_render_target)
    {
        ImguiRenderer l_return;
        GraphicsPassAllocationComposition::RenderPassAttachmentInput<1> l_graphicspass_allocation_input = {
            SliceN<Token<TextureGPU>, 1>{p_render_target.color}, SliceN<AttachmentType, 1>{AttachmentType::COLOR}, SliceN<RenderPassAttachment::ClearOp, 1>{RenderPassAttachment::ClearOp::CLEARED}};
        l_return.graphics_pass =
            GraphicsPassAllocationComposition::allocate_renderpass_then_graphicspass(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_graphicspass_allocation_input);

        // SliceN<v4f, 2> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}, v4f{1.0f, 0.0f, 0.0f, 0.0f}};
        SliceN<v4f, 1> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}};
        l_return.clear_values = Span<v4f>::allocate_slice(slice_from_slicen(&tmp_clear_values));

        l_return.imgui_ctx = ImGui::CreateContext();

#if __MEMLEAK
        push_ptr_to_tracked((int8*)l_return.imgui_ctx);
#endif

        ImGui::SetCurrentContext(ImGui::CreateContext());
        ImGui::StyleColorsClassic();

        ImGui_ImplVulkan_InitInfo l_imgui_info{};
        l_imgui_info.Instance = p_gpu_context.instance.instance;
        l_imgui_info.PhysicalDevice = p_gpu_context.instance.graphics_card.device;
        l_imgui_info.Device = p_gpu_context.instance.logical_device;
        l_imgui_info.QueueFamily = p_gpu_context.instance.graphics_card.graphics_queue_family;
        l_imgui_info.Queue = p_gpu_context.graphics_allocator.graphics_device.graphics_queue;
        l_imgui_info.DescriptorPool = p_gpu_context.graphics_allocator.graphics_device.shaderparameter_pool.descriptor_pool;
        l_imgui_info.Subpass = 0;
        l_imgui_info.MinImageCount = 2;
        l_imgui_info.ImageCount = 2;
        l_imgui_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        int8 l_success = ImGui_ImplVulkan_Init(&l_imgui_info, p_gpu_context.graphics_allocator.heap.graphics_pass.get(l_return.graphics_pass).render_pass.render_pass);
#if __DEBUG
        assert_true(l_success);
#endif

        return l_return;
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
        ImGui_ImplVulkan_CreateFontsTexture(p_buffer_memory.allocator.device.command_buffer.command_buffer);
    };

    inline ImguiLib graphics_step_begin(const RenderTargetInternal_Color_Depth& p_render_target)
    {
        ImGuiIO& l_io = ImGui::GetIO();
        l_io.DisplaySize = ImVec2((float32)p_render_target.dimensions.x, (float32)p_render_target.dimensions.y);
        l_io.DeltaTime = 1.0f / 60.0f;

        ImGui::NewFrame();

        return ImguiLib{p_render_target.dimensions};
    };

    inline void graphics_step_end(GraphicsBinder& p_graphics_binder)
    {
        ImGui::Render();
        ImDrawData* l_draw_data = ImGui::GetDrawData();

        p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.heap.graphics_pass.get(this->graphics_pass), this->clear_values.slice);
        ImGui_ImplVulkan_RenderDrawData(l_draw_data, p_graphics_binder.graphics_allocator.graphics_device.command_buffer.command_buffer);
        p_graphics_binder.end_render_pass();
    };
};
