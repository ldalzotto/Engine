
#include "GPU/gpu.hpp"
#include "AssetCompiler/asset_compiler.hpp"

// #define RENDER_DOC_DEBUG

#ifdef RENDER_DOC_DEBUG

#include "renderdoc_app.h"

RENDERDOC_API_1_1_0* rdoc_api = NULL;

#endif

inline void gpu_buffer_allocation()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());

    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    const uimax l_tested_uimax_array[3] = {10, 20, 30};
    Slice<uimax> l_tested_uimax_slice = Slice_build_memory_elementnb<uimax>((uimax*)l_tested_uimax_array, 3);
    Slice<int8> l_tested_uimax_slice_int8 = Slice_build_asint8(&l_tested_uimax_slice);

    // allocating and releasing a BufferHost
    {
        uimax l_value = 20;
        Token(BufferHost) l_buffer_host_token = l_buffer_memory.allocator.allocate_bufferhost(l_tested_uimax_slice_int8, BufferUsageFlag::TRANSFER_READ);
        Slice<int8> l_buffer_host_token_memory = l_buffer_memory.allocator.host_buffers.get(l_buffer_host_token).get_mapped_memory();
        assert_true(Slice_compare(&l_buffer_host_token_memory, &l_tested_uimax_slice_int8));
        // We can write manually
        Slice<int8> l_buffer_memory_slice = l_buffer_memory.allocator.host_buffers.get(l_buffer_host_token).get_mapped_memory();
        *Slice_get(&l_buffer_memory_slice, 1) = 25;
        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_host_token);
    }

    // allocating and releasing a BufferGPU
    {
        Token(BufferGPU) l_buffer_gpu = l_buffer_memory.allocator.allocate_buffergpu(
            l_tested_uimax_slice_int8.Size, (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ));

        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu, l_tested_uimax_slice_int8);

        BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
        // We force command buffer submit just for the test
        l_buffer_memory.allocator.device.command_buffer.force_sync_execution();

        assert_true(l_buffer_memory.events.write_buffer_gpu_to_buffer_host_events.Size == 0);

        Token(BufferHost) l_read_buffer =
            BufferReadWrite::read_from_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu, l_buffer_memory.allocator.gpu_buffers.get(l_buffer_gpu));

        // it creates an gpu read event that will be consumed the next step
        assert_true(l_buffer_memory.events.write_buffer_gpu_to_buffer_host_events.Size == 1);

        BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
        // We force command buffer submit just for the test
        l_buffer_memory.allocator.device.command_buffer.force_sync_execution();

        assert_true(l_buffer_memory.events.write_buffer_gpu_to_buffer_host_events.Size == 0);
        Slice<int8> l_host_buffer_slice = l_buffer_memory.allocator.host_buffers.get(l_read_buffer).get_mapped_memory();
        assert_true(Slice_compare(&l_host_buffer_slice, &l_tested_uimax_slice_int8));

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_read_buffer);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu);
    }

    // creation of a BufferGPU deletion the same step
    // nothing should happen
    {
        assert_true(l_buffer_memory.events.write_buffer_host_to_buffer_gpu_events.Size == 0);

        Token(BufferGPU) l_buffer_gpu = l_buffer_memory.allocator.allocate_buffergpu(Slice_build_asint8(&l_tested_uimax_slice).Size, BufferUsageFlag::TRANSFER_WRITE);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu, Slice_build_asint8(&l_tested_uimax_slice));

        // it creates an gpu write event that will be consumed the next step
        assert_true(l_buffer_memory.events.write_buffer_host_to_buffer_gpu_events.Size == 1);

        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu);

        assert_true(l_buffer_memory.events.write_buffer_host_to_buffer_gpu_events.Size == 0);

        BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
        l_buffer_memory.allocator.device.command_buffer.force_sync_execution();
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
};

inline void gpu_image_allocation()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    const uimax l_pixels_count = 16 * 16;
    color l_pixels[l_pixels_count];
    Slice<color> l_pixels_slize = Slice_build_memory_elementnb<color>(l_pixels, l_pixels_count);
    Slice<int8> l_pixels_slize_int8 = Slice_build_asint8(&l_pixels_slize);
    for (loop(i, 1, l_pixels_slize.Size))
    {
        *Slice_get(&l_pixels_slize, i) = color{(uint8)i, (uint8)i, (uint8)i, (uint8)i};
    }

    ImageFormat l_imageformat;
    l_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
    l_imageformat.arrayLayers = 1;
    l_imageformat.format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
    l_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
    l_imageformat.mipLevels = 1;
    l_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
    l_imageformat.extent = v3ui{16, 16, 1};

    // allocating and releasing a ImageHost
    {
        l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_READ;
        Token(ImageHost) l_image_host_token =
            BufferAllocatorComposition::allocate_imagehost_and_push_creation_event(l_buffer_memory.allocator, l_buffer_memory.events, l_pixels_slize_int8, l_imageformat);
        ImageHost& l_image_host = l_buffer_memory.allocator.host_images.get(l_image_host_token);
        Slice<int8> l_image_host_buffer = l_image_host.get_mapped_memory();
        assert_true(Slice_compare(&l_image_host_buffer, &l_pixels_slize_int8));

        BufferAllocatorComposition::free_image_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_image_host_token);
    }

    // allocating and releasing a ImageGPU
    {
        l_imageformat.imageUsage = (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE);
        Token(ImageGPU) l_image_gpu = BufferAllocatorComposition::allocate_imagegpu_and_push_creation_event(l_buffer_memory.allocator, l_buffer_memory.events, l_imageformat);

        BufferReadWrite::write_to_imagegpu(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu, l_buffer_memory.allocator.gpu_images.get(l_image_gpu), Slice_build_asint8(&l_pixels_slize));

        BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
        // We force command buffer submit just for the test
        l_buffer_memory.allocator.device.command_buffer.force_sync_execution();

        assert_true(l_buffer_memory.events.write_image_gpu_to_buffer_host_events.Size == 0);

        Token(BufferHost) l_read_buffer =
            BufferReadWrite::read_from_imagegpu_to_buffer(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu, l_buffer_memory.allocator.gpu_images.get(l_image_gpu));

        // it creates an gpu read event that will be consumed the next step
        assert_true(l_buffer_memory.events.write_image_gpu_to_buffer_host_events.Size == 1);

        BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
        // We force command buffer submit just for the test
        l_buffer_memory.allocator.device.command_buffer.force_sync_execution();

        assert_true(l_buffer_memory.events.write_image_gpu_to_buffer_host_events.Size == 0);
        Slice<int8> l_read_buffer_mapped_memory = l_buffer_memory.allocator.host_buffers.get(l_read_buffer).get_mapped_memory();
        assert_true(Slice_compare(&l_read_buffer_mapped_memory, &l_pixels_slize_int8));

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_read_buffer);
        BufferAllocatorComposition::free_image_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu);
    }

    // creation of a BufferGPU deletion the same step
    // nothing should happen
    {
        assert_true(l_buffer_memory.events.write_buffer_host_to_image_gpu_events.Size == 0);

        l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_WRITE;
        Token(ImageGPU) l_image_gpu = BufferAllocatorComposition::allocate_imagegpu_and_push_creation_event(l_buffer_memory.allocator, l_buffer_memory.events, l_imageformat);
        BufferReadWrite::write_to_imagegpu(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu, l_buffer_memory.allocator.gpu_images.get(l_image_gpu), Slice_build_asint8(&l_pixels_slize));

        // it creates an gpu write event that will be consumed the next
        assert_true(l_buffer_memory.events.write_buffer_host_to_image_gpu_events.Size == 1);

        BufferAllocatorComposition::free_image_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu);

        assert_true(l_buffer_memory.events.write_buffer_host_to_image_gpu_events.Size == 0);

        BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
        l_buffer_memory.allocator.device.command_buffer.force_sync_execution();
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
};

