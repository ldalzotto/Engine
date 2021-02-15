
#include "GPU/gpu.hpp"

// #define RENDER_DOC_DEBUG

#ifdef RENDER_DOC_DEBUG

#include "TestCommon/renderdoc_app.h"

RENDERDOC_API_1_1_0* rdoc_api = NULL;

#endif

namespace v2
{
	inline void gpu_buffer_allocation()
	{
		GPUContext l_gpu_context = GPUContext::allocate();

		BufferAllocator& l_buffer_allocator = l_gpu_context.buffer_allocator;

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		const uimax l_tested_uimax_array[3] = { 10, 20, 30 };
		Slice<uimax> l_tested_uimax_slice = Slice<uimax>::build_memory_elementnb((uimax*)l_tested_uimax_array, 3);

		// allocating and releasing a BufferHost
		{
			uimax l_value = 20;
			Token(BufferHost) l_buffer_host_token = l_buffer_allocator.allocate_bufferhost(l_tested_uimax_slice.build_asint8(), BufferUsageFlag::TRANSFER_READ);
			assert_true(l_buffer_allocator.get_bufferhost(l_buffer_host_token).get_mapped_memory().compare(l_tested_uimax_slice.build_asint8()));
			// We can write manually
			l_buffer_allocator.get_bufferhost(l_buffer_host_token).get_mapped_memory().get(1) = 25;
			l_buffer_allocator.free_bufferhost(l_buffer_host_token);
		}

		// allocating and releasing a BufferGPU
		{
			Token(BufferGPU) l_buffer_gpu = l_buffer_allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size,
					(BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE | (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ));

			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, l_tested_uimax_slice.build_asint8());

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 0);

			Token(BufferHost) l_read_buffer = l_buffer_allocator.read_from_buffergpu(l_buffer_gpu, l_buffer_allocator.get_buffergpu(l_buffer_gpu));

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 0);

			assert_true(l_buffer_allocator.get_bufferhost(l_read_buffer).get_mapped_memory().compare(l_tested_uimax_slice.build_asint8()));

			l_buffer_allocator.free_bufferhost(l_read_buffer);
			l_buffer_allocator.free_buffergpu(l_buffer_gpu);
		}

		// creation of a BufferGPU deletion the same step
		// nothing should happen
		{
			assert_true(l_buffer_allocator.buffer_host_to_gpu_copy_events.Size == 0);

			Token(BufferGPU) l_buffer_gpu = l_buffer_allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size, BufferUsageFlag::TRANSFER_WRITE);
			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, l_tested_uimax_slice.build_asint8());

			// it creates an gpu write event that will be consumed the next step
			assert_true(l_buffer_allocator.buffer_host_to_gpu_copy_events.Size == 1);

			l_buffer_allocator.free_buffergpu(l_buffer_gpu);

			assert_true(l_buffer_allocator.buffer_host_to_gpu_copy_events.Size == 0);

			l_buffer_allocator.step();
			l_buffer_allocator.device.command_buffer.force_sync_execution();
		}


