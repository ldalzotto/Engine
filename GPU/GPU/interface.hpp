#pragma once

using gc_t = VkDevice;
using gcqueue_t = VkQueue;
using gcmemory_t = VkDeviceMemory;
using gpuinstance_t = VkInstance;
using gpuinstance_debugger_t = VkDebugUtilsMessengerEXT;



struct HeapGPU
{
	v2::Heap heap;
	gcmemory_t memory;

	static HeapGPU allocate(const gc_t p_gc, const uint32 p_memory_type, const uimax p_memory_size);
	void free(const gc_t p_gc);

	int8 allocate_element(const gc_t p_gc, const uimax p_size, const uimax p_alignement_constraint, Token(SliceIndex)* out_token);
	void release_element(const Token(SliceIndex) p_token);
};

struct GPUMemoryToken
{
	uint8 pagedbuffer_index;
	Token(SliceIndex) chunk;
};

struct GraphicsCardPagedHeap
{
	v2::Vector<HeapGPU> page_buffers;
	uimax chunk_size;

	static GraphicsCardPagedHeap allocate(const uimax p_chunk_size);
	void free(const gc_t p_gc);

	int8 allocate_element(const uimax p_size, const uimax p_alignmenent_constaint, const uint32 p_memory_type, gc_t p_gc, GPUMemoryToken* out_chunk);
	void release_element(const GPUMemoryToken& p_token);
};

struct GraphicsCardHeap
{
	GraphicsCardPagedHeap gpu_write;
	GraphicsCardPagedHeap host_write;

	static GraphicsCardHeap allocate_default(const gc_t p_gc);
	void free(const gc_t p_gc);

	int8 allocate_gpu_write_element(const gc_t p_gc, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token);
	void release_gpu_write_element(const GPUMemoryToken& p_memory);
	int8 allocate_host_write_element(const gc_t p_gc, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token);
	void release_host_write_element(const GPUMemoryToken& p_memory);

	int8 allocate_element_always(const gc_t p_gc, const uint32 p_memory_type, const uimax p_size, const uimax p_alignement_constraint, GPUMemoryToken* out_token);
	void release_element_always(const uint32 p_memory_type, const GPUMemoryToken& p_memory);

};


struct GPUInstance
{
	gpuinstance_t instance;
#if RENDER_DEBUG
	gpuinstance_debugger_t debugger;
#endif

	static GPUInstance allocate(const Slice<int8*>& p_required_extensions);
	void free();
};

struct GraphicsCard
{
	struct PhysicalDevice
	{
		VkPhysicalDevice device;
		uint32 graphics_queue_family;
		uint32 present_queue_family;
	} physical_device;

	gc_t device;
	gcqueue_t graphics_queue;
	gcqueue_t present_queue;

	GraphicsCardHeap heap;

	static GraphicsCard allocate(const GPUInstance& p_instance);
	void free();
};

struct GPU
{
	GPUInstance instance;
	GraphicsCard gc;

	static GPU allocate(const Slice<int8*>& p_required_extensions);
	void free();
};