/*
    Creates a GraphicsPass that only clear input attachments.
    We check that the attachment has well been cleared with the input color.
*/
inline void gpu_renderpass_clear()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    SliceN<RenderPassAttachment, 2> l_attachments = {
        RenderPassAttachment{AttachmentType::COLOR,
                             ImageFormat::build_color_2d(v3ui{32, 32, 1}, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT))},
        RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(v3ui{32, 32, 1}, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)}};
    Token(GraphicsPass) l_graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(l_buffer_memory, l_graphics_allocator, l_attachments);

    color l_clear_color = color{0, uint8_max, 51, uint8_max};

    {
        l_gpu_context.buffer_step_and_submit();

        v4f l_clear_values[2];
        l_clear_values[0] = v4f{0.0f, (float)l_clear_color.y / (float)uint8_max, (float)l_clear_color.z / (float)uint8_max, 1.0f};
        l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

        GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();
        l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice_build_memory_elementnb<v4f>(l_clear_values, 2));
        l_graphics_binder.end_render_pass();

        l_gpu_context.submit_graphics_binder(l_graphics_binder);
        l_gpu_context.wait_for_completion();
    }

    Token(BufferHost) l_color_attachment_value =
        GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);

    BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
    l_buffer_memory.allocator.device.command_buffer.force_sync_execution();

    Slice<int8> l_color_attachment_value_pixels_slice_int8 = l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory();
    Slice<color> l_color_attachment_value_pixels = Slice_cast<color>(&l_color_attachment_value_pixels_slice_int8);

    for (loop(i, 0, l_color_attachment_value_pixels.Size))
    {
        assert_true(Slice_get(&l_color_attachment_value_pixels, i)->sRGB_to_linear() == l_clear_color);
    }

    BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(l_buffer_memory, l_graphics_allocator, l_graphics_pass);

    l_gpu_context.free();
};

