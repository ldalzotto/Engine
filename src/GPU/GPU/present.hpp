#pragma once

typedef VkSurfaceKHR gcsurface_t;

struct GPUPresentDevice
{
    GraphicsCard graphics_card;
    gc_t device;
    gcsurface_t surface;
    VkSurfaceCapabilitiesKHR surface_capacilities;
    VkSurfaceFormatKHR surface_format;
    gcqueue_t present_queue;
    VkPresentModeKHR present_mode;

    inline static GPUPresentDevice allocate(const GPUInstance& p_instance, const WindowHandle p_window_handle)
    {
        GPUPresentDevice l_present_device;
        l_present_device.graphics_card = p_instance.graphics_card;
        l_present_device.device = p_instance.logical_device;

#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR l_win32_surface_create{};
        l_win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        l_win32_surface_create.hwnd = (HWND)p_window_handle;
        l_win32_surface_create.hinstance = GetModuleHandle(NULL);
        vk_handle_result(vkCreateWin32SurfaceKHR(p_instance.instance, &l_win32_surface_create, NULL, &l_present_device.surface));
#endif

        l_present_device.recalculate_surface_capabilities();
        // vk_handle_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_instance.graphics_card.device, l_present_device.surface, &l_present_device.surface_capacilities));

        VkPhysicalDeviceProperties l_physical_device_properties;
        vkGetPhysicalDeviceProperties(p_instance.graphics_card.device, &l_physical_device_properties);

        uint32 l_present_queue_family_index = -1;
        Span<VkQueueFamilyProperties> l_queueFamilies = vk::getPhysicalDeviceQueueFamilyProperties(p_instance.graphics_card.device);

        for (loop(j, 0, l_queueFamilies.Capacity))
        {
            uint32 l_is_supported = 0;
            vk_handle_result(vkGetPhysicalDeviceSurfaceSupportKHR(p_instance.graphics_card.device, (uint32)j, l_present_device.surface, &l_is_supported));
            if (l_is_supported)
            {
                l_present_queue_family_index = (uint32)j;
                break;
            }
        }
        Span_free(&l_queueFamilies);

#if __DEBUG
        assert_true(l_present_queue_family_index != -1);
#endif

        vkGetDeviceQueue(l_present_device.device, l_present_queue_family_index, 0, &l_present_device.present_queue);

        l_present_device.present_mode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

        Span<VkSurfaceFormatKHR> l_surface_formats = vk::getPhysicalDeviceSurfaceFormatsKHR(p_instance.graphics_card.device, l_present_device.surface);
        for (loop(i, 0, l_surface_formats.Capacity))
        {
            VkSurfaceFormatKHR& l_surface_format = *Span_get(&l_surface_formats, i);
            if (l_surface_format.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB && l_surface_format.colorSpace == VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                l_present_device.surface_format = l_surface_format;
                break;
            }
        }
        Span_free(&l_surface_formats);

        return l_present_device;
    };

    inline void free(const GPUInstance& p_instance)
    {
        vkDestroySurfaceKHR(p_instance.instance, this->surface, NULL);
    };

    inline void recalculate_surface_capabilities()
    {
        vk_handle_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->graphics_card.device, this->surface, &this->surface_capacilities));
    };
};

struct GPUPresent_2DQuad
{
    struct _vertex
    {
        v3f pos;
        v2f uv;
    };

    Token(BufferGPU) d2_quad_vertices;
    Token(BufferGPU) d2_quad_indices_image_indices;

