#pragma once

enum class GPUExtension
{
    WINDOW_PRESENT = 0
};

using BufferUsageFlag_t = uint32;
enum class BufferUsageFlag : BufferUsageFlag_t
{
    UNKNOWN = 0,
    TRANSFER_READ = 1,
    TRANSFER_WRITE = 2,
    UNIFORM = 16,
    INDEX = 64,
    VERTEX = 128
};

declare_binary_operations(BufferUsageFlag);

using ImageUsageFlag_t = uint32;
enum class ImageUsageFlag : ImageUsageFlag_t
{
    UNKNOWN = 0,
    TRANSFER_READ = 1,
    TRANSFER_WRITE = 2,
    SHADER_TEXTURE_PARAMETER = 4,
    SHADER_COLOR_ATTACHMENT = 16,
    SHADER_DEPTH_ATTACHMENT = 32
};

declare_binary_operations(ImageUsageFlag);

using ImageType_t = uint32;
enum class ImageType : ImageType_t
{
    _1D = 0,
    _2D = 1,
    _3D = 2
};

using ImageAspectFlag_t = uint32;
enum class ImageAspectFlag : ImageAspectFlag_t
{
    UNKNOWN = 0,
    COLOR = 1,
    DEPTH = 2
};

using ImageFormatFlag_t = uint32;
enum class ImageFormatFlag : ImageFormatFlag_t
{
    UNKNOWN = 0,
    R8G8B8_SRGB = 29,
    R8G8B8A8_SRGB = 43,
    D16_UNORM = 124
};

declare_binary_operations(ImageFormatFlag);

using ImageSampleCountFlag_t = uint32;
enum class ImageSampleCountFlag : ImageSampleCountFlag_t
{
    UNKNOWN = 0,
    _1 = 1
};

using ImageLayoutFlag_t = uint32;
enum class ImageLayoutFlag : ImageLayoutFlag_t
{
    UNKNOWN = 0,
    COLOR_ATTACHMENT = 2,
    DEPTH_STENCIL_ATTACHMENT = 3,
    SHADER_READ_ONLY = 5,
    TRANSFER_SRC = 6,
    TRANSFER_DST = 7
};

using GPUPipelineStageFlag_t = uint16;
enum class GPUPipelineStageFlag : GPUPipelineStageFlag_t
{
    UNKNOWN = 0,
    TOP_OF_PIPE = 1,
    FRAGMENT_SHADER = 128,
    COLOR_ATTACHMENT_OUTPUT = 1024,
    TRANSFER = 4096
};

using GPUAccessFlag_t = uint32;
enum class GPUAccessFlag : GPUAccessFlag_t
{
    UNKNOWN = 0,
    SHADER_READ = 32,
    COLOR_ATTACHMENT_WRITE = 256,
    TRANSFER_READ = 2048,
    TRANSFER_WRITE = 4096
};

/*
    mipLevels and arrayLayers are not supported yet. They should always be equals to 1.
*/
struct ImageFormat
{
    ImageAspectFlag imageAspect;
    ImageType imageType;
    ImageUsageFlag imageUsage;
    ImageFormatFlag format;
    v3ui extent;
    uint32 mipLevels;
    uint32 arrayLayers;
    ImageSampleCountFlag samples;

    inline static ImageFormat build_color_2d(const v3ui& p_extend, const ImageUsageFlag p_usage)
    {
        ImageFormat l_color_imageformat;
        l_color_imageformat.imageAspect = ImageAspectFlag::COLOR;
        l_color_imageformat.arrayLayers = 1;
        l_color_imageformat.format = ImageFormatFlag::R8G8B8A8_SRGB;
        l_color_imageformat.imageType = ImageType::_2D;
        l_color_imageformat.mipLevels = 1;
        l_color_imageformat.samples = ImageSampleCountFlag::_1;
        l_color_imageformat.extent = p_extend;
        l_color_imageformat.imageUsage = p_usage;
        return l_color_imageformat;
    };

    inline static ImageFormat build_depth_2d(const v3ui& p_extend, const ImageUsageFlag p_usage)
    {
        ImageFormat l_depth_imageformat;
        l_depth_imageformat.imageAspect = ImageAspectFlag::DEPTH;
        l_depth_imageformat.arrayLayers = 1;
        l_depth_imageformat.format = ImageFormatFlag::D16_UNORM;
        l_depth_imageformat.imageType = ImageType::_2D;
        l_depth_imageformat.mipLevels = 1;
        l_depth_imageformat.samples = ImageSampleCountFlag::_1;
        l_depth_imageformat.extent = p_extend;
        l_depth_imageformat.imageUsage = p_usage;
        return l_depth_imageformat;
    };

