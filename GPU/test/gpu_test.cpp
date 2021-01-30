
#include "GPU/gpu.hpp"

namespace v2
{
	inline void gpu_memory_allocation()
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

			assert_true(l_buffer_allocator.gpu_to_host_copy_events.Size == 0);

			Token(BufferHost) l_read_buffer = l_buffer_allocator.read_from_buffergpu(l_buffer_gpu);

			// it creates an gpu read event that will be consumed the next step
			assert_true(l_buffer_allocator.gpu_to_host_copy_events.Size == 1);

			l_buffer_allocator.step();
			// We force command buffer submit just for the test
			l_buffer_allocator.force_command_buffer_execution_sync();

			assert_true(l_buffer_allocator.gpu_to_host_copy_events.Size == 0);

			assert_true(l_buffer_allocator.get_bufferhost_mapped_memory(l_read_buffer).compare(l_tested_uimax_slice.build_asint8()));

			l_buffer_allocator.free_bufferhost(l_read_buffer);
			l_buffer_allocator.free_buffergpu(l_buffer_gpu);
		}

		// creation of a BufferGPU deletion the same step
		// nothing should happen
		{
			assert_true(l_buffer_allocator.host_to_gpu_copy_events.Size == 0);

			Token(BufferGPU) l_buffer_gpu = l_buffer_allocator.allocate_buffergpu(l_tested_uimax_slice.build_asint8().Size, (VkFlags)BufferUsageFlag::WRITE | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, l_tested_uimax_slice.build_asint8());

			// it creates an gpu write event that will be consumed the next step
			assert_true(l_buffer_allocator.host_to_gpu_copy_events.Size == 1);

			l_buffer_allocator.free_buffergpu(l_buffer_gpu);

			assert_true(l_buffer_allocator.host_to_gpu_copy_events.Size == 0);

			l_buffer_allocator.step();
			l_buffer_allocator.force_command_buffer_execution_sync();
		}

		l_buffer_allocator.free();
		l_gpu_instance.free();
	};

}

int main()
{
	v2::gpu_memory_allocation();
};
