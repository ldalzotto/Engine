#pragma once

/*
    The GraphicsCard is the hardware responsible of all GPU related operations.
*/
struct GraphicsCard
{
    gpu::PhysicalDevice device;
    gpu::PhysicalDeviceMemoryProperties device_memory_properties;
    gpu::QueueFamily transfer_queue_family;
    gpu::QueueFamily graphics_queue_family;

    gpu::PhysicalDeviceMemoryIndex get_memory_type_index(const gpu::MemoryTypeFlag p_memory_type_required, const gpu::MemoryTypeFlag p_memory_type) const;
};

/*
    The GPUInstance is the root of all GPU related operations.
    When instanciated, it detects compatible GraphicsCard devices.
*/
struct GPUInstance
{
    gpu::Instance instance;
#if __DEBUG
    gpu::Debugger debugger;
#endif
    GraphicsCard graphics_card;
    gpu::LogicalDevice logical_device;

    static GPUInstance allocate(const Slice<gpu::GPUExtension>& p_required_extensions);
    void free();
};

inline gpu::PhysicalDeviceMemoryIndex GraphicsCard::get_memory_type_index(const gpu::MemoryTypeFlag p_memory_type_required, const gpu::MemoryTypeFlag p_memory_type) const
{
    return gpu::physical_device_get_memorytype_index(this->device_memory_properties, p_memory_type_required, p_memory_type);
};

inline GPUInstance GPUInstance::allocate(const Slice<gpu::GPUExtension>& p_required_instance_extensions)
{
    GPUInstance l_gpu;

    SliceN<gpu::LayerConstString, 1> l_validation_layers;
    VectorSlice<gpu::LayerConstString> l_validation_layers_vslice = VectorSlice<gpu::LayerConstString>::build(slice_from_slicen(&l_validation_layers), 0);

#if __DEBUG
    gpu::layer_push_debug_layers(l_validation_layers_vslice);
#endif

    gpu::ApplicationInfo l_application_info;
    l_application_info.enabled_layer_names = l_validation_layers_vslice.to_slice();
    l_application_info.enabled_extensions = p_required_instance_extensions;
    l_gpu.instance = gpu::create_instance(l_application_info);

#if __DEBUG
    l_gpu.debugger = gpu::initialize_debug_callback(l_gpu.instance);
#endif

    gpu::physical_device_pick_Return l_physical_device_pick = gpu::physical_device_pick(l_gpu.instance);
    l_gpu.graphics_card.device = l_physical_device_pick.physical_device;
    l_gpu.graphics_card.graphics_queue_family = l_physical_device_pick.graphics_queue_family;
    l_gpu.graphics_card.transfer_queue_family = l_physical_device_pick.transfer_queue_family;
    l_gpu.graphics_card.device_memory_properties = l_physical_device_pick.physical_memory_properties;

    l_gpu.logical_device = gpu::logical_device_create(l_gpu.graphics_card.device, l_validation_layers_vslice.to_slice(), p_required_instance_extensions, l_gpu.graphics_card.transfer_queue_family);

    return l_gpu;
};

inline void GPUInstance::free()
{
#if __DEBUG
    gpu::debugger_finalize(this->debugger, this->instance);
#endif

    gpu::logical_device_destroy(this->logical_device);
    gpu::instance_destroy(this->instance);
};
