
#include "GPU/gpu.hpp"
#include "AssetCompiler/asset_compiler.hpp"

// #define RENDER_DOC_DEBUG

#ifdef RENDER_DOC_DEBUG

#include "renderdoc_app.h"

RENDERDOC_API_1_1_0* rdoc_api = NULL;

#endif

inline void gpu_buffer_allocation()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());

    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    const uimax l_tested_uimax_array[3] = {10, 20, 30};
    Slice<uimax> l_tested_uimax_slice = Slice<uimax>::build_memory_elementnb((uimax*)l_tested_uimax_array, 3);

    // allocating and releasing a BufferHost
    {
        uimax l_value = 20;
        Token<BufferHost> l_buffer_host_token = l_buffer_memory.allocator.allocate_bufferhost(l_tested_uimax_slice.build_asint8(), BufferUsageFlag::TRANSFER_READ);
        assert_true(l_buffer_memory.allocator.host_buffers.get(l_buffer_host_token).get_mapped_memory().compare(l_tested_uimax_slice.build_asint8()));
        // We can write manually
        l_buffer_memory.allocator.host_buffers.get(l_buffer_host_token).get_mapped_memory().get(1) = 25;
        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_host_token);
    }

    // allocating and releasing a BufferGPU
    {
        Token<BufferGPU> l_buffer_gpu = l_buffer_memory.allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size, BufferUsageFlag::TRANSFER_WRITE | BufferUsageFlag::TRANSFER_READ);

        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu, l_tested_uimax_slice.build_asint8());

        l_gpu_context.buffer_step_force_execution();
        // We force command buffer submit just for the test

        assert_true(l_buffer_memory.events.write_buffer_gpu_to_buffer_host_events.Size == 0);

        Token<BufferHost> l_read_buffer =
            BufferReadWrite::read_from_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu, l_buffer_memory.allocator.gpu_buffers.get(l_buffer_gpu));

        // it creates an gpu read event that will be consumed the next step
        assert_true(l_buffer_memory.events.write_buffer_gpu_to_buffer_host_events.Size == 1);

        l_gpu_context.buffer_step_force_execution();
        // We force command buffer submit just for the test

        assert_true(l_buffer_memory.events.write_buffer_gpu_to_buffer_host_events.Size == 0);

        assert_true(l_buffer_memory.allocator.host_buffers.get(l_read_buffer).get_mapped_memory().compare(l_tested_uimax_slice.build_asint8()));

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_read_buffer);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu);
    }

    // creation of a BufferGPU deletion the same step
    // nothing should happen
    {
        assert_true(l_buffer_memory.events.write_buffer_host_to_buffer_gpu_events.Size == 0);

        Token<BufferGPU> l_buffer_gpu = l_buffer_memory.allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size, BufferUsageFlag::TRANSFER_WRITE);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu, l_tested_uimax_slice.build_asint8());

        // it creates an gpu write event that will be consumed the next step
        assert_true(l_buffer_memory.events.write_buffer_host_to_buffer_gpu_events.Size == 1);

        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_buffer_gpu);

        assert_true(l_buffer_memory.events.write_buffer_host_to_buffer_gpu_events.Size == 0);

        l_gpu_context.buffer_step_force_execution();
    }

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    l_gpu_context.free();
};

