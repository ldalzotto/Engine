#pragma once

#include "Engine/engine.hpp"
#include "backends/imgui_impl_vulkan.h"
#include "imgui_internal.h"

/*
  TODO -> in the D3Render module the graphics pass shouldn't allocate texture, they must be created separately and being able to be carried over the pipeline  ?
 */
inline void imgui_assert_true(const int8 p_bool)
{
#if __DEBUG
    assert_true(p_bool);
#endif
};

struct ImguiRenderer
{
    Token<GraphicsPass> graphics_pass;
    Span<v4f> clear_values;

    inline static ImguiRenderer allocate(GPUContext& p_gpu_context)
    {
        ImguiRenderer l_return;
        v3ui l_extend = v3ui{200, 200, 1};
        SliceN<RenderPassAttachment, 2> l_attachments = {RenderPassAttachment{AttachmentType::COLOR, ImageFormat::build_color_2d(l_extend, ImageUsageFlag::SHADER_COLOR_ATTACHMENT)},
                                                         RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(l_extend, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)}};
        l_return.graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_attachments);

        SliceN<v4f, 2> tmp_clear_values{v4f{0.0f, 0.0f, 0.0f, 1.0f}, v4f{1.0f, 0.0f, 0.0f, 0.0f}};
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

        imgui_assert_true(ImGui_ImplVulkan_Init(&l_imgui_info, p_gpu_context.graphics_allocator.heap.graphics_pass.get(this->graphics_pass).render_pass.render_pass));
    };

    // TODO -> move this to the buffer step.
    //         we need to create a functional object like the GraphicsBinder but for buffer allocation.
    inline void create_font_texture(GraphicsBinder& p_graphics_binder)
    {
        ImGui_ImplVulkan_CreateFontsTexture(p_graphics_binder.graphics_allocator.graphics_device.command_buffer.command_buffer);
    };

    inline void free(GPUContext& p_gpu_context)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();

        GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, this->graphics_pass);
        this->clear_values.free();
    };

    inline void render(GraphicsBinder& p_graphics_binder)
    {
        // ImGui_ImplVulkan_CreateFontsTexture(p_graphics_binder.buffer_allocator.device.command_buffer.command_buffer);
        ImGuiIO& l_io = ImGui::GetIO();
        // TODO -> the display size must be set on the initiallization. Later when we will create a resize event, we will update it's value.
        l_io.DisplaySize = ImVec2(800, 800);
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
