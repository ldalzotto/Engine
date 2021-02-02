
#include "GPU/gpu.hpp"


// #define RENDER_DOC_DEBUG

#ifdef RENDER_DOC_DEBUG
#include "./renderdoc_app.h"

RENDERDOC_API_1_1_0* rdoc_api = NULL;
#endif

namespace v2
{
	inline void gpu_buffer_allocation()
	{
		const int8* l_extensions[1] = { VK_KHR_SURFACE_EXTENSION_NAME };
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb((int8**)l_extensions, 1));
		BufferAllocator l_buffer_allocator = BufferAllocator::allocate_default(l_gpu_instance);

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		const uimax l_tested_uimax_array[3] = { 10,20,30 };
		Slice<uimax> l_tested_uimax_slice = Slice<uimax>::build_memory_elementnb((uimax*)l_tested_uimax_array, 3);

		// allocating and releasing a BufferHost
		{
			uimax l_value = 20;
			Token(BufferHost) l_buffer_host = l_buffer_allocator.allocate_bufferhost(l_tested_uimax_slice.build_asint8(), BufferUsageFlag::TRANSFER_READ);
			assert_true(l_buffer_allocator.get_bufferhost_mapped_memory(l_buffer_host).compare(l_tested_uimax_slice.build_asint8()));
			// We can write manually
			l_buffer_allocator.get_bufferhost_mapped_memory(l_buffer_host).get(1) = 25;
			l_buffer_allocator.free_bufferhost(l_buffer_host);
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

			Token(BufferHost) l_read_buffer = l_buffer_allocator.read_from_buffergpu(l_buffer_gpu);

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 0);

			assert_true(l_buffer_allocator.get_bufferhost_mapped_memory(l_read_buffer).compare(l_tested_uimax_slice.build_asint8()));

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

		l_buffer_allocator.free();
		l_gpu_instance.free();
	};

	inline void gpu_image_allocation()
	{
		const int8* l_extensions[1] = { VK_KHR_SURFACE_EXTENSION_NAME };
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb((int8**)l_extensions, 1));
		BufferAllocator l_buffer_allocator = BufferAllocator::allocate_default(l_gpu_instance);

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		struct color
		{
			uint8 r, g, b, a;
		};

		const uimax l_pixels_count = 16 * 16;
		color l_pixels[l_pixels_count];
		Slice<color> l_pixels_slize = Slice<color>::build_memory_elementnb(l_pixels, l_pixels_count);
		for (loop(i, 1, l_pixels_slize.Size))
		{
			l_pixels_slize.get(i) = color{ (uint8)i, (uint8)i , (uint8)i , (uint8)i };
		}

		ImageFormat l_imageformat;
		l_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		l_imageformat.arrayLayers = 1;
		l_imageformat.format = VkFormat::VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		l_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		l_imageformat.mipLevels = 1;
		l_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		l_imageformat.extent = v3ui{ 16,16,1 };

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

			l_buffer_allocator.write_to_imagegpu(l_image_gpu, l_pixels_slize.build_asint8());

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 0);

			Token(ImageHost) l_read_buffer = l_buffer_allocator.read_from_imagegpu(l_image_gpu);

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.device.command_buffer.force_sync_execution();

			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 0);


			//TODO -> re enable
			assert_true(l_buffer_allocator.get_imagehost_mapped_memory(l_read_buffer).compare(l_pixels_slize.build_asint8()));

			l_buffer_allocator.free_imagehost(l_read_buffer);

			l_buffer_allocator.free_imagegpu(l_image_gpu);
		}

		// creation of a BufferGPU deletion the same step
		// nothing should happen
		{
			assert_true(l_buffer_allocator.image_host_to_gpu_copy_events.Size == 0);

			l_imageformat.imageUsage = ImageUsageFlag::TRANSFER_WRITE;
			Token(ImageGPU) l_image_gpu = l_buffer_allocator.allocate_imagegpu(l_imageformat);
			l_buffer_allocator.write_to_imagegpu(l_image_gpu, l_pixels_slize.build_asint8());

			// it creates an gpu write event that will be consumed the next 
			assert_true(l_buffer_allocator.image_host_to_gpu_copy_events.Size == 1);

			l_buffer_allocator.free_imagegpu(l_image_gpu);

			assert_true(l_buffer_allocator.image_host_to_gpu_copy_events.Size == 0);

			l_buffer_allocator.step();
			l_buffer_allocator.device.command_buffer.force_sync_execution();
		}

#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		l_buffer_allocator.free();
		l_gpu_instance.free();
	};

	inline void gpu_abstraction()
	{



		const int8* l_extensions[1] = { VK_KHR_SURFACE_EXTENSION_NAME };
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb((int8**)l_extensions, 1));
		BufferAllocator l_buffer_allocator = BufferAllocator::allocate_default(l_gpu_instance);
		GraphicsAllocator l_graphics_allocator = GraphicsAllocator::allocate_default(l_gpu_instance);