inline void gpu_image_allocation()
{
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    const uimax l_pixels_count = 16 * 16;
    color l_pixels[l_pixels_count];
    Slice<color> l_pixels_slize = Slice<color>::build_memory_elementnb(l_pixels, l_pixels_count);
    for (loop(i, 1, l_pixels_slize.Size))
    {
        l_pixels_slize.get(i) = color{(uint8)i, (uint8)i, (uint8)i, (uint8)i};
    }

    ImageFormat l_imageformat;
    l_imageformat.imageAspect = ImageAspectFlag::COLOR;
    l_imageformat.arrayLayers = 1;
    l_imageformat.format = ImageFormatFlag::R8G8B8A8_SRGB;
    l_imageformat.imageType = ImageType::_2D;
    l_imageformat.mipLevels = 1;
    l_imageformat.samples = ImageSampleCountFlag::_1;
    l_imageformat.extent = v3ui{16, 16, 1};

    // allocating and releasing a ImageHost
    {
        l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_READ;
        Token<ImageHost> l_image_host_token =
            BufferAllocatorComposition::allocate_imagehost_and_push_creation_event(l_buffer_memory.allocator, l_buffer_memory.events, l_pixels_slize.build_asint8(), l_imageformat);
        ImageHost& l_image_host = l_buffer_memory.allocator.host_images.get(l_image_host_token);

        assert_true(l_image_host.get_mapped_memory().compare(l_pixels_slize.build_asint8()));

        BufferAllocatorComposition::free_image_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_image_host_token);
    }

    // allocating and releasing a ImageGPU
    {
        l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::TRANSFER_WRITE;
        Token<ImageGPU> l_image_gpu = BufferAllocatorComposition::allocate_imagegpu_and_push_creation_event(l_buffer_memory.allocator, l_buffer_memory.events, l_imageformat);

        BufferReadWrite::write_to_imagegpu(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu, l_buffer_memory.allocator.gpu_images.get(l_image_gpu), l_pixels_slize.build_asint8());

        // We force command buffer submit just for the test
        l_gpu_context.buffer_step_force_execution();

        assert_true(l_buffer_memory.events.write_image_gpu_to_buffer_host_events.Size == 0);

        Token<BufferHost> l_read_buffer =
            BufferReadWrite::read_from_imagegpu_to_buffer(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu, l_buffer_memory.allocator.gpu_images.get(l_image_gpu));

        // it creates an gpu read event that will be consumed the next step
        assert_true(l_buffer_memory.events.write_image_gpu_to_buffer_host_events.Size == 1);

        l_gpu_context.buffer_step_force_execution();

        assert_true(l_buffer_memory.events.write_image_gpu_to_buffer_host_events.Size == 0);

        assert_true(l_buffer_memory.allocator.host_buffers.get(l_read_buffer).get_mapped_memory().compare(l_pixels_slize.build_asint8()));

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_read_buffer);
        BufferAllocatorComposition::free_image_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu);
    }

    // creation of a BufferGPU deletion the same step
    // nothing should happen
    {
        assert_true(l_buffer_memory.events.write_buffer_host_to_image_gpu_events.Size == 0);

        l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_WRITE;
        Token<ImageGPU> l_image_gpu = BufferAllocatorComposition::allocate_imagegpu_and_push_creation_event(l_buffer_memory.allocator, l_buffer_memory.events, l_imageformat);
        BufferReadWrite::write_to_imagegpu(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu, l_buffer_memory.allocator.gpu_images.get(l_image_gpu), l_pixels_slize.build_asint8());

        // it creates an gpu write event that will be consumed the next
        assert_true(l_buffer_memory.events.write_buffer_host_to_image_gpu_events.Size == 1);

        BufferAllocatorComposition::free_image_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_image_gpu);

        assert_true(l_buffer_memory.events.write_buffer_host_to_image_gpu_events.Size == 0);

        l_gpu_context.buffer_step_force_execution();
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
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif
    Token<TextureGPU> l_color_texture = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(
        l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator,
        ImageFormat::build_color_2d(v3ui{32, 32, 1}, ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::SHADER_COLOR_ATTACHMENT));
    Token<TextureGPU> l_depth_texture = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator,
                                                                                                        ImageFormat::build_depth_2d(v3ui{32, 32, 1}, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT));

    // Token<ImageGPU> l_color_image = l_gpu_context.buffer_memory.allocator.allocate_imagegpu();
    // Token<TextureGPU> l_color_texture = l_gpu_context.graphics_allocator.texturegpu_allocate(l_gpu_context.buffer_memory.allocator.device, );

    Token<GraphicsPass> l_graphics_pass;
    {
        GraphicsPassAllocationComposition::RenderPassAttachmentInput<2> l_renderpasss_allocation_input = {
            SliceN<Token<TextureGPU>, 2>{l_color_texture, l_depth_texture}, SliceN<AttachmentType, 2>{AttachmentType::COLOR, AttachmentType::DEPTH},
            SliceN<RenderPassAttachment::ClearOp, 2>{RenderPassAttachment::ClearOp::CLEARED, RenderPassAttachment::ClearOp::CLEARED}};
        l_graphics_pass = GraphicsPassAllocationComposition::allocate_renderpass_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_renderpasss_allocation_input);
    }

    color l_clear_color = color{0, uint8_max, 51, uint8_max};

    {
        l_gpu_context.buffer_step_submit();

        v4f l_clear_values[2];
        l_clear_values[0] = v4f{0.0f, (float)l_clear_color.y / (float)uint8_max, (float)l_clear_color.z / (float)uint8_max, 1.0f};
        l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

        GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();
        l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));
        l_graphics_binder.end_render_pass();

        l_gpu_context.submit_graphics_binder(l_graphics_binder);
        l_gpu_context.wait_for_completion();
    }

    Token<BufferHost> l_color_attachment_value =
        GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);

    l_gpu_context.buffer_step_force_execution();

    Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory());

    for (loop(i, 0, l_color_attachment_value_pixels.Size))
    {
        assert_true(l_color_attachment_value_pixels.get(i).sRGB_to_linear() == l_clear_color);
    }

    BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);

    // We instanciate another graphics pass that doesn't clear the buffer and specify a different clear color.
    // The render target texture shoudl stay the same.

#if 1
    Token<GraphicsPass> l_not_cleared_graphics_pass;
    {
        GraphicsPassAllocationComposition::RenderPassAttachmentInput<2> l_renderpasss_allocation_input = {
            SliceN<Token<TextureGPU>, 2>{l_color_texture, l_depth_texture}, SliceN<AttachmentType, 2>{AttachmentType::COLOR, AttachmentType::DEPTH},
            SliceN<RenderPassAttachment::ClearOp, 2>{RenderPassAttachment::ClearOp::NOT_CLEARED, RenderPassAttachment::ClearOp::CLEARED}};
        l_not_cleared_graphics_pass = GraphicsPassAllocationComposition::allocate_renderpass_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_renderpasss_allocation_input);
    }

    {
        color l_not_cleared_color = color{uint8_max, 0, 51, uint8_max};
        l_gpu_context.buffer_step_submit();

        v4f l_clear_values[2];
        l_clear_values[0] = v4f{0.0f, (float)l_not_cleared_color.y / (float)uint8_max, (float)l_not_cleared_color.z / (float)uint8_max, 1.0f};
        l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

        GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();
        l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_not_cleared_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));
        l_graphics_binder.end_render_pass();

        l_gpu_context.submit_graphics_binder(l_graphics_binder);
        l_gpu_context.wait_for_completion();
    }

    l_color_attachment_value = GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);

    l_gpu_context.buffer_step_force_execution();

    l_color_attachment_value_pixels = slice_cast<color>(l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory());

    // THe color of the color attachment is the same
    for (loop(i, 0, l_color_attachment_value_pixels.Size))
    {
        assert_true(l_color_attachment_value_pixels.get(i).sRGB_to_linear() == l_clear_color);
    }

    BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);

