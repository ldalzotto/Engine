#pragma once

/*
    A GraphicsBinder is a convenient way to properly use graphics objects inside the graphics command buffer.
*/
struct GraphicsBinder
{
    BufferAllocator& buffer_allocator;
    GraphicsAllocator2& graphics_allocator;
    GraphicsPass* binded_graphics_pass;
    Shader* binded_shader;
    ShaderLayout* binded_shader_layout;
    uint32 material_set_count;

    inline static GraphicsBinder build(BufferAllocator& p_buffer_allocator, GraphicsAllocator2& p_graphics_allocator)
    {
        return GraphicsBinder{p_buffer_allocator, p_graphics_allocator, NULL, NULL, NULL, 0};
    };

    inline void start()
    {
        this->graphics_allocator.graphics_device.command_buffer.begin();
    };

    inline void end()
    {
        this->graphics_allocator.graphics_device.command_buffer.end();
    };

    inline void submit_after(const Semafore p_semaphore_wait_for, const gpu::CommandBufferSubmit p_command_buffer_wait_for)
    {
        this->graphics_allocator.graphics_device.command_buffer.submit_after(p_semaphore_wait_for, p_command_buffer_wait_for);
    };

    inline void submit_after_and_notify(const Semafore p_semaphore_wait_for, const gpu::CommandBufferSubmit p_command_buffer_wait_for, const Semafore p_notify)
    {
        this->graphics_allocator.graphics_device.command_buffer.submit_after_and_notify(p_semaphore_wait_for, p_command_buffer_wait_for, p_notify);
    };

    inline void begin_render_pass(GraphicsPass& p_graphics_pass, const Slice<v4f>& p_clear_values)
    {
#if __DEBUG
        assert_true(p_graphics_pass.render_pass.debug_attachement_layout.Capacity == p_clear_values.Size);
#endif
        this->binded_graphics_pass = &p_graphics_pass;
        _cmd_beginRenderPass2(p_clear_values);
    };

    inline void end_render_pass()
    {
#if __DEBUG
        if (this->binded_graphics_pass == NULL)
        {
            abort();
        }
#endif
        _cmd_endRenderPass();
        this->binded_graphics_pass = NULL;
    };

    inline void bind_shader(Shader& p_shader)
    {
#if __DEBUG
        if (this->binded_graphics_pass == NULL)
        {
            abort();
        }
#endif
        _cmd_bind_shader(p_shader);
        this->binded_shader = &p_shader;
        this->binded_shader_layout = &p_shader.layout;
    };

    inline void bind_shader_layout(ShaderLayout& p_shader_layout)
    {
        this->binded_shader_layout = &p_shader_layout;
    };

    inline void bind_material(const Material p_material)
    {
        Slice<ShaderParameter> l_material_parameters = this->graphics_allocator.heap.material_parameters.get_vector(p_material.parameters);
        for (loop(i, 0, l_material_parameters.Size))
        {
            _cmd_bind_shader_parameter(*this->binded_shader_layout, l_material_parameters.get(i), this->material_set_count);
            this->material_set_count += 1;
        }
    };

    inline void pop_material_bind(const Material p_material)
    {
        this->material_set_count -= (uint32)this->graphics_allocator.heap.material_parameters.get_vector(p_material.parameters).Size;

#if __DEBUG
        assert_true(this->material_set_count <= 10000);
#endif
    };

    inline void bind_shaderbufferhost_parameter(const ShaderUniformBufferHostParameter& p_parameter)
    {
        _cmd_bind_uniform_buffer_parameter(*this->binded_shader_layout, p_parameter.descriptor_set, this->material_set_count);
        this->material_set_count += 1;
    };

    inline void pop_shaderbufferhost_parameter()
    {
        this->material_set_count -= 1;
#if __DEBUG
        assert_true(this->material_set_count <= 10000);
#endif
    };

    inline void bind_shadertexturegpu_parameter(const ShaderTextureGPUParameter& p_parameter)
    {
        _cmd_bind_shader_texture_gpu_parameter(*this->binded_shader_layout, p_parameter, this->material_set_count);
        this->material_set_count += 1;
    };

    inline void pop_shadertexturegpu_parameter()
    {
        this->material_set_count -= 1;
#if __DEBUG
        assert_true(this->material_set_count <= 10000);
#endif
    };

    inline void bind_index_buffer_gpu(const BufferGPU& p_index_buffer_gpu, const BufferIndexType p_index_type)
    {
        VkDeviceSize l_offset = 0;
        vkCmdBindIndexBuffer((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), (VkBuffer)token_value(p_index_buffer_gpu.buffer), l_offset,
                             (VkIndexType)p_index_type);
    };

    inline void bind_vertex_buffer_gpu(const BufferGPU& p_vertex_buffer_gpu)
    {
        VkDeviceSize l_offset = 0;
        vkCmdBindVertexBuffers((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), 0, 1, (VkBuffer*)token_ptr(p_vertex_buffer_gpu.buffer), &l_offset);
    };

    inline void bind_vertex_buffer_host(const BufferHost& p_vertex_buffer_host)
    {
        VkDeviceSize l_offset = 0;
        vkCmdBindVertexBuffers((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), 0, 1, (VkBuffer const*)token_ptr(p_vertex_buffer_host.buffer),
                               &l_offset);
    };