    inline static GPUPresent_2DQuad allocate(BufferMemory& p_buffer_memory)
    {
        Declare_sized_slice(_vertex, 4, l_2D_quad_vertices_arr, l_2D_quad_vertices, _vertex{v3f{-1.0f, 1.0f, 0.0f}, v2f{0.0f, 1.0f}}, _vertex{v3f{1.0f, -1.0f, 0.0f}, v2f{1.0f, 0.0f}},
                            _vertex{v3f{-1.0f, -1.0f, 0.0f}, v2f{0.0f, 0.0f}}, _vertex{v3f{1.0f, 1.0f, 0.0f}, v2f{1.0f, 1.0f}});

        GPUPresent_2DQuad l_return;
        l_return.d2_quad_vertices = p_buffer_memory.allocator.allocate_buffergpu(Slice_build_asint8(&l_2D_quad_vertices).Size,
                                                                                 (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::VERTEX | (BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_return.d2_quad_vertices, Slice_build_asint8(&l_2D_quad_vertices));

        Declare_sized_slice(uint32, 6, l_indices_arr, l_indices, 0, 1, 2, 0, 3, 1);
        l_return.d2_quad_indices_image_indices = p_buffer_memory.allocator.allocate_buffergpu(
            Slice_build_asint8(&l_indices).Size, (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::INDEX | (BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_return.d2_quad_indices_image_indices, Slice_build_asint8(&l_indices));

        return l_return;
    };

    inline void free(BufferMemory& p_buffer_memory)
    {
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, this->d2_quad_vertices);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, this->d2_quad_indices_image_indices);
    };

    inline void bind_and_draw(GraphicsBinder& p_graphics_binder)
    {
        p_graphics_binder.bind_vertex_buffer_gpu(p_graphics_binder.buffer_allocator.gpu_buffers.get(this->d2_quad_vertices));
        p_graphics_binder.bind_index_buffer_gpu(p_graphics_binder.buffer_allocator.gpu_buffers.get(this->d2_quad_indices_image_indices), BufferIndexType::UINT32);
        p_graphics_binder.draw_indexed(6);
    };
};

/*
    The SwapChain render the input render texture to a present texture.
    This structure allocates all GraphicsObjects necessary for the drawing to a present texture.
*/
struct SwapChain
{
    VkSwapchainKHR swap_chain;
    Semafore swap_chain_next_image_semaphore;

    v3ui swap_chain_dimensions;
    Span<Token(ImageGPU)> swap_chain_images;
    Span<Token(GraphicsPass)> rendertarget_copy_pass;
    Span<Token(Shader)> image_copy_shaders;

    Token(ShaderLayout) image_copy_shader_layout;
    Token(ShaderModule) image_copy_shader_vertex;
    Token(ShaderModule) image_copy_shader_fragment;
    Token(ShaderTextureGPUParameter) shader_parameter_texture;

    inline static SwapChain allocate(GPUPresentDevice& p_present_device, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, const Token(TextureGPU) p_presented_texture,
                                     const Slice<int8>& p_compiled_vertex_shader, const Slice<int8>& p_compiled_fragment_shader, const v3ui p_window_handle_dimensions)
    {
        SwapChain l_swap_chain;
        allocate_swap_chain_texture_independant(l_swap_chain, p_present_device, p_graphics_allocator, p_presented_texture, p_compiled_vertex_shader, p_compiled_fragment_shader);
        allocate_swap_chain_texture_dependendant(l_swap_chain, p_present_device, p_buffer_memory, p_graphics_allocator, p_window_handle_dimensions);
        return l_swap_chain;
    };

    inline void free(GPUPresentDevice& p_present_device, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator)
    {
        free_swap_chain_texture_dependant(*this, p_present_device, p_buffer_memory, p_graphics_allocator);
        free_swap_chain_texture_independant(*this, p_present_device, p_buffer_memory, p_graphics_allocator);
    };

    inline void realloc(GPUPresentDevice& p_present_device, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, const v3ui p_window_handle_dimensions)
    {
        free_swap_chain_texture_dependant(*this, p_present_device, p_buffer_memory, p_graphics_allocator);
        allocate_swap_chain_texture_dependendant(*this, p_present_device, p_buffer_memory, p_graphics_allocator, p_window_handle_dimensions);
    };

  private:
    inline static void allocate_swap_chain_texture_independant(SwapChain& p_swap_chain, GPUPresentDevice& p_present_device, GraphicsAllocator2& p_graphics_allocator,
                                                               const Token(TextureGPU) p_presented_texture, const Slice<int8>& p_compiled_vertex_shader, const Slice<int8>& p_compiled_fragment_shader)
    {
        p_swap_chain.swap_chain_next_image_semaphore = Semafore::allocate(p_present_device.device);

        Declare_sized_slice(ShaderLayoutParameterType, 1, l_shader_parameter_layout_input_arr, l_shader_parameter_layout_input_slice, ShaderLayoutParameterType::TEXTURE_FRAGMENT);
        Span<ShaderLayoutParameterType> l_shader_parameter_layout = Span_allocate_slice(&l_shader_parameter_layout_input_slice);
        Declare_sized_slice(ShaderLayout::VertexInputParameter, 2, l_vertex_parameter_arr, l_vertex_parameter_slice, ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, 0},
                            ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_2, offsetof(GPUPresent_2DQuad::_vertex, uv)});
        Span<ShaderLayout::VertexInputParameter> l_vertex_parameter = Span_allocate_slice(&l_vertex_parameter_slice);
        p_swap_chain.image_copy_shader_layout = p_graphics_allocator.allocate_shader_layout(l_shader_parameter_layout, l_vertex_parameter, sizeof(GPUPresent_2DQuad::_vertex));

        p_swap_chain.image_copy_shader_vertex = p_graphics_allocator.allocate_shader_module(p_compiled_vertex_shader);
        p_swap_chain.image_copy_shader_fragment = p_graphics_allocator.allocate_shader_module(p_compiled_fragment_shader);

        p_swap_chain.shader_parameter_texture =
            p_graphics_allocator.allocate_shadertexturegpu_parameter(ShaderLayoutParameterType::TEXTURE_FRAGMENT, p_presented_texture, p_graphics_allocator.heap.textures_gpu.get(p_presented_texture));
    };