#endif

#ifdef RENDER_DOC_DEBUG
    rdoc_api->EndFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    GraphicsPassAllocationComposition::free_attachmentimages_then_attachmenttextures_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
    GraphicsPassAllocationComposition::free_graphicspass(l_graphics_allocator, l_not_cleared_graphics_pass);

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
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        SliceN<AttachmentType, 2> l_attachment_types = {AttachmentType::COLOR, AttachmentType::DEPTH};
        SliceN<ImageFormat, 2> l_image_formats = {ImageFormat::build_color_2d(v3ui{4, 4, 1}, ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::SHADER_COLOR_ATTACHMENT),
                                                  ImageFormat::build_depth_2d(v3ui{4, 4, 1}, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)};
        SliceN<RenderPassAttachment::ClearOp, 2> l_clears = {RenderPassAttachment::ClearOp::CLEARED, RenderPassAttachment::ClearOp::CLEARED};
        Token<GraphicsPass> l_graphics_pass = GraphicsPassAllocationComposition::allocate_attachmentimages_then_attachmenttextures_then_renderpass_then_graphicspass<2>(
            l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_attachment_types, l_image_formats, l_clears);

        struct vertex_position
        {
            v3f position;
        };

        SliceN<vertex_position, 6> l_vertices = {vertex_position{v3f{-1.0f, 1.0f, 0.0f}}, vertex_position{v3f{1.0f, -1.0f, 0.0f}}, vertex_position{v3f{-1.0f, -1.0f, 0.0f}},
                                                 vertex_position{v3f{0.0f, 0.5f, 0.0f}},  vertex_position{v3f{0.5f, 0.0f, 0.0f}},  vertex_position{v3f{0.0f, 0.0f, 0.0f}}};

        Token<BufferGPU> l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(slice_from_slicen(&l_vertices).build_asint8().Size,
                                                                                        BufferUsageFlag::TRANSFER_WRITE | BufferUsageFlag::TRANSFER_READ | BufferUsageFlag::VERTEX);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, slice_from_slicen(&l_vertices).build_asint8());

        // Token<BufferHost> l_vertex_buffer =  l_buffer_allocator.allocate_bufferhost(l_vertices_slice.build_asint8(), BufferUsageFlag::VERTEX);

        Token<Shader> l_first_shader;
        Token<ShaderLayout> l_first_shader_layout;
        Token<ShaderModule> l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span<ShaderLayout::VertexInputParameter>::allocate(1);
            l_shader_vertex_input_primitives.get(0) = ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position)};

            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span<ShaderLayoutParameterType>::allocate(4);
            l_first_shader_layout_parameters.get(0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            l_first_shader_layout_parameters.get(1) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            l_first_shader_layout_parameters.get(2) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            l_first_shader_layout_parameters.get(3) = ShaderLayoutParameterType::TEXTURE_FRAGMENT;

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

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

            l_vertex_first_shader_module = l_graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
            l_fragment_first_shader_module = l_graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

            l_vertex_shader_compiled.free();
            l_fragment_shader_compiled.free();

            ShaderAllocateInfo l_shader_allocate_info{l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), ShaderConfiguration{1, ShaderConfiguration::CompareOp::GreaterOrEqual},
                                                      l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_graphics_allocator.heap.shader_modules.get(l_vertex_first_shader_module),
                                                      l_graphics_allocator.heap.shader_modules.get(l_fragment_first_shader_module)};

            l_first_shader = l_graphics_allocator.allocate_shader(l_shader_allocate_info);
        }

        Token<ShaderLayout> l_global_shader_layout;
        {
            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span<ShaderLayoutParameterType>::allocate(1);
            l_first_shader_layout_parameters.get(0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            Span<ShaderLayout::VertexInputParameter> l_vertex_parameters = Span<ShaderLayout::VertexInputParameter>::build(NULL, 0);
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

            Span<color> l_colors = Span<color>::allocate(l_image_formats.get(0).extent.x * l_image_formats.get(0).extent.y);
            for (loop(i, 0, l_colors.Capacity))
            {
                l_colors.get(i) = l_multiplied_color_texture_color;
            }
            l_first_material.add_and_allocate_texture_gpu_parameter(l_graphics_allocator, l_buffer_memory, l_graphics_allocator.heap.shaders.get(l_first_shader).layout, l_image_formats.get(0),
                                                                    l_colors.slice.build_asint8());
            l_colors.free();
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

            l_gpu_context.buffer_step_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();

            l_graphics_binder.bind_shader_layout(l_graphics_allocator.heap.shader_layouts.get(l_global_shader_layout));

            // l_graphics_binder.bind_shader(l_graphics_allocator.get_shader(l_global_shader));

            l_graphics_binder.bind_material(l_global_material);

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));

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

        Token<BufferHost> l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        {
            l_gpu_context.buffer_step_force_execution();
        }
        Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory());

        color l_first_draw_awaited_color =
            ((l_global_color + l_mat_1_base_color + l_mat_1_added_color) * (l_multiplied_color_texture_color.to_color_f().sRGB_to_linear())).linear_to_sRGB().to_uint8_color();
        color l_second_draw_awaited_color =
            ((l_global_color + l_mat_2_base_color + l_mat_2_added_color) * (l_multiplied_color_texture_color.to_color_f().sRGB_to_linear())).linear_to_sRGB().to_uint8_color();

        assert_true(l_color_attachment_value_pixels.get(0) == l_first_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(1) == l_first_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(2) == l_first_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(3) == l_clear_color.linear_to_sRGB());

        assert_true(l_color_attachment_value_pixels.get(4) == l_first_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(5) == l_first_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(6) == l_clear_color.linear_to_sRGB());
        assert_true(l_color_attachment_value_pixels.get(7) == l_clear_color.linear_to_sRGB());

        assert_true(l_color_attachment_value_pixels.get(8) == l_first_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(9) == l_clear_color.linear_to_sRGB());
        assert_true(l_color_attachment_value_pixels.get(10) == l_second_draw_awaited_color);
        assert_true(l_color_attachment_value_pixels.get(11) == l_clear_color.linear_to_sRGB());

        assert_true(l_color_attachment_value_pixels.get(12) == l_clear_color.linear_to_sRGB());
        assert_true(l_color_attachment_value_pixels.get(13) == l_clear_color.linear_to_sRGB());
        assert_true(l_color_attachment_value_pixels.get(14) == l_clear_color.linear_to_sRGB());
        assert_true(l_color_attachment_value_pixels.get(15) == l_clear_color.linear_to_sRGB());

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
        GraphicsPassAllocationComposition::free_attachmentimages_then_attachmenttextures_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
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
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        SliceN<AttachmentType, 2> l_attachment_types = {AttachmentType::COLOR, AttachmentType::DEPTH};
        SliceN<ImageFormat, 2> l_image_formats = {ImageFormat::build_color_2d(v3ui{4, 4, 1}, ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::SHADER_COLOR_ATTACHMENT),
                                                  ImageFormat::build_depth_2d(v3ui{4, 4, 1}, ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)};
        SliceN<RenderPassAttachment::ClearOp, 2> l_clears = {RenderPassAttachment::ClearOp::CLEARED, RenderPassAttachment::ClearOp::CLEARED};
        Token<GraphicsPass> l_graphics_pass = GraphicsPassAllocationComposition::allocate_attachmentimages_then_attachmenttextures_then_renderpass_then_graphicspass<2>(
            l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_attachment_types, l_image_formats, l_clears);

        struct vertex_position
        {
            v3f position;
        };

        SliceN<vertex_position, 6> l_vertices = {vertex_position{v3f{-1.0f, 1.0f, 0.0f}}, vertex_position{v3f{1.0f, -1.0f, 0.0f}}, vertex_position{v3f{-1.0f, -1.0f, 0.0f}},
                                                 vertex_position{v3f{0.0f, 0.5f, 0.1f}},  vertex_position{v3f{0.5f, 0.0f, 0.1f}},  vertex_position{v3f{-0.5f, -0.5f, 0.1f}}};

        Token<BufferGPU> l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(slice_from_slicen(&l_vertices).build_asint8().Size,
                                                                                        BufferUsageFlag::TRANSFER_WRITE | BufferUsageFlag::TRANSFER_READ | BufferUsageFlag::VERTEX);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, slice_from_slicen(&l_vertices).build_asint8());

        // Token<BufferHost> l_vertex_buffer =  l_buffer_allocator.allocate_bufferhost(l_vertices_slice.build_asint8(), BufferUsageFlag::VERTEX);

        Token<Shader> l_first_shader;
        Token<ShaderLayout> l_first_shader_layout;
        Token<ShaderModule> l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span<ShaderLayout::VertexInputParameter>::allocate(1);
            l_shader_vertex_input_primitives.get(0) = ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position)};

            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span<ShaderLayoutParameterType>::allocate(4);
            l_first_shader_layout_parameters.get(0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            l_first_shader_layout_parameters.get(1) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            l_first_shader_layout_parameters.get(2) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
            l_first_shader_layout_parameters.get(3) = ShaderLayoutParameterType::TEXTURE_FRAGMENT;

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

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

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

            l_gpu_context.buffer_step_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));

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

        Token<BufferHost> l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        Token<BufferHost> l_depth_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 1);
        {
            l_gpu_context.buffer_step_force_execution();
        }

        {
            Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory());

            color l_first_draw_awaited_color = v4f{1.0f, 0.0f, 0.0f, 0.0f}.sRGB_to_linear().to_uint8_color();
            color l_second_draw_awaited_color = v4f{0.0f, 1.0f, 0.0f, 0.0f}.sRGB_to_linear().to_uint8_color();

            assert_true(l_color_attachment_value_pixels.get(0) == l_first_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(1) == l_first_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(2) == l_first_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(3) == l_clear_color.linear_to_sRGB());

            assert_true(l_color_attachment_value_pixels.get(4) == l_first_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(5) == l_first_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(6) == l_second_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(7) == l_clear_color.linear_to_sRGB());

            assert_true(l_color_attachment_value_pixels.get(8) == l_first_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(9) == l_second_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(10) == l_second_draw_awaited_color);
            assert_true(l_color_attachment_value_pixels.get(11) == l_clear_color.linear_to_sRGB());

            assert_true(l_color_attachment_value_pixels.get(12) == l_clear_color.linear_to_sRGB());
            assert_true(l_color_attachment_value_pixels.get(13) == l_clear_color.linear_to_sRGB());
            assert_true(l_color_attachment_value_pixels.get(14) == l_clear_color.linear_to_sRGB());
            assert_true(l_color_attachment_value_pixels.get(15) == l_clear_color.linear_to_sRGB());

            Slice<uint16> l_depth_attachment_value_pixels = slice_cast<uint16>(l_buffer_memory.allocator.host_buffers.get(l_depth_attachment_value).get_mapped_memory());

            assert_true(l_depth_attachment_value_pixels.get(0) == 0);
            assert_true(l_depth_attachment_value_pixels.get(1) == 0);
            assert_true(l_depth_attachment_value_pixels.get(2) == 0);
            assert_true(l_depth_attachment_value_pixels.get(3) == uint16_max);

            assert_true(l_depth_attachment_value_pixels.get(4) == 0);
            assert_true(l_depth_attachment_value_pixels.get(5) == 0);
            assert_true(l_depth_attachment_value_pixels.get(6) == 13107);
            assert_true(l_depth_attachment_value_pixels.get(7) == uint16_max);

            assert_true(l_depth_attachment_value_pixels.get(8) == 0);
            assert_true(l_depth_attachment_value_pixels.get(9) == 13107);
            assert_true(l_depth_attachment_value_pixels.get(10) == 13107);
            assert_true(l_depth_attachment_value_pixels.get(11) == uint16_max);

            assert_true(l_depth_attachment_value_pixels.get(12) == uint16_max);
            assert_true(l_depth_attachment_value_pixels.get(13) == uint16_max);
            assert_true(l_depth_attachment_value_pixels.get(14) == uint16_max);
            assert_true(l_depth_attachment_value_pixels.get(15) == uint16_max);
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
        GraphicsPassAllocationComposition::free_attachmentimages_then_attachmenttextures_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
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
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();
#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        SliceN<AttachmentType, 2> l_attachment_types = {AttachmentType::COLOR, AttachmentType::DEPTH};
        SliceN<ImageFormat, 2> l_image_formats = {ImageFormat::build_color_2d(v3ui{4, 4, 1}, ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::SHADER_COLOR_ATTACHMENT),
                                                  ImageFormat::build_depth_2d(v3ui{4, 4, 1}, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)};
        SliceN<RenderPassAttachment::ClearOp, 2> l_clears = {RenderPassAttachment::ClearOp::CLEARED, RenderPassAttachment::ClearOp::CLEARED};
        Token<GraphicsPass> l_graphics_pass = GraphicsPassAllocationComposition::allocate_attachmentimages_then_attachmenttextures_then_renderpass_then_graphicspass<2>(
            l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_attachment_types, l_image_formats, l_clears);

        struct vertex_position
        {
            v3f position;
        };

        SliceN<vertex_position, 4> l_vertices = {vertex_position{v3f{0.0f, 0.5f, 0.0f}}, vertex_position{v3f{0.5f, 0.0f, 0.0f}}, vertex_position{v3f{0.0f, 0.0f, 0.0f}},
                                                 vertex_position{v3f{0.5f, 0.5f, 0.0f}}};
        SliceN<uint32, 6> l_indices = {0, 1, 2, 0, 3, 1};

        Token<BufferGPU> l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(slice_from_slicen(&l_vertices).build_asint8().Size,
                                                                                        BufferUsageFlag::TRANSFER_WRITE | BufferUsageFlag::TRANSFER_READ | BufferUsageFlag::VERTEX);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, slice_from_slicen(&l_vertices).build_asint8());

        Token<BufferGPU> l_indices_buffer = l_buffer_memory.allocator.allocate_buffergpu(slice_from_slicen(&l_indices).build_asint8().Size, BufferUsageFlag::TRANSFER_WRITE | BufferUsageFlag::INDEX);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_indices_buffer, slice_from_slicen(&l_indices).build_asint8());

        Token<Shader> l_first_shader;
        Token<ShaderLayout> l_first_shader_layout;
        Token<ShaderModule> l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span<ShaderLayout::VertexInputParameter>::allocate(1);
            l_shader_vertex_input_primitives.get(0) = ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position)};

            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span<ShaderLayoutParameterType>::allocate(0);

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

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

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

            l_gpu_context.buffer_step_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));

            l_graphics_binder.bind_shader(l_graphics_allocator.heap.shaders.get(l_first_shader));

            l_graphics_binder.bind_index_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_indices_buffer), BufferIndexType::UINT32);
            l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_memory.allocator.gpu_buffers.get(l_vertex_buffer));
            l_graphics_binder.draw_indexed(l_indices.Size());

            l_graphics_binder.end_render_pass();

            l_gpu_context.submit_graphics_binder(l_graphics_binder);
        }

        {
            l_gpu_context.wait_for_completion();
        }

        Token<BufferHost> l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        {
            l_gpu_context.buffer_step_force_execution();
        }
        Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory());

        color l_white = color{uint8_max, uint8_max, uint8_max, uint8_max};

        assert_true(l_color_attachment_value_pixels.get(0) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(1) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(2) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(3) == l_clear_color);

        assert_true(l_color_attachment_value_pixels.get(4) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(5) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(6) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(7) == l_clear_color);

        assert_true(l_color_attachment_value_pixels.get(8) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(9) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(10) == l_white);
        assert_true(l_color_attachment_value_pixels.get(11) == l_white);

        assert_true(l_color_attachment_value_pixels.get(12) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(13) == l_clear_color);
        assert_true(l_color_attachment_value_pixels.get(14) == l_white);
        assert_true(l_color_attachment_value_pixels.get(15) == l_white);

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);

        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_indices_buffer);

        l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
        l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

        l_graphics_allocator.free_shader_layout(l_first_shader_layout);

        l_graphics_allocator.free_shader(l_first_shader);
        GraphicsPassAllocationComposition::free_attachmentimages_then_attachmenttextures_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_graphics_pass);
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
    GPUContext l_gpu_context = GPUContext::allocate(Slice<GPUExtension>::build_default());
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    ShaderCompiler l_shader_compiler = ShaderCompiler::allocate();

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    {
        v3ui l_render_extends = v3ui{16, 16, 1};

        SliceN<AttachmentType, 2> l_attachment_types = {AttachmentType::COLOR, AttachmentType::DEPTH};
        SliceN<ImageFormat, 2> l_image_formats = {
            ImageFormat::build_color_2d(l_render_extends, ImageUsageFlag::TRANSFER_READ | ImageUsageFlag::SHADER_COLOR_ATTACHMENT),
            ImageFormat::build_depth_2d(l_render_extends, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT)};
        SliceN<RenderPassAttachment::ClearOp, 2> l_clears = {RenderPassAttachment::ClearOp::CLEARED, RenderPassAttachment::ClearOp::CLEARED};
        Token<GraphicsPass> l_graphics_pass = GraphicsPassAllocationComposition::allocate_attachmentimages_then_attachmenttextures_then_renderpass_then_graphicspass<2>(
            l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_attachment_types, l_image_formats, l_clears);

        struct vertex
        {
            v3f position;
            v2f uv;
        };

        SliceN<vertex, 6> l_vertices = {vertex{v3f{-0.5f, 0.5f, 0.0f}, v2f{0.0f, 1.0f}}, vertex{v3f{0.5f, -0.5f, 0.0f}, v2f{1.0f, 0.0f}}, vertex{v3f{-0.5f, -0.5f, 0.0f}, v2f{0.0f, 0.0f}},
                                        vertex{v3f{-0.5f, 0.5f, 0.0f}, v2f{0.0f, 1.0f}}, vertex{v3f{0.5f, 0.5f, 0.0f}, v2f{1.0f, 1.0f}},  vertex{v3f{0.5f, -0.5f, 0.0f}, v2f{1.0f, 0.0f}}};

        Token<BufferGPU> l_vertex_buffer = l_buffer_memory.allocator.allocate_buffergpu(slice_from_slicen(&l_vertices).build_asint8().Size,
                                                                                        BufferUsageFlag::TRANSFER_WRITE | BufferUsageFlag::TRANSFER_READ | BufferUsageFlag::VERTEX);
        BufferReadWrite::write_to_buffergpu(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer, slice_from_slicen(&l_vertices).build_asint8());

        /*
        Token<TextureGPU> l_texture = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(l_buffer_allocator, l_graphics_allocator,
                ImageFormat::build_color_2d(l_render_extends,
                        (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE |
        (ImageUsageFlags)ImageUsageFlag::SHADER_TEXTURE_PARAMETER)));

        Token<ImageGPU> l_texture_image = l_graphics_allocator.heap.textures_gpu.get(l_texture).Image;
        */

        Span<color> l_texture_pixels = Span<color>::allocate(l_render_extends.x * l_render_extends.y);
        for (loop(i, 0, l_texture_pixels.Capacity))
        {
            l_texture_pixels.get(i) = (color_f{(float32)i, (float32)i, (float32)i, (float32)i} * (1.0f / l_texture_pixels.Capacity)).to_uint8_color();
        }

        // l_buffer_allocator.write_to_imagegpu(l_texture_image, l_buffer_allocator.get_imagegpu(l_texture_image), l_texture_pixels.slice.build_asint8());

        Token<Shader> l_first_shader;
        Token<ShaderLayout> l_first_shader_layout;
        Token<ShaderModule> l_vertex_first_shader_module, l_fragment_first_shader_module;
        {
            SliceN<ShaderLayout::VertexInputParameter, 2> l_shader_vertex_input_primitives_arr{
                ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex, position)},
                ShaderLayout::VertexInputParameter{PrimitiveSerializedTypes::Type::FLOAT32_2, offsetof(vertex, uv)}};
            Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives =
                Span<ShaderLayout::VertexInputParameter>::allocate_slice(slice_from_slicen(&l_shader_vertex_input_primitives_arr));

            SliceN<ShaderLayoutParameterType, 1> l_first_shader_layout_parameters_arr{ShaderLayoutParameterType::TEXTURE_FRAGMENT};
            Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span<ShaderLayoutParameterType>::allocate_slice(slice_from_slicen(&l_first_shader_layout_parameters_arr));

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

            ShaderCompiled l_vertex_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
            ShaderCompiled l_fragment_shader_compiled = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

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
        l_material.add_and_allocate_texture_gpu_parameter(l_graphics_allocator, l_buffer_memory, l_graphics_allocator.heap.shader_layouts.get(l_first_shader_layout), l_image_formats.get(0),
                                                          l_texture_pixels.slice.build_asint8());

        color l_clear_color = color{0, 0, 0, 0};

        {
            v4f l_clear_values[2];
            l_clear_values[0] = l_clear_color.to_color_f();
            l_clear_values[1] = v4f{0.0f, 0.0f, 0.0f, 0.0f};

            l_gpu_context.buffer_step_submit();

            GraphicsBinder l_graphics_binder = l_gpu_context.build_graphics_binder();

            l_graphics_binder.begin_render_pass(l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));

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

        Token<BufferHost> l_color_attachment_value =
            GraphicsPassReader::read_graphics_pass_attachment_to_bufferhost(l_buffer_memory, l_graphics_allocator, l_graphics_allocator.heap.graphics_pass.get(l_graphics_pass), 0);
        {
            l_gpu_context.buffer_step_force_execution();
        }
        Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_memory.allocator.host_buffers.get(l_color_attachment_value).get_mapped_memory());

        assert_true(l_color_attachment_value_pixels.compare(l_texture_pixels.slice));

        BufferAllocatorComposition::free_buffer_host_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_color_attachment_value);
        BufferAllocatorComposition::free_buffer_gpu_and_remove_event_references(l_buffer_memory.allocator, l_buffer_memory.events, l_vertex_buffer);

        l_material.free_with_textures(l_graphics_allocator, l_buffer_memory);

        l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
        l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

        l_graphics_allocator.free_shader_layout(l_first_shader_layout);

        l_graphics_allocator.free_shader(l_first_shader);
        GraphicsPassAllocationComposition::free_attachmentimages_then_attachmenttextures_then_graphicspass(l_buffer_memory, l_graphics_allocator, l_graphics_pass);

        l_texture_pixels.free();
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
    Token<EWindow> l_window_token = WindowAllocator::allocate(l_window_size.x, l_window_size.y, slice_int8_build_rawstr(""));
    EWindow& l_window = WindowAllocator::get_window(l_window_token);

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
    ShaderCompiled l_vertex_compilder = l_shader_compiler.compile_shader(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
    ShaderCompiled l_fragment_compilder = l_shader_compiler.compile_shader(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));