#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		l_gpu_context.free();
	};

	inline void gpu_image_allocation()
	{
		GPUContext l_gpu_context = GPUContext::allocate();
		BufferAllocator& l_buffer_allocator = l_gpu_context.buffer_allocator;

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		const uimax l_pixels_count = 16 * 16;
		color l_pixels[l_pixels_count];
		Slice<color> l_pixels_slize = Slice<color>::build_memory_elementnb(l_pixels, l_pixels_count);
		for (loop(i, 1, l_pixels_slize.Size))
		{
			l_pixels_slize.get(i) = color{ (uint8)i, (uint8)i, (uint8)i, (uint8)i };
		}

		ImageFormat l_imageformat;
		l_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		l_imageformat.arrayLayers = 1;
		l_imageformat.format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
		l_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		l_imageformat.mipLevels = 1;
		l_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		l_imageformat.extent = v3ui{ 16, 16, 1 };

		// allocating and releasing a ImageHost
		{
			l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_READ;
			Token(ImageHost) l_image_host_token = l_buffer_allocator.allocate_imagehost(l_pixels_slize.build_asint8(), l_imageformat);
			ImageHost& l_image_host = l_buffer_allocator.host_images.get(l_image_host_token);

			assert_true(l_image_host.get_mapped_memory().compare(l_pixels_slize.build_asint8()));

			l_buffer_allocator.free_imagehost(l_image_host_token);
		}

		// allocating and releasing a ImageGPU
		{
			l_imageformat.imageUsage = (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::TRANSFER_WRITE);
			Token(ImageGPU) l_image_gpu = l_buffer_allocator.allocate_imagegpu(l_imageformat);

			l_buffer_allocator.write_to_imagegpu(l_image_gpu, l_buffer_allocator.get_imagegpu(l_image_gpu), l_pixels_slize.build_asint8());

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 0);

			Token(ImageHost) l_read_buffer = l_buffer_allocator.read_from_imagegpu(l_image_gpu, l_buffer_allocator.get_imagegpu(l_image_gpu));

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 0);

			assert_true(l_buffer_allocator.get_imagehost(l_read_buffer).get_mapped_memory().compare(l_pixels_slize.build_asint8()));

			l_buffer_allocator.free_imagehost(l_read_buffer);

			l_buffer_allocator.free_imagegpu(l_image_gpu);
		}

		// creation of a BufferGPU deletion the same step
		// nothing should happen
		{
			assert_true(l_buffer_allocator.buffer_host_to_image_gpu_cpy_event.Size == 0);

			l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_WRITE;
			Token(ImageGPU) l_image_gpu = l_buffer_allocator.allocate_imagegpu(l_imageformat);
			l_buffer_allocator.write_to_imagegpu(l_image_gpu, l_buffer_allocator.get_imagegpu(l_image_gpu), l_pixels_slize.build_asint8());

			// it creates an gpu write event that will be consumed the next 
			assert_true(l_buffer_allocator.buffer_host_to_image_gpu_cpy_event.Size == 1);

			l_buffer_allocator.free_imagegpu(l_image_gpu);

			assert_true(l_buffer_allocator.buffer_host_to_image_gpu_cpy_event.Size == 0);

			l_buffer_allocator.step();
			l_buffer_allocator.device.command_buffer.force_sync_execution();
		}

#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		l_gpu_context.free();
	};


	/*
		Creates a GraphicsPass that only clear input attachments.
	 	We check that the attachment has well been cleared with the input color.
	*/
	inline void gpu_renderpass_clear()
	{
		GPUContext l_gpu_context = GPUContext::allocate();
		BufferAllocator& l_buffer_allocator = l_gpu_context.buffer_allocator;
		GraphicsAllocator& l_graphics_allocator = l_gpu_context.graphics_allocator;

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif


		RenderPassAttachment l_attachments[2] =
				{
						RenderPassAttachment{ AttachmentType::COLOR,
											  ImageFormat::build_color_2d(v3ui{ 32, 32, 1 }, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT)) },
						RenderPassAttachment{ AttachmentType::DEPTH,
											  ImageFormat::build_depth_2d(v3ui{ 32, 32, 1 }, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT) }
				};
		Token(GraphicsPass) l_graphics_pass = l_graphics_allocator.allocate_graphicspass<2>(l_buffer_allocator, l_attachments);

		color l_clear_color = color{ 0, uint8_max, 51, uint8_max };

		{
			l_gpu_context.buffer_step_and_submit();

			v4f l_clear_values[2];
			l_clear_values[0] = v4f{ 0.0f, (float)l_clear_color.y / (float)uint8_max, (float)l_clear_color.z / (float)uint8_max, 1.0f };
			l_clear_values[1] = v4f{ 0.0f, 0.0f, 0.0f, 0.0f };

			GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();
			l_graphics_binder.begin_render_pass(l_graphics_allocator.get_graphics_pass(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));
			l_graphics_binder.end_render_pass();

			l_gpu_context.submit_graphics_binder(l_graphics_binder);
			l_gpu_context.wait_for_completion();
		}

		Token(BufferHost) l_color_attachment_value = l_graphics_allocator.read_graphics_pass_attachment_to_bufferhost(l_buffer_allocator,
				l_graphics_allocator.get_graphics_pass(l_graphics_pass), 0);

		l_buffer_allocator.step();
		l_buffer_allocator.device.command_buffer.force_sync_execution();


		Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_allocator.get_bufferhost(l_color_attachment_value).get_mapped_memory());

		for (loop(i, 0, l_color_attachment_value_pixels.Size))
		{
			assert_true(l_color_attachment_value_pixels.get(i).sRGB_to_linear() == l_clear_color);
		}

		l_buffer_allocator.free_bufferhost(l_color_attachment_value);