    inline static void free_swap_chain_texture_independant(SwapChain& p_swap_chain, GPUPresentDevice& p_present_device, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator)
    {
        p_swap_chain.swap_chain_next_image_semaphore.free(p_present_device.device);

        p_graphics_allocator.free_shadertexturegpu_parameter(p_buffer_memory.allocator.device, p_swap_chain.shader_parameter_texture,
                                                             p_graphics_allocator.heap.shader_texture_gpu_parameters.get(p_swap_chain.shader_parameter_texture));
        p_graphics_allocator.free_shader_module(p_swap_chain.image_copy_shader_vertex);
        p_graphics_allocator.free_shader_module(p_swap_chain.image_copy_shader_fragment);
        p_graphics_allocator.free_shader_layout(p_swap_chain.image_copy_shader_layout);
    };

    inline static void allocate_swap_chain_texture_dependendant(SwapChain& p_swap_chain, GPUPresentDevice& p_present_device, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator,
                                                                const v3ui p_window_handle_dimensions)
    {
        p_swap_chain.swap_chain_dimensions =
            v3ui{Math::clamp_ui32(p_window_handle_dimensions.x, p_present_device.surface_capacilities.minImageExtent.width, p_present_device.surface_capacilities.maxImageExtent.width),
                 Math::clamp_ui32(p_window_handle_dimensions.y, p_present_device.surface_capacilities.minImageExtent.height, p_present_device.surface_capacilities.maxImageExtent.height), 1};

        VkSwapchainCreateInfoKHR l_swapchain_create{};
        l_swapchain_create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        l_swapchain_create.pNext = NULL;
        l_swapchain_create.surface = p_present_device.surface;
        l_swapchain_create.minImageCount = p_present_device.surface_capacilities.minImageCount;
        l_swapchain_create.imageFormat = p_present_device.surface_format.format;
        l_swapchain_create.imageColorSpace = p_present_device.surface_format.colorSpace;
        l_swapchain_create.imageExtent = VkExtent2D{p_swap_chain.swap_chain_dimensions.x, p_swap_chain.swap_chain_dimensions.y};
        l_swapchain_create.imageArrayLayers = 1;
        l_swapchain_create.imageUsage = VkImageUsageFlagBits ::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        l_swapchain_create.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
        l_swapchain_create.queueFamilyIndexCount = 0;
        l_swapchain_create.pQueueFamilyIndices = NULL;
        l_swapchain_create.preTransform = p_present_device.surface_capacilities.currentTransform;
        l_swapchain_create.compositeAlpha = VkCompositeAlphaFlagBitsKHR ::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        l_swapchain_create.presentMode = p_present_device.present_mode;
        l_swapchain_create.clipped = true;
        l_swapchain_create.oldSwapchain = NULL;
        vk_handle_result(vkCreateSwapchainKHR(p_present_device.device, &l_swapchain_create, NULL, &p_swap_chain.swap_chain));

        // l_present_device.present_mode =  VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

        Span<VkImage> l_images = vk::getSwapchainImagesKHR(p_present_device.device, p_swap_chain.swap_chain);
        p_swap_chain.swap_chain_images = Span_allocate<Token(ImageGPU)>(l_images.Capacity);
        p_swap_chain.rendertarget_copy_pass = Span_allocate<Token(GraphicsPass)>(l_images.Capacity);
        p_swap_chain.image_copy_shaders = Span_allocate<Token(Shader)>(l_images.Capacity);

        for (loop(i, 0, l_images.Capacity))
        {
            ImageFormat l_image_format = ImageFormat::build_color_2d(v3ui{l_swapchain_create.imageExtent.width, l_swapchain_create.imageExtent.height, 1}, ImageUsageFlag::SHADER_COLOR_ATTACHMENT);
            l_image_format.format = l_swapchain_create.imageFormat;
            *Span_get(&p_swap_chain.swap_chain_images, i) = p_buffer_memory.allocator.gpu_images.alloc_element(ImageGPU{TransferDeviceHeapToken{}, *Span_get(&l_images, i), l_image_format, 0});

            *Span_get(&p_swap_chain.rendertarget_copy_pass, i) = p_graphics_allocator.allocate_graphicspass<1>(
                p_buffer_memory.allocator.device, Span_get(&p_swap_chain.swap_chain_images, i), &p_buffer_memory.allocator.gpu_images.get(*Span_get(&p_swap_chain.swap_chain_images, i)),
                SliceN<RenderPassAttachment, 1>{RenderPassAttachment{AttachmentType::KHR, l_image_format}});
        }
        Span_free(&l_images);

        for (loop(i, 0, p_swap_chain.image_copy_shaders.Capacity))
        {
            *Span_get(&p_swap_chain.image_copy_shaders, i) = p_graphics_allocator.allocate_shader(ShaderAllocateInfo{
                p_graphics_allocator.heap.graphics_pass.get(*Span_get(&p_swap_chain.rendertarget_copy_pass, i)), ShaderConfiguration{0, ShaderConfiguration::CompareOp::Always},
                p_graphics_allocator.heap.shader_layouts.get(p_swap_chain.image_copy_shader_layout), p_graphics_allocator.heap.shader_modules.get(p_swap_chain.image_copy_shader_vertex),
                p_graphics_allocator.heap.shader_modules.get(p_swap_chain.image_copy_shader_fragment)});
        }
    };

