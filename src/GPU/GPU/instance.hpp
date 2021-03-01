#pragma once

namespace v2
{

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
#if GPU_DEBUG
    gpuinstance_debugger_t debugger;
#endif
    GraphicsCard graphics_card;
    gc_t logical_device;

    static GPUInstance allocate(const Slice<int8*>& p_required_extensions);
    void free();
};
} // namespace v2

namespace v2
{

namespace vk
{
inline Span<VkLayerProperties> enumerateInstanceLayerProperties()
{
    uint32_t l_size;
    vk_handle_result(vkEnumerateInstanceLayerProperties(&l_size, NULL));
    Span<VkLayerProperties> l_array = Span<VkLayerProperties>::allocate(l_size);
    vk_handle_result(vkEnumerateInstanceLayerProperties((uint32_t*)&l_array.Capacity, l_array.Memory));
    return l_array;
};

inline Span<VkPhysicalDevice> enumeratePhysicalDevices(const gpuinstance_t p_instance)
{
    uint32_t l_size;
    vk_handle_result(vkEnumeratePhysicalDevices(p_instance, &l_size, NULL));
    Span<VkPhysicalDevice> l_array = Span<VkPhysicalDevice>::allocate(l_size);
    vk_handle_result(vkEnumeratePhysicalDevices(p_instance, (uint32_t*)&l_array.Capacity, l_array.Memory));
    return l_array;
};

inline Span<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(const VkPhysicalDevice p_physical_device)
{
    uint32_t l_size;
    vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device, &l_size, NULL);
    Span<VkQueueFamilyProperties> l_array = Span<VkQueueFamilyProperties>::allocate(l_size);
    vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device, (uint32_t*)&l_array.Capacity, l_array.Memory);
    return l_array;
};
}; // namespace vk

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
                // return this->device_memory_properties.memoryTypes[i].heapIndex;
            };
        }
        l_type_bits >>= 1;
    }
    return -1;
};

inline GPUInstance GPUInstance::allocate(const Slice<int8*>& p_required_instance_extensions)
{
    GPUInstance l_gpu;

    VkApplicationInfo l_app_info{};
    l_app_info.pApplicationName = "vk";

    VkInstanceCreateInfo l_instance_create_info{};
    l_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    l_instance_create_info.pApplicationInfo = &l_app_info;

#if GPU_DEBUG

    const int8* l_validation_layers_str[1] = {"VK_LAYER_KHRONOS_validation"};
    // l_validation_layers_str[0] = "VK_LAYER_KHRONOS_validation";
    Slice<const int8*> l_validation_layers = Slice<const int8*>::build_memory_elementnb(l_validation_layers_str, 1);

    Span<VkLayerProperties> l_available_layers = vk::enumerateInstanceLayerProperties();
    for (loop(i, 0, l_validation_layers.Size))
    {
        int8 l_layer_match = 0;
        for (loop(j, 0, l_available_layers.Capacity))
        {
            if (slice_int8_build_rawstr(l_validation_layers.get(i)).compare(slice_int8_build_rawstr(l_available_layers.get(j).layerName)))
            {
                l_layer_match = 1;
                break;
            }
        }
        if (!l_layer_match)
        {
            printf("validation layers requested, but not available!");
            abort();
        }
    }

    l_instance_create_info.enabledLayerCount = (uint32)l_validation_layers.Size;
    l_instance_create_info.ppEnabledLayerNames = l_validation_layers.Begin;

#else
    l_instance_create_info.enabledLayerCount = 0;
#endif

    Span<int8*> l_extensions;

#if GPU_DEBUG
    l_extensions = Span<int8*>::allocate(p_required_instance_extensions.Size + 1);
#else
    l_extensions = Span<int8*>::allocate(p_required_instance_extensions.Size);
#endif

    l_extensions.slice.copy_memory(0, p_required_instance_extensions);

#if GPU_DEBUG
    l_extensions.get(l_extensions.Capacity - 1) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

    l_instance_create_info.enabledExtensionCount = (uint32)l_extensions.Capacity;
    l_instance_create_info.ppEnabledExtensionNames = l_extensions.Memory;

    vk_handle_result(vkCreateInstance(&l_instance_create_info, NULL, &l_gpu.instance));

    l_extensions.free();

#if GPU_DEBUG
    l_available_layers.free();
#endif

#if GPU_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT l_createinfo = {};
    l_createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    l_createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    l_createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    l_createinfo.pfnUserCallback = debugCallback;

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(l_gpu.instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(l_gpu.instance, &l_createinfo, NULL, &l_gpu.debugger);
    }
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

#if GPU_DEBUG

    l_device_create_info.enabledLayerCount = (uint32)l_validation_layers.Size;
    l_device_create_info.ppEnabledLayerNames = l_validation_layers.Begin;
#endif

    l_device_create_info.enabledExtensionCount = 0;

    vk_handle_result(vkCreateDevice(l_gpu.graphics_card.device, &l_device_create_info, NULL, &l_gpu.logical_device));

    return l_gpu;
};

inline void GPUInstance::free()
{
#if GPU_DEBUG
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func((VkInstance)this->instance, this->debugger, NULL);
    }
#endif

    vkDestroyDevice(this->logical_device, NULL);
    vkDestroyInstance(this->instance, NULL);
};

} // namespace v2