/*
    Creates a GraphicsPass with one Shader and draw vertices with Material.
    GraphicsPass :
        * Color and depth attachments
    Shader :
        * Vertex buffer (v3f)
        * Host uniform parameter
        * GPU uniform parameter
        * texture sampler paramter

    We check that the ouput color is the one awaited and calculated by the Shader.
*/
inline void gpu_draw()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        SliceN<RenderPassAttachment, 2> l_attachments = {
            RenderPassAttachment{AttachmentType::COLOR, ImageFormat::build_color_2d(v3ui{4, 4, 1}, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ |
                                                                                                                    (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT))},
            RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(v3ui{4, 4, 1}, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)}};
        Token(GraphicsPass) l_graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(l_buffer_memory, l_graphics_allocator, l_attachments);

        struct vertex_position
        {
            v3f position;
        };

        SliceN<vertex_position, 6> l_vertices = {vertex_position{v3f{-1.0f, 1.0f, 0.0f}}, vertex_position{v3f{1.0f, -1.0f, 0.0f}}, vertex_position{v3f{-1.0f, -1.0f, 0.0f}},
                                                 vertex_position{v3f{0.0f, 0.5f, 0.0f}},  vertex_position{v3f{0.5f, 0.0f, 0.0f}},  vertex_position{v3f{0.0f, 0.0f, 0.0f}}};
        Slice<vertex_position> l_vertices_slice = slice_from_slicen(&l_vertices);

        Token(BufferGPU) l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(
            Slice_build_asint8(&l_vertices_slice).Size,
            (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ | (BufferUsageFlags)BufferUsageFlag::VERTEX));
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, Slice_build_asint8(&l_vertices_slice));

        // Token(BufferHost) l_vertex_buffer =  l_buffer_allocator.allocate_bufferhost(l_vertices_slice.build_asint8(), BufferUsageFlag::VERTEX);

        Token(Shader) l_first_shader;
        Token(ShaderLayout) l_first_shader_layout;
        Token(ShaderModule) l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span_allocate<ShaderLayout::VertexInputParameter>(1);
            *Span_get(&l_shader_vertex_input_primitives, 0) = ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position)};

            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span_allocate<ShaderLayoutParameterType>(4);
            *Span_get(&l_first_shader_layout_parameters, 0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            *Span_get(&l_first_shader_layout_parameters, 1) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            *Span_get(&l_first_shader_layout_parameters, 2) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            *Span_get(&l_first_shader_layout_parameters, 3) = ShaderLayoutParameterType::TEXTURE_FRAGMENT;

            l_first_shader_layout = l_graphics_allocator.allocate_shader_layout(l_first_shader_layout_parameters, l_shader_vertex_input_primitives, sizeof(vertex_position));

            const int8* p_vertex_litteral =
						MULTILINE(\
#version 450 \n

								layout(location = 0) in vec3 pos; \n

								void main()\n
						{ \n
								gl_Position = vec4(pos.xyz, 0.5f);\n
						}\n
						);

            const int8* p_fragment_litteral =
						MULTILINE(\
#version 450\n

								struct Color { vec4 _val; }; \n

								layout(set = 0, binding = 0) uniform color0 { Color _global_color; }; \n
								layout(set = 1, binding = 0) uniform color { Color _v; }; \n
								layout(set = 2, binding = 0) uniform color2 { Color _v2; }; \n
								layout(set = 3, binding = 0) uniform sampler2D texSampler; \n

								layout(location = 0) out vec4 outColor;\n

								void main()\n
						{ \n
								outColor = (_global_color._val + _v._val + _v2._val) * texture(texSampler, vec2(0.5f, 0.5f));\n
						}\n
						);

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));

            l_vertex_first_shader_module = l_graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
            l_fragment_first_shader_module = l_graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

            l_vertex_shader_compiled.free();
            l_fragment_shader_compiled.free();

            ShaderAllocateInfo l_shader_allocate_info{l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), ShaderConfiguration{1, ShaderConfiguration::CompareOp::GreaterOrEqual},
                                                      l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_graphics_allocator.heap.shader_modules.get(l_vertex_first_shader_module),
                                                      l_graphics_allocator.heap.shader_modules.get(l_fragment_first_shader_module)};

            l_first_shader = l_graphics_allocator.allocate_shader(l_shader_allocate_info);
        }

        Token(ShaderLayout) l_global_shader_layout;
        {
            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span_allocate<ShaderLayoutParameterType>(1);
            *Span_get(&l_first_shader_layout_parameters, 0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            Span<ShaderLayout::VertexInputParameter> l_vertex_parameters = Span_build<ShaderLayout::VertexInputParameter>(NULL, 0);
            l_global_shader_layout = l_graphics_allocator.allocate_shader_layout(l_first_shader_layout_parameters, l_vertex_parameters, 0);
        }

        color_f l_global_color = color_f{0.0f, 0.3f, 0.0f, 0.0f};
        color_f l_mat_1_base_color = color_f{1.0f, 0.0f, 0.0f, 0.5f};
        color_f l_mat_1_added_color = color_f{0.0f, 0.0f, 1.0f, 0.5f};
        color l_multiplied_color_texture_color = color{100, 100, 100, 255}; // sRGB

        Material l_global_material = Material::allocate_empty(l_graphics_allocator, 0);
        l_global_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shaders.get(l_first_shader).layout, l_global_color);

        Material l_first_material = Material::allocate_empty(l_graphics_allocator, 1);
        l_first_material.add_and_allocate_buffer_gpu_parameter_typed(l_graphics_allocator, l_buffer_memory, l_graphics_allocator.heap.shaders.get(l_first_shader).layout, l_mat_1_base_color);
        l_first_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shaders.get(l_first_shader).layout,
                                                                      l_mat_1_added_color);

        {

            Span<color> l_colors = Span_allocate<color>(l_attachments.get(0).image_format.extent.x * l_attachments.get(0).image_format.extent.y);
            for (loop(i, 0, l_colors.Capacity))
            {
                *Span_get(&l_colors, i) = l_multiplied_color_texture_color;
            }
            l_first_material.add_and_allocate_texture_gpu_parameter(l_graphics_allocator, l_buffer_memory, l_graphics_allocator.heap.shaders.get(l_first_shader).layout,
                                                                    l_attachments.get(0).image_format, Slice_build_asint8(&l_colors.slice));
            Span_free(&l_colors);
        }

        color_f l_mat_2_base_color = color_f{0.0f, 0.0f, 0.0f};
        color_f l_mat_2_added_color = color_f{0.6f, 0.0f, 0.0f};

        Material l_second_material = Material::allocate_empty(l_graphics_allocator, 1);
        l_second_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shaders.get(l_first_shader).layout,
                                                                       l_mat_2_base_color);
        l_second_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shaders.get(l_first_shader).layout,
                                                                       l_mat_2_added_color);

        color l_clear_color = color{0, uint8_max, 51, uint8_max};

        {
            v4f l_clear_values[2];
            l_clear_values[0] = v4f{0.0f, 1.0f, (float)l_clear_color.z / (float)uint8_max, 1.0f};
            l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

            l_gpu_context.buffer_step_and_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();

            l_graphics_binder.bind_shader_layout(l_graphics_allocator.heap.shader_layouts.get(l_global_shader_layout));

            // l_graphics_binder.bind_shader(l_graphics_allocator.get_shader(l_global_shader));

            l_graphics_binder.bind_material(l_global_material);

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice_build_memory_elementnb<v4f>(l_clear_values, 2));

            l_graphics_binder.bind_shader(l_graphics_allocator.heap.shaders.get(l_first_shader));

            l_graphics_binder.bind_material(l_first_material);
            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw(3);
            l_graphics_binder.pop_material_bind(l_first_material);

            l_graphics_binder.bind_material(l_second_material);
            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw_offsetted(3, 3);
            l_graphics_binder.pop_material_bind(l_second_material);

            l_graphics_binder.end_render_pass();

            l_graphics_binder.pop_material_bind(l_global_material);

            l_gpu_context.submit_graphics_binder(l_graphics_binder);
        }

        {
            l_gpu_context.wait_for_completion();
        }

        Token(BufferHost) l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        {
            BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
            l_buffer_memory.allocator.device.command_buffer.submit();
            l_buffer_memory.allocator.device.command_buffer.wait_for_completion();
        }
        Slice<int8> l_color_attachment_value_pixels_slice_int8 = l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory();
        Slice<color> l_color_attachment_value_pixels = Slice_cast<color>(&l_color_attachment_value_pixels_slice_int8);

        color l_first_draw_awaited_color =
            ((l_global_color + l_mat_1_base_color + l_mat_1_added_color) * (l_multiplied_color_texture_color.to_color_f().sRGB_to_linear())).linear_to_sRGB().to_uint8_color();
        color l_second_draw_awaited_color =
            ((l_global_color + l_mat_2_base_color + l_mat_2_added_color) * (l_multiplied_color_texture_color.to_color_f().sRGB_to_linear())).linear_to_sRGB().to_uint8_color();

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 0) == l_first_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 1) == l_first_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 2) == l_first_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 3) == l_clear_color.linear_to_sRGB());

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 4) == l_first_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 5) == l_first_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 6) == l_clear_color.linear_to_sRGB());
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 7) == l_clear_color.linear_to_sRGB());

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 8) == l_first_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 9) == l_clear_color.linear_to_sRGB());
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 10) == l_second_draw_awaited_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 11) == l_clear_color.linear_to_sRGB());

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 12) == l_clear_color.linear_to_sRGB());
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 13) == l_clear_color.linear_to_sRGB());
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 14) == l_clear_color.linear_to_sRGB());
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 15) == l_clear_color.linear_to_sRGB());

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer);

        l_global_material.free_with_textures(l_graphics_allocator, l_buffer_memory);
        l_second_material.free_with_textures(l_graphics_allocator, l_buffer_memory);
        l_first_material.free_with_textures(l_graphics_allocator, l_buffer_memory);

        l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
        l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

        l_graphics_allocator.free_shader_layout(l_global_shader_layout);
        l_graphics_allocator.free_shader_layout(l_first_shader_layout);

        l_graphics_allocator.free_shader(l_first_shader);
        GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
    l_shader_compiler.free();
};

