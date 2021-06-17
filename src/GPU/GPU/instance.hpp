#pragma once

/*
    The GraphicsCard is the hardware responsible of all GPU related operations.
*/
struct GraphicsCard
{
    VkPhysicalDevice device;
    VkPhysicalDeviceMemoryProperties device_memory_properties;
    uint32 transfer_queue_family;
    uint32 graphics_queue_family;

    uint32 get_memory_type_index(const VkMemoryRequirements& p_memory_requirements, const VkMemoryPropertyFlags p_properties) const;
};

/*
    The GPUInstance is the root of all GPU related operations.
    When instanciated, it detects compatible GraphicsCard devices.
*/
struct GPUInstance
{
    gpuinstance_t instance;
#if __DEBUG
    gpuinstance_debugger_t debugger;
#endif
    GraphicsCard graphics_card;
    gc_t logical_device;

    static GPUInstance allocate(const Slice<GPUExtension>& p_required_extensions);
    void free();
};

inline uint32 GraphicsCard::get_memory_type_index(const VkMemoryRequirements& p_memory_requirements, const VkMemoryPropertyFlags p_properties) const
{
    uint32 l_type_bits = p_memory_requirements.memoryTypeBits;
    for (uint32 i = 0; i < this->device_memory_properties.memoryTypeCount; i++)
    {
        if ((l_type_bits & 1) == 1)
        {
            if ((this->device_memory_properties.memoryTypes[i].propertyFlags & p_properties) == p_properties)
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
    l_gpu.instance = (gpuinstance_t)gpu::create_instance(l_application_info).tok;

#if __DEBUG
    l_gpu.debugger = (gpuinstance_debugger_t)gpu::initialize_debug_callback(gpu::Instance{(token_t)l_gpu.instance}).tok;
#endif

    Span<VkPhysicalDevice> l_physical_devices = vk::enumeratePhysicalDevices(l_gpu.instance);

    for (loop(i, 0, l_physical_devices.Capacity))
    {
        VkPhysicalDevice& l_physical_device = l_physical_devices.get(i);
        VkPhysicalDeviceProperties l_physical_device_properties;
        vkGetPhysicalDeviceProperties(l_physical_device, &l_physical_device_properties);
        if (l_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            Span<VkQueueFamilyProperties> l_queueFamilies = vk::getPhysicalDeviceQueueFamilyProperties(l_physical_device);
            uint8 l_queueus_found = 0;

            for (loop(j, 0, l_queueFamilies.Capacity))
            {
                if (l_queueFamilies.get(j).queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
                {
                    l_gpu.graphics_card.transfer_queue_family = (uint32)j;
                    l_queueus_found += 1;
                    if (l_queueus_found == 2)
                    {
                        break;
                    }
                }

                if (l_queueFamilies.get(j).queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
                {
                    l_gpu.graphics_card.graphics_queue_family = (uint32)j;
                    l_queueus_found += 1;
                    if (l_queueus_found == 2)
                    {
                        break;
                    }
                }
            }

            l_queueFamilies.free();

            l_gpu.graphics_card.device = l_physical_device;
            vkGetPhysicalDeviceMemoryProperties(l_gpu.graphics_card.device, &l_gpu.graphics_card.device_memory_properties);
            break;
        }
    }

    l_physical_devices.free();

    VkDeviceQueueCreateInfo l_devicequeue_create_info{};
    l_devicequeue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    l_devicequeue_create_info.queueFamilyIndex = l_gpu.graphics_card.transfer_queue_family;
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

    vk_handle_result(vkCreateDevice(l_gpu.graphics_card.device, &l_device_create_info, NULL, &l_gpu.logical_device));

    return l_gpu;
};

inline void GPUInstance::free()
{
#if __DEBUG
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func((VkInstance)this->instance, this->debugger, NULL);
    }
#endif

    vkDestroyDevice(this->logical_device, NULL);
    vkDestroyInstance(this->instance, NULL);
};
