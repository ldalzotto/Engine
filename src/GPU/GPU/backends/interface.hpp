#pragma once

enum class GPUExtension
{
    WINDOW_PRESENT = 0
};

using BufferUsageFlag_t = uint8;
enum class BufferUsageFlag : BufferUsageFlag_t
{
    UNDEFINED = 0,
    TRANSFER_READ = 1,
    TRANSFER_WRITE = 2,
    UNIFORM = 16,
    INDEX = 64,
    VERTEX = 128
};

declare_binary_operations(BufferUsageFlag);

using ImageUsageFlag_t = uint8;
enum class ImageUsageFlag : ImageUsageFlag_t
{
    UNDEFINED = 0,
    TRANSFER_READ = 1,
    TRANSFER_WRITE = 2,
    SHADER_TEXTURE_PARAMETER = 4,
    SHADER_COLOR_ATTACHMENT = 16,
    SHADER_DEPTH_ATTACHMENT = 32
};

declare_binary_operations(ImageUsageFlag);

using ImageType_t = uint8;
enum class ImageType : ImageType_t
{
    _1D = 0,
    _2D = 1,
    _3D = 2
};

using ImageAspectFlag_t = uint8;
enum class ImageAspectFlag : ImageAspectFlag_t
{
    UNDEFINED = 0,
    COLOR = 1,
    DEPTH = 2
};

using ImageFormatFlag_t = uint8;
enum class ImageFormatFlag : ImageFormatFlag_t
{
    UNDEFINED = 0,
    R8G8B8A8_SRGB = 43,
    D16_UNORM = 124
};

declare_binary_operations(ImageFormatFlag);

#define GPU_DECLARE_TOKEN(p_name)                                                                                                                                                                      \
    struct _##p_name                                                                                                                                                                                   \
    {                                                                                                                                                                                                  \
    };                                                                                                                                                                                                 \
    using p_name = Token<_##p_name>

namespace gpu
{

using LayerConstString = SliceN<int8, 50>;

void layer_push_debug_layers(VectorSlice<LayerConstString>& p_layers);

GPU_DECLARE_TOKEN(Queue);
GPU_DECLARE_TOKEN(Instance);
GPU_DECLARE_TOKEN(Surface);
GPU_DECLARE_TOKEN(Debugger);
GPU_DECLARE_TOKEN(PhysicalDevice);
GPU_DECLARE_TOKEN(LogicalDevice);
GPU_DECLARE_TOKEN(DeviceMemory);
GPU_DECLARE_TOKEN(Semaphore);
GPU_DECLARE_TOKEN(CommandBuffer);
GPU_DECLARE_TOKEN(CommandPool);
GPU_DECLARE_TOKEN(Buffer);
GPU_DECLARE_TOKEN(Image);

struct ApplicationInfo
{
    Slice<LayerConstString> enabled_layer_names;
    Slice<GPUExtension> enabled_extensions;
};

Instance create_instance(const ApplicationInfo& p_application_info);
void instance_destroy(Instance p_instance);
Debugger initialize_debug_callback(Instance p_instance);
void debugger_finalize(Debugger p_debugger, Instance p_instance);

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

    inline static physical_device_pick_Return build_default()
    {
        physical_device_pick_Return l_return;
        l_return.physical_device = token_build_default<_PhysicalDevice>();
        l_return.transfer_queue_family = QueueFamily::build_default();
        l_return.graphics_queue_family = QueueFamily::build_default();
        l_return.physical_memory_properties = PhysicalDeviceMemoryProperties::build_default();
        return l_return;
    };
};

gpu::physical_device_pick_Return physical_device_pick(Instance p_instance);

struct PhysicalDeviceMemoryIndex
{
    uint32 index;
};

struct MemoryRequirements
{
    uimax size;
    uimax alignment;
    MemoryTypeFlag memory_type;
};

PhysicalDeviceMemoryIndex physical_device_get_memorytype_index(const PhysicalDeviceMemoryProperties& p_memory_properties, const MemoryTypeFlag p_required_memory_type,
                                                               const MemoryTypeFlag p_memory_type);

gpu::LogicalDevice logical_device_create(PhysicalDevice p_physical_device, const Slice<LayerConstString>& p_validation_layers, const Slice<GPUExtension>& p_gpu_extensions, const QueueFamily& p_queue);
void logical_device_destroy(LogicalDevice p_logical_device);
Queue logical_device_get_queue(const LogicalDevice p_logical_device, const QueueFamily p_queue_family);

DeviceMemory allocate_memory(const LogicalDevice p_device, const uimax p_allocation_size, const PhysicalDeviceMemoryIndex p_memory_index);
void free_memory(const LogicalDevice p_device, const DeviceMemory p_device_memory);
int8* map_memory(const LogicalDevice p_device, const DeviceMemory p_device_memory, const uimax p_offset, const uimax p_size);

Semaphore semaphore_allocate(const LogicalDevice p_device);
void semaphore_destroy(const LogicalDevice p_device, const Semaphore p_semaphore);

void queue_wait_idle(Queue p_queue);

void command_buffer_begin(CommandBuffer p_command_buffer);
void command_buffer_end(CommandBuffer p_command_buffer);
void command_buffer_submit(CommandBuffer p_command_buffer, Queue p_queue);
void command_buffer_submit_and_notify(CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_notified_semaphore);
void command_buffer_submit_after(CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore);
void command_buffer_submit_after_and_notify(CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore, Semaphore p_notify_semaphore);

CommandPool command_pool_allocate(const LogicalDevice p_logical_device, const QueueFamily p_queue_family);
void command_pool_destroy(const LogicalDevice p_logical_device, CommandPool p_pool);
CommandBuffer command_pool_allocate_command_buffer(const LogicalDevice p_logical_device, CommandPool p_command_pool);

Buffer buffer_allocate(const LogicalDevice p_logical_device, const uimax p_size, const BufferUsageFlag p_usage_flag);
void buffer_destroy(const LogicalDevice p_logical_device, const Buffer p_buffer);
MemoryRequirements buffer_get_memory_requirements(const LogicalDevice p_logical_device, const Buffer p_buffer);
void buffer_bind_memory(const LogicalDevice p_logical_device, const Buffer p_buffer, const DeviceMemory p_memory, const uimax p_offset);
} // namespace gpu

#undef GPU_DECLARE_TOKEN