/*
    Creates a GraphicsPass with one Shader and draw vertices with depth comparison.
    GraphicsPass :
        * Color and depth attachments
    Shader :
        * Vertex buffer (v3f)
        * Host uniform parameter

    We check that the ouput color takes into account the depth value setted in vertices.
     We check the value of the depth buffer.
*/
inline void gpu_depth_compare_test()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        SliceN<RenderPassAttachment, 2> l_attachments = {
            RenderPassAttachment{AttachmentType::COLOR, ImageFormat::build_color_2d(v3ui{4, 4, 1}, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ |
                                                                                                                    (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT))},
            RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(v3ui{4, 4, 1}, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ |
                                                                                                                    (ImageUsageFlags)ImageUsageFlag::SHADER_DEPTH_ATTACHMENT))}};
        Token(GraphicsPass) l_graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(l_buffer_memory, l_graphics_allocator, l_attachments);

        struct vertex_position
        {
            v3f position;
        };

        SliceN<vertex_position, 6> l_vertices = {vertex_position{v3f{-1.0f, 1.0f, 0.0f}}, vertex_position{v3f{1.0f, -1.0f, 0.0f}}, vertex_position{v3f{-1.0f, -1.0f, 0.0f}},
                                                 vertex_position{v3f{0.0f, 0.5f, 0.1f}},  vertex_position{v3f{0.5f, 0.0f, 0.1f}},  vertex_position{v3f{-0.5f, -0.5f, 0.1f}}};
        Slice<vertex_position> l_vertices_slice = slice_from_slicen(&l_vertices);

        Token(BufferGPU) l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(
            Slice_build_asint8(&l_vertices_slice).Size,
            (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ | (BufferUsageFlags)BufferUsageFlag::VERTEX));
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, Slice_build_asint8(&l_vertices_slice));

        // Token(BufferHost) l_vertex_buffer =  l_buffer_allocator.allocate_bufferhost(l_vertices_slice.build_asint8(), BufferUsageFlag::VERTEX);

        Token(Shader) l_first_shader;
        Token(ShaderLayout) l_first_shader_layout;
        Token(ShaderModule) l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span_allocate<ShaderLayout::VertexInputParameter>(1);
            *Span_get(&l_shader_vertex_input_primitives, 0) = ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position)};

            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span_allocate<ShaderLayoutParameterType>(4);
            *Span_get(&l_first_shader_layout_parameters, 0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            *Span_get(&l_first_shader_layout_parameters, 1) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            *Span_get(&l_first_shader_layout_parameters, 2) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            *Span_get(&l_first_shader_layout_parameters, 3) = ShaderLayoutParameterType::TEXTURE_FRAGMENT;

            l_first_shader_layout = l_graphics_allocator.allocate_shader_layout(l_first_shader_layout_parameters, l_shader_vertex_input_primitives, sizeof(vertex_position));

            const int8* p_vertex_litteral =
						MULTILINE(\
#version 450 \n

								layout(location = 0) in vec4 pos; \n

								void main()\n
						{ \n
								gl_Position = vec4(pos.xyz, 0.5f);\n
						}\n
						);

            const int8* p_fragment_litteral =
						MULTILINE(\
#version 450\n

								struct Color { vec4 _val; }; \n

								layout(set = 0, binding = 0) uniform color0 { Color _color; }; \n

								layout(location = 0) out vec4 outColor;\n

								void main()\n
						{ \n
								outColor = _color._val;\n
						}\n
						);

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));

            l_vertex_first_shader_module = l_graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
            l_fragment_first_shader_module = l_graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

            l_vertex_shader_compiled.free();
            l_fragment_shader_compiled.free();

            ShaderAllocateInfo l_shader_allocate_info{l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), ShaderConfiguration{1, ShaderConfiguration::CompareOp::LessOrEqual},
                                                      l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_graphics_allocator.heap.shader_modules.get(l_vertex_first_shader_module),
                                                      l_graphics_allocator.heap.shader_modules.get(l_fragment_first_shader_module)};

            l_first_shader = l_graphics_allocator.allocate_shader(l_shader_allocate_info);
        }

        Material l_red_material = Material::allocate_empty(l_graphics_allocator, 0);
        l_red_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shaders.get(l_first_shader).layout,
                                                                    color_f{1.0f, 0.0f, 0.0f, 0.0f});
        Material l_green_material = Material::allocate_empty(l_graphics_allocator, 0);
        l_green_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shaders.get(l_first_shader).layout,
                                                                      color_f{0.0f, 1.0f, 0.0f, 0.0f});

        color l_clear_color = color{0, uint8_max, 51, uint8_max};

        {
            v4f l_clear_values[2];
            l_clear_values[0] = v4f{0.0f, 1.0f, (float)l_clear_color.z / (float)uint8_max, 1.0f};
            l_clear_values[1] = v4f{1.0f, 0.0f, 0.0f, 0.0f};

            l_gpu_context.buffer_step_and_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice_build_memory_elementnb<v4f>(l_clear_values, 2));

            l_graphics_binder.bind_shader(l_graphics_allocator.heap.shaders.get(l_first_shader));

            l_graphics_binder.bind_material(l_red_material);
            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw(3);
            l_graphics_binder.pop_material_bind(l_red_material);

            l_graphics_binder.bind_material(l_green_material);
            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw_offsetted(3, 3);
            l_graphics_binder.pop_material_bind(l_green_material);

            l_graphics_binder.end_render_pass();

            l_gpu_context.submit_graphics_binder(l_graphics_binder);
        }

        {
            l_gpu_context.wait_for_completion();
        }

        Token(BufferHost) l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        Token(BufferHost) l_depth_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 1);
        {
            BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
            l_buffer_memory.allocator.device.command_buffer.submit();
            l_buffer_memory.allocator.device.command_buffer.wait_for_completion();
        }

        {
            Slice<int8> l_color_attachment_value_pixels_slice_int8 = l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory();
            Slice<color> l_color_attachment_value_pixels = Slice_cast<color>(&l_color_attachment_value_pixels_slice_int8);

            color l_first_draw_awaited_color = v4f{1.0f, 0.0f, 0.0f, 0.0f}.sRGB_to_linear().to_uint8_color();
            color l_second_draw_awaited_color = v4f{0.0f, 1.0f, 0.0f, 0.0f}.sRGB_to_linear().to_uint8_color();

            assert_true(*Slice_get(&l_color_attachment_value_pixels, 0) == l_first_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 1) == l_first_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 2) == l_first_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 3) == l_clear_color.linear_to_sRGB());

            assert_true(*Slice_get(&l_color_attachment_value_pixels, 4) == l_first_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 5) == l_first_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 6) == l_second_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 7) == l_clear_color.linear_to_sRGB());

            assert_true(*Slice_get(&l_color_attachment_value_pixels, 8) == l_first_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 9) == l_second_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 10) == l_second_draw_awaited_color);
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 11) == l_clear_color.linear_to_sRGB());

            assert_true(*Slice_get(&l_color_attachment_value_pixels, 12) == l_clear_color.linear_to_sRGB());
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 13) == l_clear_color.linear_to_sRGB());
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 14) == l_clear_color.linear_to_sRGB());
            assert_true(*Slice_get(&l_color_attachment_value_pixels, 15) == l_clear_color.linear_to_sRGB());

            Slice<int8> l_depth_attachment_value_pixels_slice_int8 = l_buffer_memory.allocator.host_buffers.get(l_depth_attachment_value).get_mapped_memory();
            Slice<uint16> l_depth_attachment_value_pixels = Slice_cast<uint16>(&l_depth_attachment_value_pixels_slice_int8);

            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 0) == 0);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 1) == 0);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 2) == 0);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 3) == uint16_max);

            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 4) == 0);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 5) == 0);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 6) == 13107);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 7) == uint16_max);

            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 8) == 0);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 9) == 13107);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 10) == 13107);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 11) == uint16_max);

            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 12) == uint16_max);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 13) == uint16_max);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 14) == uint16_max);
            assert_true(*Slice_get(&l_depth_attachment_value_pixels, 15) == uint16_max);
        }

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);
        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_depth_attachment_value);

        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer);

        l_red_material.free_with_textures(l_graphics_allocator, l_buffer_memory);
        l_green_material.free_with_textures(l_graphics_allocator, l_buffer_memory);

        l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
        l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

        l_graphics_allocator.free_shader_layout(l_first_shader_layout);

        l_graphics_allocator.free_shader(l_first_shader);
        GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
    l_shader_compiler.free();
};