    inline int8 get_pixel_size()
    {
        switch (this->format)
        {
        case ImageFormatFlag::R8G8B8A8_SRGB:
            return 4;
        case ImageFormatFlag::D16_UNORM:
            return 2;
        case ImageFormatFlag::R8G8B8_SRGB:
            return 3;
        case ImageFormatFlag::UNKNOWN:
            abort();
        }
        return 0;
    };

    inline uint32 get_size()
    {
        return (this->extent.x * this->extent.y * this->extent.z) * this->get_pixel_size();
    };
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

GPU_DECLARE_TOKEN(Queue);
GPU_DECLARE_TOKEN(Instance);
GPU_DECLARE_TOKEN(Surface);
GPU_DECLARE_TOKEN(Debugger);
GPU_DECLARE_TOKEN(PhysicalDevice);
GPU_DECLARE_TOKEN(LogicalDevice);
GPU_DECLARE_TOKEN(DeviceMemory);
GPU_DECLARE_TOKEN(Semaphore);
GPU_DECLARE_TOKEN(CommandBuffer);
GPU_DECLARE_TOKEN(CommandBufferSubmit);
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

using MemoryTypeFlag_t = uint32;
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

void command_buffer_begin(LogicalDevice p_device, CommandBuffer p_command_buffer);
void command_buffer_end(LogicalDevice p_device, CommandBuffer p_command_buffer);
CommandBufferSubmit command_buffer_submit(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue);
CommandBufferSubmit command_buffer_submit_and_notify(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_notified_semaphore);
CommandBufferSubmit command_buffer_submit_after(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore, CommandBufferSubmit p_after_command_buffer);
CommandBufferSubmit command_buffer_submit_after_and_notify(LogicalDevice p_device, CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore, CommandBufferSubmit p_after_command_buffer, Semaphore p_notify_semaphore);
void command_copy_buffer(LogicalDevice p_device, CommandBuffer p_command_buffer, const Buffer p_source, const uimax p_source_size, Buffer p_target, const uimax p_target_size);
void command_copy_buffer_to_image(LogicalDevice p_device, CommandBuffer p_command_buffer, const Buffer p_source, const uimax p_source_size, const Image p_target, const ImageFormat& p_target_format);
void command_copy_image_to_buffer(LogicalDevice p_device, CommandBuffer p_command_buffer, const Image p_source, const ImageFormat& p_source_format, const Buffer p_target);
void command_copy_image(LogicalDevice p_device, CommandBuffer p_command_buffer, const Image p_source, const ImageFormat& p_source_format, const Image p_target, const ImageFormat& p_target_format);
void command_image_layout_transition(LogicalDevice p_device, CommandBuffer p_command_buffer, const Image p_image, const ImageFormat& p_image_format, const GPUPipelineStageFlag p_source_stage,
                                     const ImageLayoutFlag p_source_layout, const GPUAccessFlag p_source_access, const GPUPipelineStageFlag p_target_stage, const ImageLayoutFlag p_target_layout,
                                     const GPUAccessFlag p_target_access);

CommandPool command_pool_allocate(const LogicalDevice p_logical_device, const QueueFamily p_queue_family);
void command_pool_destroy(const LogicalDevice p_logical_device, CommandPool p_pool);
CommandBuffer command_pool_allocate_command_buffer(const LogicalDevice p_logical_device, CommandPool p_command_pool);
void command_pool_destroy_command_buffer(const LogicalDevice p_logical_device, CommandPool p_command_pool, CommandBuffer p_command_buffer);

Buffer buffer_allocate(const LogicalDevice p_logical_device, const uimax p_size, const BufferUsageFlag p_usage_flag);
void buffer_destroy(const LogicalDevice p_logical_device, const Buffer p_buffer);
MemoryRequirements buffer_get_memory_requirements(const LogicalDevice p_logical_device, const Buffer p_buffer);
void buffer_bind_memory(const LogicalDevice p_logical_device, const Buffer p_buffer, const DeviceMemory p_memory, const uimax p_offset);

using ImageTilingFlag_t = int8;
enum class ImageTilingFlag : ImageTilingFlag_t
{
    OPTIMAL = 0,
    LINEAR = 1
};

Image image_allocate(const LogicalDevice p_logical_device, const ImageFormat& p_image_format, const ImageTilingFlag p_image_tiling);
void image_destroy(const LogicalDevice p_logical_device, const Image p_image);
MemoryRequirements image_get_memory_requirements(const LogicalDevice p_logical_device, const Image p_image);
void image_bind_memory(const LogicalDevice p_logical_device, const Image p_image, const DeviceMemory p_memory, const uimax p_offset);
} // namespace gpu

#undef GPU_DECLARE_TOKEN