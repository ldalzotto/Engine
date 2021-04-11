#pragma once

namespace ShaderParameterBufferAllocationFunctions
{
inline static Token<TextureGPU> allocate_texture_gpu_for_shaderparameter(GraphicsAllocator2& p_graphics_allocator, BufferMemory& p_buffer_memory, const ImageFormat& p_base_image_format)
{
    ImageFormat l_format = p_base_image_format;
    l_format.imageUsage = (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE | (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER);
    Token<TextureGPU> l_texture_gpu_token = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(p_buffer_memory, p_graphics_allocator, l_format);
    return l_texture_gpu_token;
};
}; // namespace ShaderParameterBufferAllocationFunctions

/*
    A Material is a convenient way to allocate and access shader parameter buffers.
    On debug, it also performs runtime verification against the Shader layout.
*/
struct Material
{
    Token<Slice<ShaderParameter>> parameters;

    uint32 set_index_offset;

    inline static Material allocate_empty(GraphicsAllocator2& p_graphics_allocator, const uint32 p_set_index_offset)
    {
        return Material{p_graphics_allocator.allocate_material_parameters(), p_set_index_offset};
    };

    inline void free(GraphicsAllocator2& p_graphics_allocator, BufferMemory& p_buffer_memory)
    {
        Slice<ShaderParameter> l_shader_parameters = p_graphics_allocator.heap.material_parameters.get_vector(this->parameters);
        for (loop(i, 0, l_shader_parameters.Size))
        {
            ShaderParameter& l_shader_paramter = l_shader_parameters.get(i);
            switch (l_shader_paramter.type)
            {
            case ShaderParameter::Type::UNIFORM_HOST:
            {
                auto& l_shader_parameter = p_graphics_allocator.heap.shader_uniform_buffer_host_parameters.get(l_shader_paramter.uniform_host);
                BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, l_shader_parameter.memory);
                p_graphics_allocator.free_shaderuniformbufferhost_parameter(l_shader_paramter.uniform_host);
            }
            break;
            case ShaderParameter::Type::UNIFORM_GPU:
            {
                auto& l_shader_parameter = p_graphics_allocator.heap.shader_uniform_buffer_gpu_parameters.get(l_shader_paramter.uniform_gpu);
                BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, l_shader_parameter.memory);
                p_graphics_allocator.free_shaderuniformbuffergpu_parameter(l_shader_paramter.uniform_gpu);
            }
            break;
            case ShaderParameter::Type::TEXTURE_GPU:
            {
                auto& l_shader_pameter = p_graphics_allocator.heap.shader_texture_gpu_parameters.get(l_shader_paramter.texture_gpu);
                p_graphics_allocator.free_shadertexturegpu_parameter(p_buffer_memory.allocator.device, l_shader_paramter.texture_gpu,
                                                                     p_graphics_allocator.heap.shader_texture_gpu_parameters.get(l_shader_paramter.texture_gpu));
            }
            break;
            default:
                abort();
            }
        }