/*
    Creates a GraphicsPass with one Shader and draw vertices with depth comparison.
    GraphicsPass :
        * Color and depth attachments
    Shader :
        * Vertex buffer (v3f)
        * Index buffer (uint32)

    We check that the indexed mesh is well written in the color attachment.
*/
inline void gpu_draw_indexed()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();
#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        SliceN<RenderPassAttachment, 2> l_attachments = {
            RenderPassAttachment{AttachmentType::COLOR, ImageFormat::build_color_2d(v3ui{4, 4, 1}, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ |
                                                                                                                    (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT))},
            RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(v3ui{4, 4, 1}, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)}};
        Token(GraphicsPass) l_graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(l_buffer_memory, l_graphics_allocator, l_attachments);

        struct vertex_position
        {
            v3f position;
        };

        Declare_sized_slice(vertex_position, 4, l_vertices, l_vertices_slice, vertex_position{v3f{0.0f, 0.5f, 0.0f}}, vertex_position{v3f{0.5f, 0.0f, 0.0f}},
                                   vertex_position{v3f{0.0f, 0.0f, 0.0f}}, vertex_position{v3f{0.5f, 0.5f, 0.0f}});

        SliceN<uint32, 6> l_indices = {0, 1, 2, 0, 3, 1};
        Slice<uint32> l_indices_slice = slice_from_slicen(&l_indices);

        Token(BufferGPU) l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(
            Slice_build_asint8(&l_vertices_slice).Size,
            (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ | (BufferUsageFlags)BufferUsageFlag::VERTEX));
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, Slice_build_asint8(&l_vertices_slice));

        Token(BufferGPU) l_indices_buffer = l_buffer_memory.allocator.allocate_buffergpu(
            Slice_build_asint8(&l_indices_slice).Size, (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::INDEX));
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_indices_buffer, Slice_build_asint8(&l_indices_slice));

        Token(Shader) l_first_shader;
        Token(ShaderLayout) l_first_shader_layout;
        Token(ShaderModule) l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span_allocate<ShaderLayout::VertexInputParameter>(1);
            *Span_get(&l_shader_vertex_input_primitives, 0) = ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position)};

            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span_allocate<ShaderLayoutParameterType>(0);

            l_first_shader_layout = l_graphics_allocator.allocate_shader_layout(l_first_shader_layout_parameters, l_shader_vertex_input_primitives, sizeof(vertex_position));

            const int8* p_vertex_litteral =
						MULTILINE(\
#version 450 \n

								layout(location = 0) in vec3 pos; \n

								void main()\n
						{ \n
								gl_Position = vec4(pos.xyz, 0.5f);\n
						}\n
						);

            const int8* p_fragment_litteral = MULTILINE(#version 450\n layout(location = 0) out vec4 outColor;\n

                                                        void main()\n {
                                                            \n outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                                                            \n
                                                        }\n);

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));

            l_vertex_first_shader_module = l_graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
            l_fragment_first_shader_module = l_graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

            l_vertex_shader_compiled.free();
            l_fragment_shader_compiled.free();

            ShaderAllocateInfo l_shader_allocate_info{l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), ShaderConfiguration{1, ShaderConfiguration::CompareOp::GreaterOrEqual},
                                                      l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_graphics_allocator.heap.shader_modules.get(l_vertex_first_shader_module),
                                                      l_graphics_allocator.heap.shader_modules.get(l_fragment_first_shader_module)};

            l_first_shader = l_graphics_allocator.allocate_shader(l_shader_allocate_info);
        }

        color l_clear_color = color{0, 0, 0, 0};

        {
            v4f l_clear_values[2];
            l_clear_values[0] = l_clear_color.to_color_f();
            l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

            l_gpu_context.buffer_step_and_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice_build_memory_elementnb<v4f>(l_clear_values, 2));

            l_graphics_binder.bind_shader(l_graphics_allocator.heap.shaders.get(l_first_shader));

            l_graphics_binder.bind_index_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_indices_buffer), BufferIndexType::UINT32);
            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw_indexed(l_indices_slice.Size);

            l_graphics_binder.end_render_pass();

            l_gpu_context.submit_graphics_binder(l_graphics_binder);
        }

        {
            l_gpu_context.wait_for_completion();
        }

        Token(BufferHost) l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        {
            BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
            l_buffer_memory.allocator.device.command_buffer.submit();
            l_buffer_memory.allocator.device.command_buffer.wait_for_completion();
        }
        Slice<int8> l_color_attachment_value_pixels_slice_int8 = l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory();
        Slice<color> l_color_attachment_value_pixels = Slice_cast<color>(&l_color_attachment_value_pixels_slice_int8);

        color l_white = color{uint8_max, uint8_max, uint8_max, uint8_max};

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 0) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 1) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 2) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 3) == l_clear_color);

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 4) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 5) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 6) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 7) == l_clear_color);

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 8) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 9) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 10) == l_white);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 11) == l_white);

        assert_true(*Slice_get(&l_color_attachment_value_pixels, 12) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 13) == l_clear_color);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 14) == l_white);
        assert_true(*Slice_get(&l_color_attachment_value_pixels, 15) == l_white);

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);

        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_indices_buffer);

        l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
        l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

        l_graphics_allocator.free_shader_layout(l_first_shader_layout);

        l_graphics_allocator.free_shader(l_first_shader);
        GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
    l_shader_compiler.free();
};