    inline static void free_swap_chain_texture_dependant(SwapChain& p_swap_chain, GPUPresentDevice& p_present_device, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator)
    {
        vkDestroySwapchainKHR(p_present_device.device, p_swap_chain.swap_chain, NULL);
        for (loop(i, 0, p_swap_chain.image_copy_shaders.Capacity))
        {
            p_graphics_allocator.free_shader(*Span_get(&p_swap_chain.image_copy_shaders, i));
        }
        Span_free(&p_swap_chain.image_copy_shaders);

        for (loop(i, 0, p_swap_chain.rendertarget_copy_pass.Capacity))
        {
            p_graphics_allocator.free_graphicspass(p_buffer_memory.allocator.device, *Span_get(&p_swap_chain.rendertarget_copy_pass, i));
        }
        Span_free(&p_swap_chain.rendertarget_copy_pass);

        for (loop(i, 0, p_swap_chain.swap_chain_images.Capacity))
        {
            p_buffer_memory.allocator.gpu_images.release_element(*Span_get(&p_swap_chain.swap_chain_images, i));
        }
        Span_free(&p_swap_chain.swap_chain_images);
    };
};

/*
    The presentation is a direct usage of the GraphicsBinder.
*/
struct GPUPresent
{
    GPUPresentDevice device;
    SwapChain swap_chain;
    uint32 current_swapchain_image_index;
    GPUPresent_2DQuad d2_quad;

