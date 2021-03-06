#pragma once

namespace v2
{

typedef int8* GPUPresentWindowHandle;

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

    inline static GPUPresentDevice allocate(const GPUInstance& p_instance, const GPUPresentWindowHandle p_window_handle)
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

        vk_handle_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_instance.graphics_card.device, l_present_device.surface, &l_present_device.surface_capacilities));

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
        l_queueFamilies.free();

#if GPU_DEBUG
        assert_true(l_present_queue_family_index != -1);
#endif

        vkGetDeviceQueue(l_present_device.device, l_present_queue_family_index, 0, &l_present_device.present_queue);

        l_present_device.present_mode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

        Span<VkSurfaceFormatKHR> l_surface_formats = vk::getPhysicalDeviceSurfaceFormatsKHR(p_instance.graphics_card.device, l_present_device.surface);
        for (loop(i, 0, l_surface_formats.Capacity))
        {
            VkSurfaceFormatKHR& l_surface_format = l_surface_formats.get(i);
            if (l_surface_format.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB && l_surface_format.colorSpace == VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                l_present_device.surface_format = l_surface_format;
                break;
            }
        }
        l_surface_formats.free();
        // vkGetPhysicalDeviceSurfaceFormatsKHR(p_instance.graphics_card.device, l_present_device.surface, )

        return l_present_device;
    };

    inline void free(const GPUInstance& p_instance)
    {
        vkDestroySurfaceKHR(p_instance.instance, this->surface, NULL);
    }
};

struct GPUPresent
{
    GPUPresentDevice device;

    VkSwapchainKHR swap_chain;
    Semafore swap_chain_next_image_semaphore;

    Span<Token(ImageGPU)> swap_chain_images;
    Span<Token(GraphicsPass)> rendertarget_copy_pass;
    uint32 current_swapchain_image_index;

    Token(ShaderLayout) image_copy_shader_layout;
    Span<Token(Shader)> image_copy_shaders;
    Token(ShaderModule) image_copy_shader_vertex;
    Token(ShaderModule) image_copy_shader_fragment;
    Token(ShaderTextureGPUParameter) shader_parameter_texture;
    Token(BufferGPU) image_vertices;
    Token(BufferGPU) image_indices;