/*
    Creates a GraphicsPass with one Shader and draw vertices with depth comparison.
    GraphicsPass :
        * Color and depth attachments
    Shader :
        * Vertex buffer (v3f + v2f)
        * Texture parameter

     We draw a quad and check that the texture is correctly sampled
*/
inline void gpu_texture_mapping()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice_build_default<GPUExtension>());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        v3ui l_render_extends = v3ui{16, 16, 1};

        SliceN<RenderPassAttachment, 2> l_attachments = {
            RenderPassAttachment{AttachmentType::COLOR, ImageFormat::build_color_2d(l_render_extends, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ |
                                                                                                                       (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT))},
            RenderPassAttachment{AttachmentType::DEPTH, ImageFormat::build_depth_2d(l_render_extends, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)}};
        Token(GraphicsPass) l_graphics_pass = GraphicsAllocatorComposition::allocate_graphicspass_with_associatedimages<2>(l_buffer_memory, l_graphics_allocator, l_attachments);

        struct vertex
        {
            v3f position;
            v2f uv;
        };

        SliceN<vertex, 6> l_vertices = {vertex{v3f{-0.5f, 0.5f, 0.0f}, v2f{0.0f, 1.0f}}, vertex{v3f{0.5f, -0.5f, 0.0f}, v2f{1.0f, 0.0f}}, vertex{v3f{-0.5f, -0.5f, 0.0f}, v2f{0.0f, 0.0f}},
                                        vertex{v3f{-0.5f, 0.5f, 0.0f}, v2f{0.0f, 1.0f}}, vertex{v3f{0.5f, 0.5f, 0.0f}, v2f{1.0f, 1.0f}},  vertex{v3f{0.5f, -0.5f, 0.0f}, v2f{1.0f, 0.0f}}};
        Slice<vertex> l_vertices_slice = slice_from_slicen(&l_vertices);

        Token(BufferGPU) l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(
            Slice_build_asint8(&l_vertices_slice).Size,
            (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ | (BufferUsageFlags)BufferUsageFlag::VERTEX));
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, Slice_build_asint8(&l_vertices_slice));

        /*
        Token(TextureGPU) l_texture = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(l_buffer_allocator, l_graphics_allocator,
                ImageFormat::build_color_2d(l_render_extends,
                        (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE |
        (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER)));

        Token(ImageGPU) l_texture_image = l_graphics_allocator.heap.textures_gpu.Slice_get(l_texture).Image;
        */

        Span<color> l_texture_pixels = Span_allocate<color>(l_render_extends.x * l_render_extends.y);
        for (loop(i, 0, l_texture_pixels.Capacity))
        {
            *Span_get(&l_texture_pixels, i) = (color_f{(float32)i, (float32)i, (float32)i, (float32)i} * (1.0f / l_texture_pixels.Capacity)).to_uint8_color();
        }

        // l_buffer_allocator.write_to_imagegpu(l_texture_image, l_buffer_allocator.get_imagegpu(l_texture_image), l_texture_pixels.slice.build_asint8());

        Token(Shader) l_first_shader;
        Token(ShaderLayout) l_first_shader_layout;
        Token(ShaderModule) l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            SliceN<ShaderLayout::VertexInputParameter, 2> l_shader_vertex_input_primitives_arr{
                ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex, position)},
                ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_2, offsetof(vertex, uv)}};
            Slice<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives_slice = slice_from_slicen(&l_shader_vertex_input_primitives_arr);
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span_allocate_slice(&l_shader_vertex_input_primitives_slice);

            SliceN<ShaderLayoutParameterType, 1> l_first_shader_layout_parameters_arr{ShaderLayoutParameterType::TEXTURE_FRAGMENT};
            Slice<ShaderLayoutParameterType> l_first_shader_layout_parameters_slice = slice_from_slicen(&l_first_shader_layout_parameters_arr);
            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span_allocate_slice(&l_first_shader_layout_parameters_slice);

            l_first_shader_layout = l_graphics_allocator.allocate_shader_layout(l_first_shader_layout_parameters, l_shader_vertex_input_primitives, sizeof(vertex));

            const int8* p_vertex_litteral =
						MULTILINE(\
#version 450 \n

								layout(location = 0) in vec3 pos; \n
								layout(location = 1) in vec2 uv; \n

								layout(location = 0) out vec2 out_uv;\n

								void main()\n
						{ \n
								out_uv = uv;\n
								gl_Position = vec4(pos.xyz, 0.5f);\n
						}\n
						);

            const int8* p_fragment_litteral =
						MULTILINE(\
#version 450\n

								layout(location = 0) in vec2 in_uv;\n

								layout(location = 0) out vec4 outColor;\n

								layout(set = 0, binding = 0) uniform sampler2D texSampler; \n

								void main()\n
						{ \n
								outColor = texture(texSampler, in_uv);\n
						}\n
						);

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));

            l_vertex_first_shader_module = l_graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
            l_fragment_first_shader_module = l_graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

            l_vertex_shader_compiled.free();
            l_fragment_shader_compiled.free();

            ShaderAllocateInfo l_shader_allocate_info{l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), ShaderConfiguration{1, ShaderConfiguration::CompareOp::GreaterOrEqual},
                                                      l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_graphics_allocator.heap.shader_modules.get(l_vertex_first_shader_module),
                                                      l_graphics_allocator.heap.shader_modules.get(l_fragment_first_shader_module)};

            l_first_shader = l_graphics_allocator.allocate_shader(l_shader_allocate_info);
        }

        Material l_material = Material::allocate_empty(l_graphics_allocator, 0);
        l_material.add_and_allocate_texture_gpu_parameter(l_graphics_allocator, l_buffer_memory, l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_attachments.get(0).image_format,
                                                          Slice_build_asint8(&l_texture_pixels.slice));

        color l_clear_color = color{0, 0, 0, 0};

        {
            v4f l_clear_values[2];
            l_clear_values[0] = l_clear_color.to_color_f();
            l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

            l_gpu_context.buffer_step_and_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice_build_memory_elementnb<v4f>(l_clear_values, 2));

            l_graphics_binder.bind_shader(l_graphics_allocator.heap.shaders.get(l_first_shader));

            l_graphics_binder.bind_material(l_material);

            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw(l_vertices.Size());

            l_graphics_binder.pop_material_bind(l_material);

            l_graphics_binder.end_render_pass();

            l_gpu_context.submit_graphics_binder(l_graphics_binder);
        }

        {
            l_gpu_context.wait_for_completion();
        }

        Token(BufferHost) l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        {
            BufferStep::step(l_buffer_memory.allocator, l_buffer_memory.events);
            l_buffer_memory.allocator.device.command_buffer.submit();
            l_buffer_memory.allocator.device.command_buffer.wait_for_completion();
        }
        Slice<int8> l_color_attachment_value_pixels_slice_int8 = l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory();
        Slice<color> l_color_attachment_value_pixels = Slice_cast<color>(&l_color_attachment_value_pixels_slice_int8);

        assert_true(Slice_compare(&l_color_attachment_value_pixels, &l_texture_pixels.slice));

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer);

        l_material.free_with_textures(l_graphics_allocator, l_buffer_memory);

        l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
        l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

        l_graphics_allocator.free_shader_layout(l_first_shader_layout);

        l_graphics_allocator.free_shader(l_first_shader);
        GraphicsAllocatorComposition::free_graphicspass_with_associatedimages(l_buffer_memory, l_graphics_allocator, l_graphics_pass);

        Span_free(&l_texture_pixels);
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
    l_shader_compiler.free();
};

