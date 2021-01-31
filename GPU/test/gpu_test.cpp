
#include "GPU/gpu.hpp"

namespace v2
{
	inline void gpu_buffer_allocation()
	{
		const int8* l_extensions[1] = { VK_KHR_SURFACE_EXTENSION_NAME };
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb((int8**)l_extensions, 1));
		BufferAllocator l_buffer_allocator = BufferAllocator::allocate_default(l_gpu_instance);

		const uimax l_tested_uimax_array[3] = { 10,20,30 };
		Slice<uimax> l_tested_uimax_slice = Slice<uimax>::build_memory_elementnb((uimax*)l_tested_uimax_array, 3);

		// allocating and releasing a BufferHost
		{
			uimax l_value = 20;
			Token(BufferHost) l_buffer_host = l_buffer_allocator.allocate_bufferhost(l_tested_uimax_slice.build_asint8(), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			assert_true(l_buffer_allocator.get_bufferhost_mapped_memory(l_buffer_host).compare(l_tested_uimax_slice.build_asint8()));
			// We can write manually
			l_buffer_allocator.get_bufferhost_mapped_memory(l_buffer_host).get(1) = 25;
			l_buffer_allocator.free_bufferhost(l_buffer_host);
		}

		// allocating and releasing a BufferGPU
		{
			Token(BufferGPU) l_buffer_gpu = l_buffer_allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size,
				(VkFlags)BufferUsageFlag::WRITE | (VkFlags)BufferUsageFlag::READ | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, l_tested_uimax_slice.build_asint8());

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.force_command_buffer_execution_sync();

			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 0);

			Token(BufferHost) l_read_buffer = l_buffer_allocator.read_from_buffergpu(l_buffer_gpu);

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.force_command_buffer_execution_sync();

			assert_true(l_buffer_allocator.buffer_gpu_to_host_copy_events.Size == 0);

			assert_true(l_buffer_allocator.get_bufferhost_mapped_memory(l_read_buffer).compare(l_tested_uimax_slice.build_asint8()));

			l_buffer_allocator.free_bufferhost(l_read_buffer);
			l_buffer_allocator.free_buffergpu(l_buffer_gpu);
		}

		// creation of a BufferGPU deletion the same step
		// nothing should happen
		{
			assert_true(l_buffer_allocator.buffer_host_to_gpu_copy_events.Size == 0);

			Token(BufferGPU) l_buffer_gpu = l_buffer_allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size, (VkFlags)BufferUsageFlag::WRITE | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, l_tested_uimax_slice.build_asint8());

			// it creates an gpu write event that will be consumed the next step
			assert_true(l_buffer_allocator.buffer_host_to_gpu_copy_events.Size == 1);

			l_buffer_allocator.free_buffergpu(l_buffer_gpu);

			assert_true(l_buffer_allocator.buffer_host_to_gpu_copy_events.Size == 0);

			l_buffer_allocator.step();
			l_buffer_allocator.force_command_buffer_execution_sync();
		}

		l_buffer_allocator.free();
		l_gpu_instance.free();
	};

	inline void gpu_image_allocation()
	{
		const int8* l_extensions[1] = { VK_KHR_SURFACE_EXTENSION_NAME };
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb((int8**)l_extensions, 1));
		BufferAllocator l_buffer_allocator = BufferAllocator::allocate_default(l_gpu_instance);

		struct color
		{
			int8 r, g, b, a;
		};

		const uimax l_pixels_count = 16 * 16;
		color l_pixels[l_pixels_count];
		Slice<color> l_pixels_slize = Slice<color>::build_memory_elementnb(l_pixels, l_pixels_count);
		for (loop(i, 0, l_pixels_slize.Size))
		{
			l_pixels_slize.get(i) = color{ (int8)i,(int8)i,(int8)i,(int8)i };
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
			Token(ImageHost) l_image_host_token = l_buffer_allocator.allocate_imagehost(l_pixels_slize.build_asint8(), l_imageformat, (ImageUsageFlags)ImageUsageFlag::READ);
			ImageHost& l_image_host = l_buffer_allocator.host_images.get(l_image_host_token);

			assert_true(l_image_host.get_mapped_memory().compare(l_pixels_slize.build_asint8()));

			l_buffer_allocator.free_imagehost(l_image_host_token);
		}

		// allocating and releasing a ImageGPU
		{
			Token(ImageGPU) l_image_gpu = l_buffer_allocator.allocate_imagegpu(l_imageformat, (ImageUsageFlags)ImageUsageFlag::READ | (ImageUsageFlags)ImageUsageFlag::WRITE);

			l_buffer_allocator.write_to_imagegpu(l_image_gpu, l_pixels_slize.build_asint8());

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.force_command_buffer_execution_sync();

			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 0);

			Token(ImageHost) l_read_buffer = l_buffer_allocator.read_from_imagegpu(l_image_gpu);

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.force_command_buffer_execution_sync();

			assert_true(l_buffer_allocator.image_gpu_to_host_copy_events.Size == 0);

			assert_true(l_buffer_allocator.get_imagehost_mapped_memory(l_read_buffer).compare(l_pixels_slize.build_asint8()));

			l_buffer_allocator.free_imagehost(l_read_buffer);
			l_buffer_allocator.free_imagegpu(l_image_gpu);
		}

		// creation of a BufferGPU deletion the same step
		// nothing should happen
		{
			assert_true(l_buffer_allocator.image_host_to_gpu_copy_events.Size == 0);

			Token(ImageGPU) l_image_gpu = l_buffer_allocator.allocate_imagegpu(l_imageformat, (VkFlags)ImageUsageFlag::WRITE);
			l_buffer_allocator.write_to_imagegpu(l_image_gpu, l_pixels_slize.build_asint8());

			// it creates an gpu write event that will be consumed the next 
			assert_true(l_buffer_allocator.image_host_to_gpu_copy_events.Size == 1);

			l_buffer_allocator.free_imagegpu(l_image_gpu);

			assert_true(l_buffer_allocator.image_host_to_gpu_copy_events.Size == 0);

			l_buffer_allocator.step();
			l_buffer_allocator.force_command_buffer_execution_sync();
		}

		l_buffer_allocator.free();
		l_gpu_instance.free();
	};
}

int main()
{
	v2::gpu_buffer_allocation();
	v2::gpu_image_allocation();
};