    inline void draw(const uimax p_vertex_count)
    {
        vkCmdDraw((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), (uint32_t)p_vertex_count, 1, 0, 1);
    };

    inline void draw_indexed(const uimax p_indices_count)
    {
        vkCmdDrawIndexed((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), (uint32_t)p_indices_count, 1, 0, 0, 0);
    };

    inline void draw_offsetted(const uimax p_vertex_count, const uimax p_offset)
    {
        vkCmdDraw((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), (uint32_t)p_vertex_count, 1, (uint32)p_offset, 1);
    };

  private:
    inline void _cmd_beginRenderPass2(const Slice<v4f>& p_clear_values)
    {
        VkRenderPassBeginInfo l_renderpass_begin{};
        l_renderpass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        l_renderpass_begin.renderPass = this->binded_graphics_pass->render_pass.render_pass;

        Slice<Token<TextureGPU>> l_attachments = this->graphics_allocator.heap.renderpass_attachment_textures.get_vector(this->binded_graphics_pass->attachment_textures);
        ImageFormat& l_target_format = this->buffer_allocator.gpu_images.get(this->graphics_allocator.heap.textures_gpu.get(l_attachments.get(0)).Image).format;

        l_renderpass_begin.renderArea = VkRect2D{VkOffset2D{0, 0}, VkExtent2D{(uint32_t)l_target_format.extent.x, (uint32_t)l_target_format.extent.y}};
        l_renderpass_begin.clearValueCount = (uint32)p_clear_values.Size;
        l_renderpass_begin.pClearValues = (VkClearValue*)p_clear_values.Begin;

        l_renderpass_begin.framebuffer = this->binded_graphics_pass->frame_buffer;

        VkViewport l_viewport{};
        l_viewport.width = (float)l_target_format.extent.x;
        l_viewport.height = (float)l_target_format.extent.y;
        l_viewport.minDepth = 0.0f;
        l_viewport.maxDepth = 1.0f;

        VkRect2D l_windowarea = VkRect2D{VkOffset2D{0, 0}, VkExtent2D{(uint32_t)l_target_format.extent.x, (uint32_t)l_target_format.extent.y}};
        vkCmdSetViewport((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), 0, 1, &l_viewport);
        vkCmdSetScissor((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), 0, 1, &l_windowarea);

        vkCmdBeginRenderPass((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), &l_renderpass_begin, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    };

    inline void _cmd_endRenderPass()
    {
        vkCmdEndRenderPass((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer));
    };

    inline void _cmd_bind_shader(const Shader& p_shader)
    {
        vkCmdBindPipeline((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, p_shader.shader);
    };

    inline void _cmd_bind_shader_parameter(const ShaderLayout& p_shader_layout, const ShaderParameter& p_shader_parameter, const uint32 p_set_number)
    {
        switch (p_shader_parameter.type)
        {
        case ShaderParameter::Type::UNIFORM_HOST:
        {
            _cmd_bind_uniform_buffer_parameter(p_shader_layout, this->graphics_allocator.heap.shader_uniform_buffer_host_parameters.get(p_shader_parameter.uniform_host).descriptor_set, p_set_number);
        }
        break;
        case ShaderParameter::Type::UNIFORM_GPU:
        {
            _cmd_bind_uniform_buffer_parameter(p_shader_layout, this->graphics_allocator.heap.shader_uniform_buffer_gpu_parameters.get(p_shader_parameter.uniform_gpu).descriptor_set, p_set_number);
        }
        break;
        case ShaderParameter::Type::TEXTURE_GPU:
        {
            _cmd_bind_shader_texture_gpu_parameter(p_shader_layout, this->graphics_allocator.heap.shader_texture_gpu_parameters.get(p_shader_parameter.texture_gpu), p_set_number);
        }
        break;
        default:
            abort();
        }
    };

    inline void _cmd_bind_uniform_buffer_parameter(const ShaderLayout& p_shader_layout, const VkDescriptorSet p_descriptor_set, const uint32 p_set_number)
    {
#if __DEBUG
        assert_true(p_shader_layout.shader_layout_parameter_types.get(p_set_number) == ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX ||
                    p_shader_layout.shader_layout_parameter_types.get(p_set_number) == ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT);
#endif

        vkCmdBindDescriptorSets((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
                                p_shader_layout.layout, p_set_number, 1, &p_descriptor_set, 0, NULL);
    };

    inline void _cmd_bind_shader_texture_gpu_parameter(const ShaderLayout& p_shader_layout, const ShaderTextureGPUParameter& p_shader_parameter, const uint32 p_set_number)
    {
#if __DEBUG
        assert_true(p_shader_layout.shader_layout_parameter_types.get(p_set_number) == ShaderLayoutParameterType::TEXTURE_FRAGMENT);
#endif

        vkCmdBindDescriptorSets((VkCommandBuffer)token_value(this->graphics_allocator.graphics_device.command_buffer.command_buffer), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
                                p_shader_layout.layout, p_set_number, 1, &p_shader_parameter.descriptor_set, 0, NULL);
    };
};