#ifdef RENDER_DOC_DEBUG
		rdoc_api->StartFrameCapture(l_buffer_allocator.device.device, NULL);
#endif


		Semafore l_buffer_allocator_semaphore = Semafore::allocate(l_gpu_instance.logical_device);

		ImageFormat l_color_imageformat;
		l_color_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		l_color_imageformat.arrayLayers = 1;
		l_color_imageformat.format = VkFormat::VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		l_color_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		l_color_imageformat.mipLevels = 1;
		l_color_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		l_color_imageformat.extent = v3ui{ 16,16,1 };
		l_color_imageformat.imageUsage = (ImageUsageFlag)((ImageUsageFlags)ImageUsageFlag::TRANSFER_READ | (ImageUsageFlags)ImageUsageFlag::SHADER_COLOR);

		ImageFormat l_depth_imageformat;
		l_depth_imageformat.imageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
		l_depth_imageformat.arrayLayers = 1;
		l_depth_imageformat.format = VkFormat::VK_FORMAT_D16_UNORM;
		l_depth_imageformat.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		l_depth_imageformat.mipLevels = 1;
		l_depth_imageformat.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		l_depth_imageformat.extent = v3ui{ 16,16,1 };
		l_depth_imageformat.imageUsage = ImageUsageFlag::SHADER_DEPTH;

		// TextureGPU allocation


		AttachmentType l_attachment_types[2] = { AttachmentType::COLOR, AttachmentType::DEPTH };
		ImageFormat l_image_formats[2] = { l_color_imageformat, l_depth_imageformat };
		Token(GraphicsPass) l_graphics_pass = l_graphics_allocator.allocate_graphicspass<2>(l_buffer_allocator, l_attachment_types, l_image_formats);


		l_buffer_allocator.step();
		l_buffer_allocator.device.command_buffer.submit_and_notity(l_buffer_allocator_semaphore);
		l_graphics_allocator.graphicsDevice.command_buffer.begin();
		/*
		vkCmdBeginRenderPass(l_graphics_allocator.graphicsDevice.command_buffer.command_buffer, , );
				*/

		v4f l_clear_values[2];
		l_clear_values[0] = v4f{ 0.0f, 0.0f, 0.2f, 1.0f };
		l_clear_values[1] = v4f{ 0.0f,0.0f,0.0f,0.0f };

		l_graphics_allocator.cmd_beginRenderPass2(l_buffer_allocator, l_graphics_allocator.graphics_pass.get(l_graphics_pass), Slice<v4f>::build_memory_elementnb(l_clear_values, 2));
		l_graphics_allocator.cmd_endRenderPass(l_graphics_allocator.graphics_pass.get(l_graphics_pass));

		l_graphics_allocator.graphicsDevice.command_buffer.end();
		l_graphics_allocator.graphicsDevice.command_buffer.submit_after(l_buffer_allocator_semaphore, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_HOST_BIT);

		l_graphics_allocator.graphicsDevice.command_buffer.wait_for_completion();

		GraphicsPass& l_graphics_pass_value = l_graphics_allocator.graphics_pass.get(l_graphics_pass);
		Slice<Token(TextureGPU)> l_attachments = l_graphics_allocator.renderpass_attachment_textures.get_vector(l_graphics_pass_value.attachment_textures);
		Token(ImageHost) l_color_attachment_value = l_buffer_allocator.read_from_imagegpu(l_graphics_allocator.textures_gpu.get(l_attachments.get(0)).Image);

		l_buffer_allocator.step();
		l_buffer_allocator.device.command_buffer.force_sync_execution();


		struct color
		{
			uint8 r, g, b, a;
		};

		Slice<color> l_color_attachment_value_pixels = slice_cast<color>(l_buffer_allocator.get_imagehost_mapped_memory(l_color_attachment_value));

		for (loop(i, 0, l_color_attachment_value_pixels.Size))
		{
			assert_true(l_color_attachment_value_pixels.get(i).r == 0);
			assert_true(l_color_attachment_value_pixels.get(i).g == 0);
			assert_true(l_color_attachment_value_pixels.get(i).b == 0);
		}

		l_buffer_allocator.free_imagehost(l_color_attachment_value);

		// l_graphics_allocator.graphicsDevice.command_buffer.force_sync_execution();
// 		l_graphics_allocator.ste

#ifdef RENDER_DOC_DEBUG
		rdoc_api->EndFrameCapture(l_buffer_allocator.device.device, NULL);
#endif

		l_buffer_allocator_semaphore.free(l_gpu_instance.logical_device);

		l_graphics_allocator.free_graphicspass(l_buffer_allocator, l_graphics_pass);
		l_graphics_allocator.free();
		l_buffer_allocator.free();
		l_gpu_instance.free();
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
	v2::gpu_abstraction();

};