    inline static GPUPresent allocate(const GPUInstance& p_gpu_instance, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator, const GPUPresentWindowHandle p_window_handle,
                                      const v3ui p_window_size, Token(TextureGPU) p_presented_texture)
    {
        GPUPresent l_gpu_present;
        l_gpu_present.device = GPUPresentDevice::allocate(p_gpu_instance, p_window_handle);

        VkSwapchainCreateInfoKHR l_swapchain_create{};
        l_swapchain_create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        l_swapchain_create.pNext = NULL;
        l_swapchain_create.surface = l_gpu_present.device.surface;
        l_swapchain_create.minImageCount = l_gpu_present.device.surface_capacilities.minImageCount;
        l_swapchain_create.imageFormat = l_gpu_present.device.surface_format.format;
        l_swapchain_create.imageColorSpace = l_gpu_present.device.surface_format.colorSpace;
        l_swapchain_create.imageExtent =
            VkExtent2D{Math::clamp_ui32(p_window_size.x, l_gpu_present.device.surface_capacilities.minImageExtent.width, l_gpu_present.device.surface_capacilities.maxImageExtent.width),
                       Math::clamp_ui32(p_window_size.y, l_gpu_present.device.surface_capacilities.minImageExtent.height, l_gpu_present.device.surface_capacilities.maxImageExtent.height)};
        l_swapchain_create.imageArrayLayers = 1;
        l_swapchain_create.imageUsage = VkImageUsageFlagBits ::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        l_swapchain_create.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
        l_swapchain_create.queueFamilyIndexCount = 0;
        l_swapchain_create.pQueueFamilyIndices = NULL;
        l_swapchain_create.preTransform = l_gpu_present.device.surface_capacilities.currentTransform;
        l_swapchain_create.compositeAlpha = VkCompositeAlphaFlagBitsKHR ::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        l_swapchain_create.presentMode = l_gpu_present.device.present_mode;
        l_swapchain_create.clipped = true;
        l_swapchain_create.oldSwapchain = NULL;
        vk_handle_result(vkCreateSwapchainKHR(l_gpu_present.device.device, &l_swapchain_create, NULL, &l_gpu_present.swap_chain));

        l_gpu_present.swap_chain_next_image_semaphore = Semafore::allocate(l_gpu_present.device.device);
        // l_present_device.present_mode =  VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

        Span<VkImage> l_images = vk::getSwapchainImagesKHR(l_gpu_present.device.device, l_gpu_present.swap_chain);
        l_gpu_present.swap_chain_images = Span<Token(ImageGPU)>::allocate(l_images.Capacity);
        l_gpu_present.rendertarget_copy_pass = Span<Token(GraphicsPass)>::allocate(l_images.Capacity);
        l_gpu_present.image_copy_shaders = Span<Token(Shader)>::allocate(l_images.Capacity);

        for (loop(i, 0, l_images.Capacity))
        {
            ImageFormat l_image_format = ImageFormat::build_color_2d(v3ui{l_swapchain_create.imageExtent.width, l_swapchain_create.imageExtent.height, 1}, ImageUsageFlag::SHADER_COLOR_ATTACHMENT);
            l_image_format.format = l_swapchain_create.imageFormat;
            l_gpu_present.swap_chain_images.get(i) = p_buffer_memory.allocator.gpu_images.alloc_element(ImageGPU{TransferDeviceHeapToken{}, l_images.get(i), l_image_format, 0});

            l_gpu_present.rendertarget_copy_pass.get(i) = p_graphics_allocator.allocate_graphicspass<1>(p_buffer_memory.allocator.device, &l_gpu_present.swap_chain_images.get(i),
                                                                                                        &p_buffer_memory.allocator.gpu_images.get(l_gpu_present.swap_chain_images.get(i)),
                                                                                                        SliceN<RenderPassAttachment, 1>{RenderPassAttachment{AttachmentType::KHR, l_image_format}});
        }
        l_images.free();

        struct _vertex
        {
            v3f pos;
            v2f uv;
        };

        Span<ShaderLayoutParameterType> l_shader_parameter_layout =
            Span<ShaderLayoutParameterType>::allocate_slice(SliceN<ShaderLayoutParameterType, 1>{ShaderLayoutParameterType::TEXTURE_FRAGMENT}.to_slice());
        Span<ShaderLayout::VertexInputParameter> l_vertex_parameter = Span<ShaderLayout::VertexInputParameter>::allocate_slice(SliceN<ShaderLayout::VertexInputParameter, 2>{
            ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, 0},
            ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_2, offsetof(_vertex, pos)}}.to_slice());
        l_gpu_present.image_copy_shader_layout = p_graphics_allocator.allocate_shader_layout(l_shader_parameter_layout, l_vertex_parameter, sizeof(_vertex));

        // TODO -> using the asset database for vertex, fragment
        File l_shader_file = File::open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/.tmp/QuadDrawVert.out"));
        Span<int8> l_compiled_shader = l_shader_file.read_file_allocate();
        l_gpu_present.image_copy_shader_vertex = p_graphics_allocator.allocate_shader_module(l_compiled_shader.slice);
        l_compiled_shader.free();
        l_shader_file.free();
        l_shader_file = File::open(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/.tmp/QuadDrawFrag.out"));
        l_compiled_shader = l_shader_file.read_file_allocate();
        l_gpu_present.image_copy_shader_fragment = p_graphics_allocator.allocate_shader_module(l_compiled_shader.slice);
        l_compiled_shader.free();
        l_shader_file.free();

        for (loop(i, 0, l_gpu_present.image_copy_shaders.Capacity))
        {
            l_gpu_present.image_copy_shaders.get(i) = p_graphics_allocator.allocate_shader(ShaderAllocateInfo{
                p_graphics_allocator.heap.graphics_pass.get(l_gpu_present.rendertarget_copy_pass.get(i)), ShaderConfiguration{0, ShaderConfiguration::CompareOp::Always},
                p_graphics_allocator.heap.shader_layouts.get(l_gpu_present.image_copy_shader_layout), p_graphics_allocator.heap.shader_modules.get(l_gpu_present.image_copy_shader_vertex),
                p_graphics_allocator.heap.shader_modules.get(l_gpu_present.image_copy_shader_fragment)});
        }

        l_gpu_present.shader_parameter_texture =
            p_graphics_allocator.allocate_shadertexturegpu_parameter(ShaderLayoutParameterType::TEXTURE_FRAGMENT, p_presented_texture, p_graphics_allocator.heap.textures_gpu.get(p_presented_texture));

        Slice<_vertex> l_vertices =
            SliceN<_vertex, 4>{
                _vertex{v3f{-1.0f, 1.0f, 0.0f}, v2f{0.0f, 1.0f}},
                _vertex{v3f{1.0f, -1.0f, 0.0f}, v2f{1.0f, 0.0f}},
                _vertex{v3f{-1.0f, -1.0f, 0.0f}, v2f{0.0f, 0.0f}},
                _vertex{v3f{1.0f, 1.0f, 0.0f}, v2f{1.0f, 1.0f}},
            }
                .to_slice();

        l_gpu_present.image_vertices = p_buffer_memory.allocator.allocate_buffergpu(l_vertices.build_asint8().Size,
                                                                                    (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::VERTEX | (BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_gpu_present.image_vertices, l_vertices.build_asint8());

        Slice<uint32> l_indices = SliceN<uint32, 6>{0, 1, 2, 0, 3, 1}.to_slice();
        l_gpu_present.image_indices = p_buffer_memory.allocator.allocate_buffergpu(l_indices.build_asint8().Size,
                                                                                   (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::INDEX | (BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE));
        BufferReadWrite::write_to_buffergpu(p_buffer_memory.allocator, p_buffer_memory.events, l_gpu_present.image_indices, l_indices.build_asint8());

        return l_gpu_present;
    };

    inline void free(const GPUInstance& p_gpu_instance, BufferMemory& p_buffer_memory, GraphicsAllocator2& p_graphics_allocator)
    {

        this->swap_chain_next_image_semaphore.free(this->device.device);
        vkDestroySwapchainKHR(this->device.device, this->swap_chain, NULL);
        p_graphics_allocator.free_shadertexturegpu_parameter(p_buffer_memory.allocator.device, this->shader_parameter_texture,
                                                             p_graphics_allocator.heap.shader_texture_gpu_parameters.get(this->shader_parameter_texture));
        p_graphics_allocator.free_shader_module(this->image_copy_shader_vertex);
        p_graphics_allocator.free_shader_module(this->image_copy_shader_fragment);
        p_graphics_allocator.free_shader_layout(this->image_copy_shader_layout);

        for (loop(i, 0, this->image_copy_shaders.Capacity))
        {
            p_graphics_allocator.free_shader(this->image_copy_shaders.get(i));
        }
        this->image_copy_shaders.free();

        for (loop(i, 0, this->rendertarget_copy_pass.Capacity))
        {
            p_graphics_allocator.free_graphicspass(p_buffer_memory.allocator.device, this->rendertarget_copy_pass.get(i));
            // GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(p_buffer_memory, p_graphics_allocator, this->rendertarget_copy_pass.get(i));
        }
        this->rendertarget_copy_pass.free();

        for (loop(i, 0, this->swap_chain_images.Capacity))
        {
            p_buffer_memory.allocator.gpu_images.release_element(this->swap_chain_images.get(i));
        }
        this->swap_chain_images.free();

        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, this->image_vertices);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(p_buffer_memory.allocator, p_buffer_memory.events, this->image_indices);
        this->device.free(p_gpu_instance);
    };

    inline void graphics_step(GraphicsBinder& p_graphics_binder)
    {
        ImageGPU& l_presented_image = p_graphics_binder.buffer_allocator.gpu_images.get(
            p_graphics_binder.graphics_allocator.heap.textures_gpu.get(p_graphics_binder.graphics_allocator.heap.shader_texture_gpu_parameters.get(this->shader_parameter_texture).texture).Image);
        BufferCommandUtils::cmd_image_layout_transition_v2(p_graphics_binder.graphics_allocator.graphics_device.command_buffer, p_graphics_binder.buffer_allocator.image_layout_barriers,
                                                           l_presented_image, l_presented_image.format.imageUsage, ImageUsageFlag::SHADER_TEXTURE_PARAMETER);

        vk_handle_result(vkAcquireNextImageKHR(this->device.device, this->swap_chain, 0, this->swap_chain_next_image_semaphore.semaphore, VK_NULL_HANDLE, &this->current_swapchain_image_index));

        p_graphics_binder.begin_render_pass(p_graphics_binder.graphics_allocator.heap.graphics_pass.get(this->rendertarget_copy_pass.get(this->current_swapchain_image_index)),
                                            SliceN<v4f, 1>{v4f{1.0f, 0, 0, 0}}.to_slice());
        p_graphics_binder.bind_shader(p_graphics_binder.graphics_allocator.heap.shaders.get(this->image_copy_shaders.get(this->current_swapchain_image_index)));
        p_graphics_binder.bind_shadertexturegpu_parameter(p_graphics_binder.graphics_allocator.heap.shader_texture_gpu_parameters.get(this->shader_parameter_texture));
        p_graphics_binder.bind_vertex_buffer_gpu(p_graphics_binder.buffer_allocator.gpu_buffers.get(this->image_vertices));
        p_graphics_binder.bind_index_buffer_gpu(p_graphics_binder.buffer_allocator.gpu_buffers.get(this->image_indices), BufferIndexType::UINT32);
        p_graphics_binder.draw_indexed(6);
        p_graphics_binder.pop_shadertexturegpu_parameter();
        p_graphics_binder.end_render_pass();

        BufferCommandUtils::cmd_image_layout_transition_v2(p_graphics_binder.graphics_allocator.graphics_device.command_buffer, p_graphics_binder.buffer_allocator.image_layout_barriers,
                                                           l_presented_image, ImageUsageFlag::SHADER_TEXTURE_PARAMETER, l_presented_image.format.imageUsage);
    };

    inline void present(const Semafore& p_awaited_semaphore)
    {

        Slice<Semafore> l_awaited_semaphores = SliceN<Semafore, 2>{this->swap_chain_next_image_semaphore, p_awaited_semaphore}.to_slice();

        VkPresentInfoKHR l_present_info{};
        l_present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        l_present_info.waitSemaphoreCount = (uint32)l_awaited_semaphores.Size;
        l_present_info.pWaitSemaphores = (VkSemaphore*)l_awaited_semaphores.Begin;
        l_present_info.swapchainCount = 1;
        l_present_info.pSwapchains = &this->swap_chain;
        l_present_info.pImageIndices = &this->current_swapchain_image_index;
        vk_handle_result(vkQueuePresentKHR(this->device.present_queue, &l_present_info));
    };
};
} // namespace v2