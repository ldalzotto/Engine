#pragma once

#include "GPU/gpu.hpp"

#include "./render_target.hpp"
#include "./d3_renderer.hpp"

struct Renderer_3D
{
    RenderTargetInternal_Color_Depth render_targets;
    D3Renderer d3_renderer;

    inline static Renderer_3D allocate(GPUContext& p_gpu_context, const RenderTargetInternal_Color_Depth::AllocateInfo& p_render_targets_allocate_info)
    {
        Renderer_3D l_return;
        l_return.render_targets = RenderTargetInternal_Color_Depth::allocate(p_gpu_context, p_render_targets_allocate_info);
        l_return.d3_renderer = D3Renderer::allocate(p_gpu_context, l_return.render_targets);
        return l_return;
    };

    inline void free(GPUContext& p_gpu_context)
    {
        this->d3_renderer.free(p_gpu_context);
        this->render_targets.free(p_gpu_context);
    };

    inline void buffer_step(GPUContext& p_gpu_context, const float32 p_totaltime)
    {
        this->d3_renderer.buffer_step(p_gpu_context, p_totaltime);
    };

    inline void graphics_step(GraphicsBinder& p_graphics_binder)
    {
        this->d3_renderer.graphics_step(p_graphics_binder);
    };
};

// TODO Renderer_3D_imgui