    inline static GPUPresent allocate(const GPUInstance& p_gpu_instance, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, const WindowHandle p_window_handle,
                                      const v3ui p_window_size, Token(TextureGPU) p_presented_texture, const Slice<int8>& p_compiled_vertex_shader, const Slice<int8>& p_compiled_fragment_shader)
    {
        GPUPresent l_gpu_present;
        l_gpu_present.device = GPUPresentDevice::allocate(p_gpu_instance, p_window_handle);
        l_gpu_present.swap_chain =
            SwapChain::allocate(l_gpu_present.device, p_buffer_memory, p_graphics_allocator, p_presented_texture, p_compiled_vertex_shader, p_compiled_fragment_shader, p_window_size);
        l_gpu_present.d2_quad = GPUPresent_2DQuad::allocate(p_buffer_memory);
        return l_gpu_present;
    };

    inline void free(const GPUInstance& p_gpu_instance, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator)
    {
        this->swap_chain.free(this->device, p_buffer_memory, p_graphics_allocator);
        this->d2_quad.free(p_buffer_memory);
        this->device.free(p_gpu_instance);
    };

    inline void resize(const v3ui p_window_size, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator)
    {
        this->device.recalculate_surface_capabilities();
        this->swap_chain.realloc(this->device, p_buffer_memory, p_graphics_allocator, p_window_size);
    };

    inline void graphics_step(GraphicsBinder& p_graphics_binder)
    {
        ImageGPU& l_presented_image = p_graphics_binder.buffer_allocator.gpu_images.get(
            p_graphics_binder.graphics_allocator.heap.textures_gpu.get(p_graphics_binder.graphics_allocator.heap.shader_texture_gpu_parameters.get(this->swap_chain.shader_parameter_texture).texture)
                .Image);
        BufferCommandUtils::cmd_image_layout_transition_v2(p_graphics_binder.graphics_allocator.graphics_device.command_buffer, p_graphics_binder.buffer_allocator.image_layout_barriers,
                                                           l_presented_image, l_presented_image.format.imageUsage, ImageUsageFlag::SHADER_TEXTURE_PARAMETER);

        vk_handle_result(vkAcquireNextImageKHR(this->device.device, this->swap_chain.swap_chain, 0, this->swap_chain.swap_chain_next_image_semaphore.semaphore, VK_NULL_HANDLE,
                                               &this->current_swapchain_image_index));

        Declare_sized_slice(v4f, 1, l_renderpass_clear_arr, l_renderpass_clear_slice, v4f{1.0f, 0, 0, 0});
        p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.heap.graphics_pass.get(*Span_get(&this->swap_chain.rendertarget_copy_pass, this->current_swapchain_image_index)),
                                            l_renderpass_clear_slice);
        p_graphics_binder.bind_shader(p_graphics_binder.graphics_allocator.heap.shaders.get(*Span_get(&this->swap_chain.image_copy_shaders, this->current_swapchain_image_index)));
        p_graphics_binder.bind_shadertexturegpu_parameter(p_graphics_binder.graphics_allocator.heap.shader_texture_gpu_parameters.get(this->swap_chain.shader_parameter_texture));
        this->d2_quad.bind_and_draw(p_graphics_binder);
        p_graphics_binder.pop_shadertexturegpu_parameter();
        p_graphics_binder.end_render_pass();

        BufferCommandUtils::cmd_image_layout_transition_v2(p_graphics_binder.graphics_allocator.graphics_device.command_buffer, p_graphics_binder.buffer_allocator.image_layout_barriers,
                                                           l_presented_image, ImageUsageFlag::SHADER_TEXTURE_PARAMETER, l_presented_image.format.imageUsage);
    };

    inline void present(const Semafore& p_awaited_semaphore)
    {
        Declare_sized_slice(Semafore, 2, l_awaited_semaphores_arr, l_awaited_semaphores, this->swap_chain.swap_chain_next_image_semaphore, p_awaited_semaphore);

        VkPresentInfoKHR l_present_info{};
        l_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        l_present_info.waitSemaphoreCount = (uint32)l_awaited_semaphores.Size;
        l_present_info.pWaitSemaphores = (VkSemaphore*)l_awaited_semaphores.Begin;
        l_present_info.swapchainCount = 1;
        l_present_info.pSwapchains = &this->swap_chain.swap_chain;
        l_present_info.pImageIndices = &this->current_swapchain_image_index;
        vk_handle_result(vkQueuePresentKHR(this->device.present_queue, &l_present_info));
    };
};