#ifdef RENDER_DOC_DEBUG
    rdoc_api->StartFrameCapture(l_gpu_context.buffer_memory.allocator.device.device, NULL);
#endif

    v3ui l_render_target_size = {4, 4, 1};

    Token<TextureGPU> l_render_target_texture = GraphicsAllocatorComposition::allocate_texturegpu_with_imagegpu(
        l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator,
        ImageFormat::build_color_2d(l_render_target_size, ImageUsageFlag::SHADER_COLOR_ATTACHMENT | ImageUsageFlag::SHADER_TEXTURE_PARAMETER | ImageUsageFlag::TRANSFER_WRITE));

    // ShowWindow(hwnd, SW_NORMAL);
    GPUPresent l_gpu_present =
        GPUPresent::allocate(l_gpu_context.instance, l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator, l_window.handle, v3ui{l_window.client_width, l_window.client_height, 1},
                             l_render_target_texture, l_vertex_compilder.get_compiled_binary(), l_fragment_compilder.get_compiled_binary());

    assert_true(l_gpu_present.device.surface_capacilities.currentExtent.width == l_window.client_width);
    assert_true(l_gpu_present.device.surface_capacilities.currentExtent.height == l_window.client_height);

    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;

    {

        Span<color> l_render_target_color = Span<color>::allocate(l_render_target_size.x * l_render_target_size.y);
        for (loop(i, 0, l_render_target_color.Capacity))
        {
            l_render_target_color.get(i) = color{UINT8_MAX, 0, 0};
        }
        Token<ImageGPU> l_render_target_image = l_gpu_context.graphics_allocator.heap.textures_gpu.get(l_render_target_texture).Image;
        BufferReadWrite::write_to_imagegpu(l_gpu_context.buffer_memory.allocator, l_gpu_context.buffer_memory.events, l_render_target_image,
                                           l_gpu_context.buffer_memory.allocator.gpu_images.get(l_render_target_image), l_render_target_color.slice.build_asint8());
        l_render_target_color.free();

        l_gpu_context.buffer_step_submit();
        GraphicsBinder l_binder = l_gpu_context.build_graphics_binder();
        l_gpu_present.graphics_step(l_binder);
        l_gpu_context.submit_graphics_binder_and_notity_end(l_binder);
        l_gpu_present.present(l_gpu_context.graphics_end_semaphore);
        l_gpu_context.wait_for_completion();
    }
    {
        Span<color> l_render_target_color = Span<color>::allocate(l_render_target_size.x * l_render_target_size.y);
        for (loop(i, 0, l_render_target_color.Capacity))
        {
            l_render_target_color.get(i) = color{0, 0, UINT8_MAX};
        }

        Token<ImageGPU> l_render_target_image = l_gpu_context.graphics_allocator.heap.textures_gpu.get(l_render_target_texture).Image;
        BufferReadWrite::write_to_imagegpu(l_gpu_context.buffer_memory.allocator, l_gpu_context.buffer_memory.events, l_render_target_image,
                                           l_gpu_context.buffer_memory.allocator.gpu_images.get(l_render_target_image), l_render_target_color.slice.build_asint8());

        l_render_target_color.free();

        l_gpu_context.buffer_step_submit();
        GraphicsBinder l_binder = l_gpu_context.build_graphics_binder();
        l_gpu_present.graphics_step(l_binder);
        l_gpu_context.submit_graphics_binder_and_notity_end(l_binder);
        l_gpu_present.present(l_gpu_context.graphics_end_semaphore);
        l_gpu_context.wait_for_completion();
    }

    // resize
    window_native_simulate_resize_appevent(l_window.handle, 200, 200);
    l_window.consume_resize_event();

    v3ui l_size_before = v3ui{l_gpu_present.device.surface_capacilities.currentExtent.width, l_gpu_present.device.surface_capacilities.currentExtent.height, 1};
    l_gpu_present.resize(v3ui{l_window.client_width, l_window.client_height, 1}, l_gpu_context.buffer_memory, l_gpu_context.graphics_allocator);
    v3ui l_size_after = v3ui{l_gpu_present.device.surface_capacilities.currentExtent.width, l_gpu_present.device.surface_capacilities.currentExtent.height, 1};

    assert_true(l_size_after.x == l_window.client_width);
    assert_true(l_size_after.y == l_window.client_height);
    assert_true(l_size_before != l_size_after);

    {
        l_gpu_context.buffer_step_submit();
        GraphicsBinder l_binder = l_gpu_context.build_graphics_binder();
        l_gpu_present.graphics_step(l_binder);
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

    WindowAllocator::free(l_window_token);
    l_gpu_context.free();
};

