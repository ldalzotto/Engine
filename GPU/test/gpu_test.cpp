
#include "GPU/gpu.hpp"

//TODO
int main()
{
	GPU l_gpu = GPU::allocate(Slice<int8*>{});

	GPUMemoryToken l_token;
	l_gpu.gc.heap.allocate_gpu_write_element(l_gpu.gc.device, 200, 200, &l_token);
	l_gpu.gc.heap.release_gpu_write_element(l_token);

	l_gpu.free();
};