        p_graphics_allocator.free_material_parameters(this->parameters);
    };

    inline void free_with_textures(GraphicsAllocator2& p_graphics_allocator, BufferMemory& p_buffer_memory)
    {
        Slice<ShaderParameter> l_shader_parameters = p_graphics_allocator.heap.material_parameters.get_vector(this->parameters);
        for (loop(i, 0, l_shader_parameters.Size))
        {
            ShaderParameter& l_shader_paramter = l_shader_parameters.get(i);
            switch (l_shader_paramter.type)
            {
            case ShaderParameter::Type::UNIFORM_HOST:
            {
                auto& l_shader_parameter = p_graphics_allocator.heap.shader_uniform_buffer_host_parameters.get(l_shader_paramter.uniform_host);
                BufferAllocatorComposition::free_buffer_host_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, l_shader_parameter.memory);
                p_graphics_allocator.free_shaderuniformbufferhost_parameter(l_shader_paramter.uniform_host);
            }
            break;
            case ShaderParameter::Type::UNIFORM_GPU:
            {
                auto& l_shader_parameter = p_graphics_allocator.heap.shader_uniform_buffer_gpu_parameters.get(l_shader_paramter.uniform_gpu);
                BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, l_shader_parameter.memory);
                p_graphics_allocator.free_shaderuniformbuffergpu_parameter(l_shader_paramter.uniform_gpu);
            }
            break;
            case ShaderParameter::Type::TEXTURE_GPU:
            {
                auto& l_shader_pameter = p_graphics_allocator.heap.shader_texture_gpu_parameters.get(l_shader_paramter.texture_gpu);
                BufferAllocatorComposition::free_image_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events,
                                                                                       p_graphics_allocator.heap.textures_gpu.get(l_shader_pameter.texture).Image);
                p_graphics_allocator.free_shadertexturegpu_parameter_with_texture(p_buffer_memory.allocator.device, l_shader_paramter.texture_gpu,
                                                                                  p_graphics_allocator.heap.shader_texture_gpu_parameters.get(l_shader_paramter.texture_gpu));
            }
            break;
            default:
                abort();
            }
        }

        p_graphics_allocator.free_material_parameters(this->parameters);
    };

    inline void add_buffer_host_parameter(GraphicsAllocator2& p_graphics_allocator, const ShaderLayout& p_shader_layout, const Token<BufferHost> p_buffer_token, const BufferHost& p_buffer)
    {
        uimax l_inserted_index = this->set_index_offset + p_graphics_allocator.heap.material_parameters.get_vector(this->parameters).Size;

#if __DEBUG
        assert_true(l_inserted_index < p_shader_layout.shader_layout_parameter_types.Capacity);
        assert_true(p_shader_layout.shader_layout_parameter_types.get(l_inserted_index) == ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX ||
                    p_shader_layout.shader_layout_parameter_types.get(l_inserted_index) == ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT);
#endif

        Token<ShaderUniformBufferHostParameter> l_parameter =
            p_graphics_allocator.allocate_shaderuniformbufferhost_parameter(p_shader_layout.shader_layout_parameter_types.get(l_inserted_index), p_buffer_token, p_buffer);

        p_graphics_allocator.heap.material_parameters.element_push_back_element(this->parameters, ShaderParameter{ShaderParameter::Type::UNIFORM_HOST, token_value(l_parameter)});
    };

    inline void add_and_allocate_buffer_host_parameter(GraphicsAllocator2& p_graphics_allocator, BufferAllocator& p_buffer_allocator, const ShaderLayout& p_shader_layout, const Slice<int8>& p_memory)
    {
        Token<BufferHost> l_buffer = p_buffer_allocator.allocate_bufferhost(p_memory, BufferUsageFlag::UNIFORM);
        this->add_buffer_host_parameter(p_graphics_allocator, p_shader_layout, l_buffer, p_buffer_allocator.host_buffers.get(l_buffer));
    };

    template <class ElementType>
    inline void add_and_allocate_buffer_host_parameter_typed(GraphicsAllocator2& p_graphics_allocator, BufferAllocator& p_buffer_allocator, const ShaderLayout& p_shader_layout,
                                                             const ElementType& p_memory)
    {
        this->add_and_allocate_buffer_host_parameter(p_graphics_allocator, p_buffer_allocator, p_shader_layout, Slice<ElementType>::build_asint8_memory_singleelement(&p_memory));
    };

    inline Slice<int8> get_buffer_host_parameter_memory(GraphicsAllocator2& p_graphics_allocator, BufferAllocator& p_buffer_allocator, const uimax p_index)
    {
        ShaderParameter& l_shader_parameter = p_graphics_allocator.heap.material_parameters.get_vector(this->parameters).get(this->set_index_offset + p_index);

#if __DEBUG
        assert_true(l_shader_parameter.type == ShaderParameter::Type::UNIFORM_HOST);
#endif

        return p_buffer_allocator.host_buffers.get(p_graphics_allocator.heap.shader_uniform_buffer_host_parameters.get(l_shader_parameter.uniform_host).memory).get_mapped_effective_memory();
    };

    template <class ElementType> inline ElementType& get_buffer_host_parameter_memory_typed(GraphicsAllocator2& p_graphics_allocator, BufferAllocator& p_buffer_allocator, const uimax p_index)
    {
        return *(ElementType*)this->get_buffer_host_parameter_memory(p_graphics_allocator, p_buffer_allocator, p_index).Begin;
    };

    inline void add_buffer_gpu_parameter(GraphicsAllocator2& p_graphics_allocator, const ShaderLayout& p_shader_layout, const Token<BufferGPU> p_buffer_gpu_token, const BufferGPU& p_buffer_gpu)
    {
        uimax l_inserted_index = this->set_index_offset + p_graphics_allocator.heap.material_parameters.get_vector(this->parameters).Size;

#if __DEBUG
        assert_true(l_inserted_index < p_shader_layout.shader_layout_parameter_types.Capacity);
        assert_true(p_shader_layout.shader_layout_parameter_types.get(l_inserted_index) == ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX ||
                    p_shader_layout.shader_layout_parameter_types.get(l_inserted_index) == ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT);
#endif

        Token<ShaderUniformBufferGPUParameter> l_shader_uniform_buffer_parameter =
            p_graphics_allocator.allocate_shaderuniformbuffergpu_parameter(p_shader_layout.shader_layout_parameter_types.get(l_inserted_index), p_buffer_gpu_token, p_buffer_gpu);

        p_graphics_allocator.heap.material_parameters.element_push_back_element(this->parameters, ShaderParameter{ShaderParameter::Type::UNIFORM_GPU, token_value(l_shader_uniform_buffer_parameter)});
    };

    inline void add_and_allocate_buffer_gpu_parameter(GraphicsAllocator2& p_graphics_allocator, BufferMemory& p_buffer_memory, const ShaderLayout& p_shader_layout, const Slice<int8>& p_memory)
    {
        Token<BufferGPU> l_buffer_gpu =
            p_buffer_memory.allocator.allocate_buffergpu(p_memory.Size, (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::UNIFORM));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_buffer_gpu, p_memory);

        this->add_buffer_gpu_parameter(p_graphics_allocator, p_shader_layout, l_buffer_gpu, p_buffer_memory.allocator.gpu_buffers.get(l_buffer_gpu));
    };

    template <class ElementType>
    inline void add_and_allocate_buffer_gpu_parameter_typed(GraphicsAllocator2& p_graphics_allocator, BufferMemory& p_buffer_memory, const ShaderLayout& p_shader_layout, const ElementType& p_memory)
    {
        this->add_and_allocate_buffer_gpu_parameter(p_graphics_allocator, p_buffer_memory, p_shader_layout, Slice<ElementType>::build_asint8_memory_singleelement(&p_memory));
    };

    inline Token<BufferGPU> get_buffer_gpu_parameter(GraphicsAllocator2& p_graphics_allocator, const uimax p_index)
    {
        ShaderParameter& l_shader_parameter = p_graphics_allocator.heap.material_parameters.get_vector(this->parameters).get(this->set_index_offset + p_index);

#if __DEBUG
        assert_true(l_shader_parameter.type == ShaderParameter::Type::UNIFORM_GPU);
#endif

        return p_graphics_allocator.heap.shader_uniform_buffer_gpu_parameters.get(l_shader_parameter.uniform_gpu).memory;
    };

    inline void add_texture_gpu_parameter(GraphicsAllocator2& p_graphics_allocator, const ShaderLayout& p_shader_layout, const Token<TextureGPU> p_texture_gpu_token, const TextureGPU& p_texture_gpu)
    {
        uimax l_inserted_index = this->set_index_offset + p_graphics_allocator.heap.material_parameters.get_vector(this->parameters).Size;

#if __DEBUG
        assert_true(l_inserted_index < p_shader_layout.shader_layout_parameter_types.Capacity);
        assert_true(p_shader_layout.shader_layout_parameter_types.get(l_inserted_index) == ShaderLayoutParameterType::TEXTURE_FRAGMENT);
#endif

        Token<ShaderTextureGPUParameter> l_shader_texture_gpu_parameter =
            p_graphics_allocator.allocate_shadertexturegpu_parameter(p_shader_layout.shader_layout_parameter_types.get(l_inserted_index), p_texture_gpu_token, p_texture_gpu);

        p_graphics_allocator.heap.material_parameters.element_push_back_element(this->parameters, ShaderParameter{ShaderParameter::Type::TEXTURE_GPU, token_value(l_shader_texture_gpu_parameter)});
    };

    inline void add_and_allocate_texture_gpu_parameter(GraphicsAllocator2& p_graphics_allocator, BufferMemory& p_buffer_memory, const ShaderLayout& p_shader_layout,
                                                       const ImageFormat& p_base_image_format, const Slice<int8>& p_memory)
    {
        Token<TextureGPU> l_texture_gpu_token = ShaderParameterBufferAllocationFunctions::allocate_texture_gpu_for_shaderparameter(p_graphics_allocator, p_buffer_memory, p_base_image_format);
        TextureGPU& l_texture_gpu = p_graphics_allocator.heap.textures_gpu.get(l_texture_gpu_token);
        BufferReadWrite::write_to_imagegpu(p_buffer_memory.allocator, p_buffer_memory.events, l_texture_gpu.Image, p_buffer_memory.allocator.gpu_images.get(l_texture_gpu.Image), p_memory);
        this->add_texture_gpu_parameter(p_graphics_allocator, p_shader_layout, l_texture_gpu_token, l_texture_gpu);
    };

    inline Token<TextureGPU> get_texture_gpu_parameter(GraphicsAllocator2& p_graphics_allocator, const uimax p_index)
    {
        ShaderParameter& l_shader_parameter = p_graphics_allocator.heap.material_parameters.get_vector(this->parameters).get(this->set_index_offset + p_index);

#if __DEBUG
        assert_true(l_shader_parameter.type == ShaderParameter::Type::TEXTURE_GPU);
#endif

        return p_graphics_allocator.heap.shader_texture_gpu_parameters.get(l_shader_parameter.texture_gpu).texture;
    };

    // inline void set_
};