inline void gpu_material_parameter_set()
{
    SliceN<GPUExtension, 1> l_gpu_extension_arr{GPUExtension::WINDOW_PRESENT};
    GPUContext l_gpu_context = GPUContext::allocate(slice_from_slicen(&l_gpu_extension_arr));
    GraphicsAllocator2& l_graphics_allocator = l_gpu_context.graphics_allocator;
    BufferMemory& l_buffer_memory = l_gpu_context.buffer_memory;

    SliceN<ShaderLayoutParameterType, 2> l_material_layout_parameter_types_arr = {ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT, ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX};
    Span<ShaderLayoutParameterType> l_material_layout_parameter_types = Span<ShaderLayoutParameterType>::allocate_slice(slice_from_slicen(&l_material_layout_parameter_types_arr));
    Span<ShaderLayout::VertexInputParameter> l_vertex_parameters = Span<ShaderLayout::VertexInputParameter>::allocate(0);
    Token<ShaderLayout> l_material_layout = l_graphics_allocator.allocate_shader_layout(l_material_layout_parameter_types, l_vertex_parameters, 0);

    Material l_material = Material::allocate_empty(l_graphics_allocator, 0);
    l_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shader_layouts.get(l_material_layout), (uimax)100);
    l_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_memory.allocator, l_graphics_allocator.heap.shader_layouts.get(l_material_layout), (uimax)300);

    {
        uimax& l_param_0 = l_material.get_buffer_host_parameter_memory_typed<uimax>(l_graphics_allocator, l_buffer_memory.allocator, (uimax)0);
        assert_true(l_param_0 == 100);
        l_param_0 = 200;
    }
    {
        uimax& l_param_0 = l_material.get_buffer_host_parameter_memory_typed<uimax>(l_graphics_allocator, l_buffer_memory.allocator, (uimax)0);
        assert_true(l_param_0 == 200);
        uimax& l_param_1 = l_material.get_buffer_host_parameter_memory_typed<uimax>(l_graphics_allocator, l_buffer_memory.allocator, (uimax)1);
        assert_true(l_param_1 == 300);
        l_param_1 = 400;
    }
    {
        uimax& l_param_1 = l_material.get_buffer_host_parameter_memory_typed<uimax>(l_graphics_allocator, l_buffer_memory.allocator, (uimax)1);
        assert_true(l_param_1 == 400);
    }

    l_material.free(l_graphics_allocator, l_buffer_memory);
    l_graphics_allocator.free_shader_layout(l_material_layout);

    l_gpu_context.free();
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
    gpu_material_parameter_set();

    memleak_ckeck();
};

#include "Common2/common2_external_implementation.hpp"
#include "GPU/gpu_external_implementation.hpp"