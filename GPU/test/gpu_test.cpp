
#include "GPU/gpu.hpp"

namespace v2
{
	inline void gpu_memory_allocation()
	{
		int8* l_extensions[1];
		l_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb(l_extensions, 1));
		BufferAllocator l_buffer_allocator = BufferAllocator::allocate_default(l_gpu_instance);

		{
			uimax l_value = 20;
			Token(BufferHost) l_buffer_host =
				l_buffer_allocator.allocate_bufferhost(Slice<uimax>::build_asint8_memory_singleelement(&l_value), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

			assert_true((*(uimax*)l_buffer_allocator.host_buffers.get(l_buffer_host).memory.memory.Begin) == l_value);

			l_buffer_allocator.free_bufferhost(l_buffer_host);
		}

		{
			Token(BufferGPU) l_buffer_gpu = l_buffer_allocator.allocate_buffergpu(sizeof(uimax), (VkFlags)BufferUsageFlag::GPU_WRITE | (VkFlags)BufferUsageFlag::GPU_READ | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

			uimax l_value = 20;
			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, Slice<uimax>::build_asint8_memory_singleelement(&l_value));
			l_buffer_allocator.step();

			//This is just for test purpose;
			l_buffer_allocator.device.command_buffer.submit();
			l_buffer_allocator.device.command_buffer.wait_for_completion();

			Token(BufferHost) l_read_buffer = l_buffer_allocator.read_from_buffergpu(l_buffer_gpu);

			l_buffer_allocator.step();
			l_buffer_allocator.device.command_buffer.submit();
			l_buffer_allocator.device.command_buffer.wait_for_completion();

			uimax* l_ptr = (uimax*)l_buffer_allocator.host_buffers.get(l_read_buffer).memory.memory.Begin;

			assert_true((*l_ptr) == l_value);

			l_buffer_allocator.free_bufferhost(l_read_buffer);
			l_buffer_allocator.free_buffergpu(l_buffer_gpu);
		}

		// creation and deletion the same frame
		{
			uimax l_value = 20;
			Token(BufferGPU) l_buffer_gpu =
				l_buffer_allocator.allocate_buffergpu(sizeof(uimax), (VkFlags)BufferUsageFlag::GPU_WRITE | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			l_buffer_allocator.write_to_buffergpu(l_buffer_gpu, Slice<uimax>::build_asint8_memory_singleelement(&l_value));
			l_buffer_allocator.free_buffergpu(l_buffer_gpu);

			l_buffer_allocator.step();
			l_buffer_allocator.device.command_buffer.submit();
			l_buffer_allocator.device.command_buffer.wait_for_completion();

		}

		l_buffer_allocator.free();
		l_gpu_instance.free();
	};

}


//TODO
int main()
{
	v2::gpu_memory_allocation();
};