#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		l_graphics_allocator.free_graphicspass(l_buffer_allocator, l_graphics_pass);

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
		GPUContext l_gpu_context = GPUContext::allocate();
		BufferAllocator& l_buffer_allocator = l_gpu_context.buffer_allocator;
		GraphicsAllocator& l_graphics_allocator = l_gpu_context.graphics_allocator;

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		{
			RenderPassAttachment l_attachments[2] =
					{
							RenderPassAttachment{ AttachmentType::COLOR,
												  ImageFormat::build_color_2d(v3ui{ 4, 4, 1 }, (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR_ATTACHMENT)) },
							RenderPassAttachment{ AttachmentType::DEPTH, ImageFormat::build_depth_2d(v3ui{ 4, 4, 1 }, ImageUsageFlag::SHADER_DEPTH_ATTACHMENT) }
					};
			Token(GraphicsPass) l_graphics_pass = l_graphics_allocator.allocate_graphicspass<2>(l_buffer_allocator, l_attachments);


			struct vertex_position
			{
				v3f position;
			};

			vertex_position l_vertices[6] = {};
			l_vertices[0] = vertex_position{ v3f{ -1.0f, 1.0f, 0.0f }};
			l_vertices[1] = vertex_position{ v3f{ 1.0f, -1.0f, 0.0f }};
			l_vertices[2] = vertex_position{ v3f{ -1.0f, -1.0f, 0.0f }};
			l_vertices[3] = vertex_position{ v3f{ 0.0f, 0.5f, 0.0f }};
			l_vertices[4] = vertex_position{ v3f{ 0.5f, 0.0f, 0.0f }};
			l_vertices[5] = vertex_position{ v3f{ 0.0f, 0.0f, 0.0f }};
			Slice<vertex_position> l_vertices_slice = Slice<vertex_position>::build_memory_elementnb(l_vertices, 6);

			Token(BufferGPU) l_vertex_buffer = l_buffer_allocator.allocate_buffergpu(l_vertices_slice.build_asint8().Size, (BufferUsageFlag)((BufferUsageFlags)BufferUsageFlag::TRANSFER_WRITE |
																																			 (BufferUsageFlags)BufferUsageFlag::TRANSFER_READ | (BufferUsageFlags)BufferUsageFlag::VERTEX));
			l_buffer_allocator.write_to_buffergpu(l_vertex_buffer, l_vertices_slice.build_asint8());

			// Token(BufferHost) l_vertex_buffer =  l_buffer_allocator.allocate_bufferhost(l_vertices_slice.build_asint8(), BufferUsageFlag::VERTEX);

			Token(Shader) l_first_shader;
			Token(ShaderLayout) l_first_shader_layout;
			Token(ShaderModule) l_vertex_first_shader_module, l_fragment_first_shader_module;
			{
				Span<ShaderLayout::VertexInputParameter> l_shader_vertex_input_primitives = Span<ShaderLayout::VertexInputParameter>::allocate(1);
				l_shader_vertex_input_primitives.get(0) = ShaderLayout::VertexInputParameter{ PrimitiveSerializedTypes::Type::FLOAT32_3, offsetof(vertex_position, position) };

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

				ShaderCompiled l_vertex_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::VERTEX, slice_int8_build_rawstr(p_vertex_litteral));
				ShaderCompiled l_fragment_shader_compiled = ShaderCompiled::compile(ShaderModuleStage::FRAGMENT, slice_int8_build_rawstr(p_fragment_litteral));


				l_vertex_first_shader_module = l_graphics_allocator.allocate_shader_module(l_vertex_shader_compiled.get_compiled_binary());
				l_fragment_first_shader_module = l_graphics_allocator.allocate_shader_module(l_fragment_shader_compiled.get_compiled_binary());

				l_vertex_shader_compiled.free();
				l_fragment_shader_compiled.free();

				ShaderAllocateInfo l_shader_allocate_info{
						l_graphics_allocator.get_graphics_pass(l_graphics_pass),
						ShaderConfiguration{ 1, ShaderConfiguration::CompareOp::GreaterOrEqual },
						l_graphics_allocator.get_shader_layout(l_first_shader_layout),
						l_graphics_allocator.get_shader_module(l_vertex_first_shader_module),
						l_graphics_allocator.get_shader_module(l_fragment_first_shader_module)
				};

				l_first_shader = l_graphics_allocator.allocate_shader(l_shader_allocate_info);
			}


			Token(ShaderLayout) l_global_shader_layout;
			{
				Span<ShaderLayoutParameterType> l_first_shader_layout_parameters = Span<ShaderLayoutParameterType>::allocate(1);
				l_first_shader_layout_parameters.get(0) = ShaderLayoutParameterType::UNIFORM_BUFFER_VERTEX_FRAGMENT;
				Span<ShaderLayout::VertexInputParameter> l_vertex_parameters = Span<ShaderLayout::VertexInputParameter>::build(NULL, 0);
				l_global_shader_layout =l_graphics_allocator.allocate_shader_layout(l_first_shader_layout_parameters, l_vertex_parameters, 0);
			}


			color_f l_global_color = color_f{ 0.0f, 0.3f, 0.0f, 0.0f };
			color_f l_mat_1_base_color = color_f{ 1.0f, 0.0f, 0.0f, 0.5f };
			color_f l_mat_1_added_color = color_f{ 0.0f, 0.0f, 1.0f, 0.5f };
			color l_multiplied_color_texture_color = color{ 100, 100, 100, 255 }; //sRGB


			Material l_global_material = Material::allocate_empty(l_graphics_allocator, 0);
			l_global_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_allocator, l_graphics_allocator.get_shader(l_first_shader).layout, l_global_color);

			Material l_first_material = Material::allocate_empty(l_graphics_allocator, 1);
			l_first_material.add_and_allocate_buffer_gpu_parameter_typed(l_graphics_allocator, l_buffer_allocator, l_graphics_allocator.get_shader(l_first_shader).layout, l_mat_1_base_color);
			l_first_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_allocator, l_graphics_allocator.get_shader(l_first_shader).layout, l_mat_1_added_color);

			{

				Span<color> l_colors = Span<color>::allocate(l_attachments[0].image_format.extent.x * l_attachments[0].image_format.extent.y);
				for (loop(i, 0, l_colors.Capacity))
				{
					l_colors.get(i) = l_multiplied_color_texture_color;
				}
				l_first_material.add_and_allocate_texture_gpu_parameter(l_graphics_allocator, l_buffer_allocator, l_graphics_allocator.get_shader(l_first_shader).layout, l_attachments[0].image_format, l_colors.slice.build_asint8());
				l_colors.free();
			}

			color_f l_mat_2_base_color = color_f{ 0.0f, 0.0f, 0.0f };
			color_f l_mat_2_added_color = color_f{ 0.6f, 0.0f, 0.0f };

			Material l_second_material = Material::allocate_empty(l_graphics_allocator, 1);
			l_second_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_allocator, l_graphics_allocator.get_shader(l_first_shader).layout, l_mat_2_base_color);
			l_second_material.add_and_allocate_buffer_host_parameter_typed(l_graphics_allocator, l_buffer_allocator, l_graphics_allocator.get_shader(l_first_shader).layout, l_mat_2_added_color);

			color l_clear_color = color{ 0, uint8_max, 51, uint8_max };

			{
				v4f l_clear_values[2];
				l_clear_values[0] = v4f{ 0.0f, 1.0f, (float)l_clear_color.z / (float)uint8_max, 1.0f };
				l_clear_values[1] = v4f{ 0.0f, 0.0f, 0.0f, 0.0f };

				l_gpu_context.buffer_step_and_submit();

				GraphicsBinder l_graphics_binder = l_gpu_context.creates_graphics_binder();

				l_graphics_binder.bind_shader_layout(l_graphics_allocator.get_shader_layout(l_global_shader_layout));

				// l_graphics_binder.bind_shader(l_graphics_allocator.get_shader(l_global_shader));

				l_graphics_binder.bind_material(l_global_material);

				l_graphics_binder.begin_render_pass(l_graphics_allocator.get_graphics_pass(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));

				l_graphics_binder.bind_shader(l_graphics_allocator.get_shader(l_first_shader));

				l_graphics_binder.bind_material(l_first_material);
				l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_allocator.get_buffergpu(l_vertex_buffer));
				l_graphics_binder.draw(3);
				l_graphics_binder.pop_material_bind(l_first_material);

				l_graphics_binder.bind_material(l_second_material);
				l_graphics_binder.bind_vertex_buffer_gpu(l_buffer_allocator.get_buffergpu(l_vertex_buffer));
				l_graphics_binder.draw_offsetted(3, 3);
				l_graphics_binder.pop_material_bind(l_second_material);

				l_graphics_binder.end_render_pass();

				l_graphics_binder.pop_material_bind(l_global_material);

				l_gpu_context.submit_graphics_binder(l_graphics_binder);
			}

			{
				l_gpu_context.wait_for_completion();
			}


			Token(BufferHost) l_color_attachment_value = l_graphics_allocator.read_graphics_pass_attachment_to_bufferhost(l_buffer_allocator, l_graphics_allocator.get_graphics_pass(l_graphics_pass), 0);
			{
				l_buffer_allocator.step();
				l_buffer_allocator.device.command_buffer.submit();
				l_buffer_allocator.device.command_buffer.wait_for_completion();
			}
			Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_allocator.get_bufferhost(l_color_attachment_value).get_mapped_memory());

			color l_first_draw_awaited_color = ((l_global_color + l_mat_1_base_color + l_mat_1_added_color) * (l_multiplied_color_texture_color.to_color_f().sRGB_to_linear())).linear_to_sRGB().to_uint8_color();
			color l_second_draw_awaited_color = ((l_global_color + l_mat_2_base_color + l_mat_2_added_color) * (l_multiplied_color_texture_color.to_color_f().sRGB_to_linear())).linear_to_sRGB().to_uint8_color();

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


			l_buffer_allocator.free_bufferhost(l_color_attachment_value);

			l_buffer_allocator.free_buffergpu(l_vertex_buffer);

			l_global_material.free(l_graphics_allocator, l_buffer_allocator);
			l_second_material.free(l_graphics_allocator, l_buffer_allocator);
			l_first_material.free(l_graphics_allocator, l_buffer_allocator);


			l_graphics_allocator.free_shader_module(l_fragment_first_shader_module);
			l_graphics_allocator.free_shader_module(l_vertex_first_shader_module);

			l_graphics_allocator.free_shader_layout(l_global_shader_layout);
			l_graphics_allocator.free_shader_layout(l_first_shader_layout);

			l_graphics_allocator.free_shader(l_first_shader);
			l_graphics_allocator.free_graphicspass(l_buffer_allocator, l_graphics_pass);
		}

#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		l_gpu_context.free();
	};

}


int main()
{
#ifdef RENDER_DOC_DEBUG
	HMODULE mod = GetModuleHandleA("renderdoc.dll");

	pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
	int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_0, (void**)&rdoc_api);
	assert_true(ret == 1);
#endif

	v2::gpu_buffer_allocation();
	v2::gpu_image_allocation();
	v2::gpu_renderpass_clear();
	v2::gpu_draw();

	memleak_ckeck();
};

