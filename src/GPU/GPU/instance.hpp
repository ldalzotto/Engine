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

    uint32 get_memory_type_index(const VkMemoryRequirements& p_memory_requirements, const VkMemoryPropertyFlags p_properties) const;
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
    gc_t logical_device;

    static GPUInstance allocate(const Slice<GPUExtension>& p_required_extensions);
    void free();
};

inline uint32 GraphicsCard::get_memory_type_index(const VkMemoryRequirements& p_memory_requirements, const VkMemoryPropertyFlags p_properties) const
{
    uint32 l_type_bits = p_memory_requirements.memoryTypeBits;
    for (int8 i = 0; i < this->device_memory_properties.memory_types_size; i++)
    {
        if ((l_type_bits & 1) == 1)
        {
            if (((gpu::MemoryTypeFlag_t)(this->device_memory_properties.memory_types.get(i).type) & p_properties) == p_properties)
            {
                return i;
            };
        }
        l_type_bits >>= 1;
    }
    return -1;
};

inline GPUInstance GPUInstance::allocate(const Slice<GPUExtension>& p_required_instance_extensions)
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


    VkDeviceQueueCreateInfo l_devicequeue_create_info{};
    l_devicequeue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    l_devicequeue_create_info.queueFamilyIndex = l_gpu.graphics_card.transfer_queue_family.family;
    l_devicequeue_create_info.queueCount = 1;
    const float32 l_priority = 1.0f;
    l_devicequeue_create_info.pQueuePriorities = &l_priority;

    VkDeviceCreateInfo l_device_create_info{};
    l_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    l_device_create_info.pQueueCreateInfos = &l_devicequeue_create_info;
    l_device_create_info.queueCreateInfoCount = 1;

#if __DEBUG
    l_device_create_info.enabledLayerCount = (uint32)l_validation_layers_vslice.to_slice().Size;
    l_device_create_info.ppEnabledLayerNames = (const char* const*)&l_validation_layers_vslice.Memory.Begin;
#endif

    int8 l_window_present_enabled = 0;
    for (loop(i, 0, p_required_instance_extensions.Size))
    {
        if (p_required_instance_extensions.get(0) == GPUExtension::WINDOW_PRESENT)
        {
            l_window_present_enabled = 1;
            break;
        }
    }

    SliceN<int8*, 1> l_swap_chain_extension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    if (l_window_present_enabled)
    {
        l_device_create_info.enabledExtensionCount = 1;
        l_device_create_info.ppEnabledExtensionNames = (const char* const*)&l_swap_chain_extension.Memory;
    }

    vk_handle_result(vkCreateDevice((VkPhysicalDevice)l_gpu.graphics_card.device.tok, &l_device_create_info, NULL, &l_gpu.logical_device));

    return l_gpu;
};

inline void GPUInstance::free()
{
#if __DEBUG
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)this->instance.tok, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func((VkInstance)this->instance.tok, (VkDebugUtilsMessengerEXT)this->debugger.tok, NULL);
    }
#endif

    vkDestroyDevice(this->logical_device, NULL);
    vkDestroyInstance((VkInstance)this->instance.tok, NULL);
};
