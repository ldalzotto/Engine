
#include "GPU/gpu.hpp"

namespace v2
{
	inline void gpu_memory_allocation()
	{
		int8* l_extensions[1];
		l_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
		GPUInstance l_gpu_instance = GPUInstance::allocate(Slice<int8*>::build_memory_elementnb(l_extensions, 1));
		TransferDevice l_transfer_device = TransferDevice::allocate(l_gpu_instance);
		{
			v2::BufferHost l_host_buffer = v2::BufferHost::allocate(l_transfer_device, 10, sizeof(uimax), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			uimax l_value = 20;
			l_host_buffer.push(Slice<uimax>::build_asint8_memory_singleelement(&l_value));
			l_host_buffer.free(l_transfer_device);
		}

		{
			v2::BufferGPU l_gpu_buffer = v2::BufferGPU::allocate(l_transfer_device, 10, sizeof(uimax), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
			l_gpu_buffer.free(l_transfer_device);
		}

		l_transfer_device.free();
		l_gpu_instance.free();
	};

}


//TODO
int main()
{
	v2::gpu_memory_allocation();
};