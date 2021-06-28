#pragma once

struct RenderTargetInternal_Color_Depth
{
    v3ui dimensions;
    TextureGPU_Token color;
    TextureGPU_Token depth;

    struct AllocateInfo
    {
        v3ui render_target_dimensions;
        int8 attachment_host_read;
        int8 color_attachment_sample;
    };

    inline static RenderTargetInternal_Color_Depth allocate(GPUContext& p_gpu_context, const AllocateInfo& p_allocate_info)
    {
        RenderTargetInternal_Color_Depth l_return;
        l_return.dimensions = p_allocate_info.render_target_dimensions;

        ImageUsageFlag l_additional_attachment_usage_flags = ImageUsageFlag::UNKNOWN;
        if (p_allocate_info.attachment_host_read)
        {
            l_additional_attachment_usage_flags = l_additional_attachment_usage_flags | ImageUsageFlag::TRANSFER_READ;
        }
        if (p_allocate_info.color_attachment_sample)
        {
            l_additional_attachment_usage_flags = l_additional_attachment_usage_flags | ImageUsageFlag::SHADER_TEXTURE_PARAMETER;
        }

        ImageFormat l_color_attachment_format = ImageFormat::build_color_2d(p_allocate_info.render_target_dimensions, ImageUsageFlag::SHADER_COLOR_ATTACHMENT | l_additional_attachment_usage_flags);
        l_return.color = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_color_attachment_format);

        ImageFormat l_depth_attachment_format = ImageFormat::build_depth_2d(p_allocate_info.render_target_dimensions, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT | l_additional_attachment_usage_flags);
        l_return.depth = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, l_depth_attachment_format);

        return l_return;
    };

    inline void free(GPUContext& p_gpu_context)
    {
        GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, this->color);
        GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(p_gpu_context.buffer_memory, p_gpu_context.graphics_allocator, this->depth);
    };
};