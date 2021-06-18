#pragma once

enum class GPUExtension
{
    WINDOW_PRESENT = 0
};

#define GPU_DECLARE_TOKEN(p_name)                                                                                                                                                                      \
    struct _##p_name                                                                                                                                                                                   \
    {                                                                                                                                                                                                  \
    };                                                                                                                                                                                                 \
    using p_name = Token<_##p_name>

namespace gpu
{

using LayerConstString = SliceN<int8, 50>;

void layer_push_debug_layers(VectorSlice<LayerConstString>& p_layers);

GPU_DECLARE_TOKEN(Device);
GPU_DECLARE_TOKEN(Queue);
GPU_DECLARE_TOKEN(Instance);
GPU_DECLARE_TOKEN(Surface);
GPU_DECLARE_TOKEN(Debugger);
GPU_DECLARE_TOKEN(PhysicalDevice);

struct ApplicationInfo
{
    Slice<LayerConstString> enabled_layer_names;
    Slice<GPUExtension> enabled_extensions;
};

Instance create_instance(const ApplicationInfo& p_application_info);
Debugger initialize_debug_callback(Instance p_instance);

using MemoryTypeFlag_t = int8;
enum class MemoryTypeFlag : MemoryTypeFlag_t
{
    UNkNOWN = 0,
    DEVICE_LOCAL = 1,
    HOST_VISIBLE = 2,
	HOST_COHERENT = 4,
    HOST_CACHED = 8
};

struct QueueFamily
{
    uint32 family;

    inline static QueueFamily build_default()
    {
        QueueFamily l_return;
        l_return.family = -1;
        return l_return;
    };
};

struct HeapIndex
{
    uint32 index;
};

struct MemoryType
{
    MemoryTypeFlag type;
    HeapIndex heap_index;
};

struct MemoryHeap
{
    uimax size;
    MemoryTypeFlag type;
};

struct PhysicalDeviceMemoryProperties
{
    static constexpr int8 stack_size = 32;

    int8 memory_types_size;
    SliceN<MemoryType, stack_size> memory_types;
    int8 memory_heaps_size;
    SliceN<MemoryHeap, stack_size> memory_heaps;

    inline static PhysicalDeviceMemoryProperties build_default()
    {
        PhysicalDeviceMemoryProperties l_return;
        l_return.memory_types_size = 0;
        l_return.memory_types = SliceN<MemoryType, stack_size>{};
        l_return.memory_heaps_size = 0;
        l_return.memory_heaps = SliceN<MemoryHeap, stack_size>{};
        return l_return;
    };
};

struct physical_device_pick_Return
{
    gpu::PhysicalDevice physical_device;
    QueueFamily transfer_queue_family;
    QueueFamily graphics_queue_family;
    PhysicalDeviceMemoryProperties physical_memory_properties;

    inline static physical_device_pick_Return build_default(){
        physical_device_pick_Return l_return;
        l_return.physical_device = token_build_default<_PhysicalDevice>();
        l_return.transfer_queue_family = QueueFamily::build_default();
        l_return.graphics_queue_family = QueueFamily::build_default();
        l_return.physical_memory_properties = PhysicalDeviceMemoryProperties::build_default();
        return l_return;
    };
};

gpu::physical_device_pick_Return physical_device_pick(Instance p_instance);

} // namespace gpu

#undef GPU_DECLARE_TOKEN