inline void gpu_present()
{
    v3ui l_window_size = v3ui{100, 100, 1};
    Token(Window) l_window_token = WindowAllocator::allocate(l_window_size.x, l_window_size.y, Slice_int8_build_rawstr(""));
    Window& l_window = WindowAllocator::get_window(l_window_token);

    SliceN<GPUExtension, 1> l_gpu_extension_arr{GPUExtension::WINDOW_PRESENT};
    GPUContext l_gpu_context = GPUContext::allocate(slice_from_slicen(&l_gpu_extension_arr));
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

    const int8* p_vertex_litteral = MULTILINE(#version 450 \n

                                              layout(location = 0) in vec3 pos; \n
                                              layout(location = 1) in vec2 uv; \n

                                              layout(location = 0) out vec2 out_uv; \n

                                              void main() { \n
                                                  gl_Position = vec4(pos.x, pos.y, 0.0, 1.0f); \n
                                                  out_uv = uv; \n
                                              } \n );

    const int8* p_fragment_litteral = MULTILINE(#version 450 \n

                                                layout(location = 0) in vec2 uv; \n

                                                layout(set = 0, binding = 0) uniform sampler2D texSampler; \n

                                                layout(location = 0) out vec4 outColor; \n

                                                void main() { outColor = texture(texSampler, uv); } \n );
    ShaderCompiled l_vertex_compilder = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, Slice_int8_build_rawstr(p_vertex_litteral));
    ShaderCompiled l_fragment_compilder = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, Slice_int8_build_rawstr(p_fragment_litteral));

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    v3ui l_render_target_size = {4, 4, 1};

    Token(TextureGPU) l_render_target_texture = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(
        l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator,
        ImageFormat::build_color_2d(l_render_target_size, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT | (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER |
                                                                           (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE)));

    // ShowWindow(hwnd, SW_NORMAL);
    GPUPresent l_gpu_present =
        GPUPresent::allocate(l_gpu_context.instance, l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_window.handle, v3ui{l_window.client_width, l_window.client_height, 1},
                             l_render_target_texture, l_vertex_compilder.get_compiled_binary(), l_fragment_compilder.get_compiled_binary());

    assert_true(l_gpu_present.device.surface_capacilities.currentExtent.width == l_window.client_width);
    assert_true(l_gpu_present.device.surface_capacilities.currentExtent.height == l_window.client_height);

    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;

    {

        Span<color> l_render_target_color = Span_allocate<color>(l_render_target_size.x * l_render_target_size.y);
        for (loop(i, 0, l_render_target_color.Capacity))
        {
            *Span_get(&l_render_target_color, i) = color{UINT8_MAX, 0, 0};
        }
        Token(ImageGPU) l_render_target_image = l_gpu_context.graphics_allocator.heap.textures_gpu.get(l_render_target_texture).Image;
        BufferReadWrite::write_to_imagegpu(l_gpu_context.buffer_memory.allocator, l_gpu_context.buffer_memory.events, l_render_target_image,
                                           l_gpu_context.buffer_memory.allocator.gpu_images.get(l_render_target_image), Slice_build_asint8(&l_render_target_color.slice));
        Span_free(&l_render_target_color);

        l_gpu_context.buffer_step_and_submit();
        GraphicsBinder l_binder = l_gpu_context.creates_graphics_binder();
        l_binder.start();
        l_gpu_present.graphics_step(l_binder);
        l_binder.end();
        l_gpu_context.submit_graphics_binder_and_notity_end(l_binder);
        l_gpu_present.present(l_gpu_context.graphics_end_semaphore);
        l_gpu_context.wait_for_completion();
    }
    {
        Span<color> l_render_target_color = Span_allocate<color>(l_render_target_size.x * l_render_target_size.y);
        for (loop(i, 0, l_render_target_color.Capacity))
        {
            *Span_get(&l_render_target_color, i) = color{0, 0, UINT8_MAX};
        }

        Token(ImageGPU) l_render_target_image = l_gpu_context.graphics_allocator.heap.textures_gpu.get(l_render_target_texture).Image;
        BufferReadWrite::write_to_imagegpu(l_gpu_context.buffer_memory.allocator, l_gpu_context.buffer_memory.events, l_render_target_image,
                                           l_gpu_context.buffer_memory.allocator.gpu_images.get(l_render_target_image), Slice_build_asint8(&l_render_target_color.slice));

        Span_free(&l_render_target_color);

        l_gpu_context.buffer_step_and_submit();
        GraphicsBinder l_binder = l_gpu_context.creates_graphics_binder();
        l_binder.start();
        l_gpu_present.graphics_step(l_binder);
        l_binder.end();
        l_gpu_context.submit_graphics_binder_and_notity_end(l_binder);
        l_gpu_present.present(l_gpu_context.graphics_end_semaphore);
        l_gpu_context.wait_for_completion();
    }

    // resize
    WindowNative::simulate_resize_appevent(l_window.handle, 200, 200);
    l_window.consume_resize_event();

    v3ui l_size_before = v3ui{l_gpu_present.device.surface_capacilities.currentExtent.width, l_gpu_present.device.surface_capacilities.currentExtent.height, 1};
    l_gpu_present.resize(v3ui{l_window.client_width, l_window.client_height, 1}, l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator);
    v3ui l_size_after = v3ui{l_gpu_present.device.surface_capacilities.currentExtent.width, l_gpu_present.device.surface_capacilities.currentExtent.height, 1};

    assert_true(l_size_after.x == l_window.client_width);
    assert_true(l_size_after.y == l_window.client_height);
    assert_true(l_size_before != l_size_after);

    {
        l_gpu_context.buffer_step_and_submit();
        GraphicsBinder l_binder = l_gpu_context.creates_graphics_binder();
        l_binder.start();
        l_gpu_present.graphics_step(l_binder);
        l_binder.end();
        l_gpu_context.submit_graphics_binder_and_notity_end(l_binder);
        l_gpu_present.present(l_gpu_context.graphics_end_semaphore);
        l_gpu_context.wait_for_completion();
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_vertex_compilder.free();
    l_fragment_compilder.free();
    l_shader_compiler.free();

    l_gpu_present.free(l_gpu_context.instance, l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator);
    GraphicsAllocatorComposition::free_texturegpu_with_imagegpu(l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_render_target_texture);
    l_gpu_context.free();

    WindowAllocator::free(l_window_token);
};

int main()
{
#ifdef RENDER_DOC_DEBUG
    HMODULE mod = GetModuleHandleA("renderdoc.dll");

    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_0, (void**)&rdoc_api);
    assert_true(ret == 1);
#endif

    gpu_buffer_allocation();
    gpu_image_allocation();
    gpu_renderpass_clear();
    gpu_draw();
    gpu_depth_compare_test();
    gpu_draw_indexed();
    gpu_texture_mapping();
    gpu_present();

    memleak_ckeck();
};
