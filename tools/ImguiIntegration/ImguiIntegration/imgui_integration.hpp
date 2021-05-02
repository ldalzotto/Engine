#pragma once

#include "Engine/engine.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "imgui_internal.h"

/*
    // TODO -> forbid the usage of imgui static
    // TODO -> move to the render module as an independant renderer
    // TODO -> write test with the default window
 */

struct ImguiRenderer
{
    Token<GraphicsPass> graphics_pass;
    Span<v4f> clear_values;

    inline static ImguiRenderer allocate(GPUContext& p_gpu_context, const RenderTargetInternal_Color_Depth& p_render_target)
    {
        ImguiRenderer l_return;
        SliceN<AttachmentType, 1> l_attachment_types = {AttachmentType::COLOR};
        SliceN<Token<TextureGPU>, 1> l_attachments = {p_render_target.color};
        l_return.graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_from_textures<1>(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_attachments, l_attachment_types);

        // SliceN<v4f, 2> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}, v4f{1.0f, 0.0f, 0.0f, 0.0f}};
        SliceN<v4f, 1> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}};
        l_return.clear_values = Span<v4f>::allocate_slice(slice_from_slicen(&tmp_clear_values));

        return l_return;
    };

    inline void initialize_imgui(GPUContext& p_gpu_context)
    {
        ImGui::SetCurrentContext(ImGui::CreateContext());
        // ImGui::CreateContext();
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

        int8 l_success =ImGui_ImplVulkan_Init(&l_imgui_info, p_gpu_context.graphics_allocator.heap.graphics_pass.get(this->graphics_pass).render_pass.render_pass);
#if __DEBUG
        assert_true(l_success);
#endif
    };

    inline void buffer_step(BufferMemory& p_buffer_memory)
    {
        ImGui_ImplVulkan_CreateFontsTexture(p_buffer_memory.allocator.device.command_buffer.command_buffer);
    };

    inline void free(GPUContext& p_gpu_context)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();

        p_gpu_context.graphics_allocator.free_graphicspass_with_texture_slice(this->graphics_pass);
        this->clear_values.free();
    };

    inline void graphics_step(GraphicsBinder& p_graphics_binder, const RenderTargetInternal_Color_Depth& p_render_target)
    {
        ImGuiIO& l_io = ImGui::GetIO();
        l_io.DisplaySize = ImVec2((float32)p_render_target.dimensions.x, (float32)p_render_target.dimensions.y);
        l_io.DeltaTime = 1.0f / 60.0f;

        ImGui::NewFrame();

        ImGui::ShowDemoWindow(NULL);

        // ImGui::EndFrame();
        ImGui::Render();
        ImDrawData* l_draw_data = ImGui::GetDrawData();

        p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.heap.graphics_pass.get(this->graphics_pass), this->clear_values.slice);
        ImGui_ImplVulkan_RenderDrawData(l_draw_data, p_graphics_binder.graphics_allocator.graphics_device.command_buffer.command_buffer);
        p_graphics_binder.end_render_pass();
    };
};
