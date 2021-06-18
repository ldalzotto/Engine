#pragma once

#include "vk_platform.h"
#include "vulkan_core.h"

using gc_t = VkDevice;
using gcqueue_t = VkQueue;
using gcmemory_t = VkDeviceMemory;
using gpuinstance_t = VkInstance;
using gpuinstance_debugger_t = VkDebugUtilsMessengerEXT;

inline void _vk_handle_result(const VkResult p_result)
{
#if __DEBUG
    if (p_result != VK_SUCCESS)
    {
        abort();
    };
#endif
};

#if __DEBUG
#define vk_handle_result(Code) _vk_handle_result(Code)
#else
#define vk_handle_result(Code) Code
#endif

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

inline Span<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModesKHR(const VkPhysicalDevice p_physical_device, const VkSurfaceKHR p_surface)
{
    uint32_t l_size;
    vk_handle_result(vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface, &l_size, NULL));
    Span<VkPresentModeKHR> l_array = Span<VkPresentModeKHR>::allocate(l_size);
    vk_handle_result(vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface, (uint32_t*)&l_array.Capacity, l_array.Memory));
    return l_array;
};

inline Span<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormatsKHR(const VkPhysicalDevice p_physical_device, const VkSurfaceKHR p_surface)
{
    uint32_t l_size;
    vk_handle_result(vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface, &l_size, NULL));
    Span<VkSurfaceFormatKHR> l_array = Span<VkSurfaceFormatKHR>::allocate(l_size);
    vk_handle_result(vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface, (uint32_t*)&l_array.Capacity, l_array.Memory));
    return l_array;
};

inline Span<VkImage> getSwapchainImagesKHR(const VkDevice p_device, const VkSwapchainKHR p_swap_chain)
{
    uint32_t l_size;
    vk_handle_result(vkGetSwapchainImagesKHR(p_device, p_swap_chain, &l_size, NULL));
    Span<VkImage> l_array = Span<VkImage>::allocate(l_size);
    vk_handle_result(vkGetSwapchainImagesKHR(p_device, p_swap_chain, (uint32_t*)&l_array.Capacity, l_array.Memory));
    return l_array;
};

}; // namespace vk

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{

    String l_severity = String::allocate(100);

    int8 is_error = 0;

    switch (messageSeverity)
    {
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        // l_severity.append(slice_int8_build_rawstr("[Verbose] - "));
        l_severity.free();
        return VK_FALSE;
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        l_severity.append(slice_int8_build_rawstr("[Info] - "));
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        l_severity.append(slice_int8_build_rawstr("[Warn] - "));
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        l_severity.append(slice_int8_build_rawstr("[Error] - "));
        is_error = 1;
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        is_error = 1;
        break;
    }
    l_severity.append(slice_int8_build_rawstr("validation layer: "));
    l_severity.append(slice_int8_build_rawstr(pCallbackData->pMessage));
    l_severity.append(slice_int8_build_rawstr("\n"));
    printf("%s\n", l_severity.get_memory());

    l_severity.free();

    if (is_error)
    {
        abort();
    }

    return VK_FALSE;
};

void gpu::layer_push_debug_layers(VectorSlice<LayerConstString>& p_layers)
{
    SliceN<LayerConstString, 1> l_validation_layers_arr{"VK_LAYER_KHRONOS_validation"};
    Slice<LayerConstString> l_validation_layers = slice_from_slicen(&l_validation_layers_arr);

    Span<VkLayerProperties> l_available_layers = vk::enumerateInstanceLayerProperties();
    for (loop(i, 0, l_validation_layers.Size))
    {
        int8 l_layer_match = 0;
        for (loop(j, 0, l_available_layers.Capacity))
        {
            if (slice_from_slicen(&l_validation_layers.get(i)).compare(slice_int8_build_rawstr(l_available_layers.get(j).layerName)))
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
    l_available_layers.free();

    LayerConstString l_layer = {"VK_LAYER_KHRONOS_validation"};
    p_layers.push_back_element(l_layer);
};

gpu::Instance gpu::create_instance(const ApplicationInfo& p_application_info)
{
    VkApplicationInfo l_app_info{};
    l_app_info.pApplicationName = "vk";

    VkInstanceCreateInfo l_instance_create_info{};
    l_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    l_instance_create_info.pApplicationInfo = &l_app_info;

    SliceN<int8*, 10> l_layers;
    for (loop(i, 0, p_application_info.enabled_layer_names.Size))
    {
        l_layers.get(i) = (int8*)&p_application_info.enabled_layer_names.get(i);
    }
    l_instance_create_info.enabledLayerCount = (uint32)p_application_info.enabled_layer_names.Size;
    l_instance_create_info.ppEnabledLayerNames = l_layers.Memory;

    Vector<LayerConstString> l_extensions = Vector<LayerConstString>::allocate(0);

    for (loop(i, 0, p_application_info.enabled_extensions.Size))
    {
        switch (p_application_info.enabled_extensions.get(i))
        {
        case GPUExtension::WINDOW_PRESENT:
            LayerConstString l_extension = {VK_KHR_SURFACE_EXTENSION_NAME};
            l_extensions.push_back_element(l_extension);
            l_extensions.push_back_array(gpu_get_platform_surface_extensions());
            break;
        }
    }

#if __DEBUG
    LayerConstString l_extension = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    l_extensions.push_back_element(l_extension);
#endif

    SliceN<int8*, 10> l_extensions_raw;
    for (loop(i, 0, l_extensions.Size))
    {
        l_extensions_raw.get(i) = (int8*)&l_extensions.get(i);
    }
    l_instance_create_info.enabledExtensionCount = (uint32)l_extensions.Size;
    l_instance_create_info.ppEnabledExtensionNames = l_extensions_raw.Memory;

    VkInstance l_vkinstance;
    vk_handle_result(vkCreateInstance(&l_instance_create_info, NULL, &l_vkinstance));

    l_extensions.free();

    gpu::Instance l_instance;
    l_instance.tok = (uimax)l_vkinstance;
    return l_instance;
};

void gpu::instance_destroy(Instance p_instance)
{
    vkDestroyInstance((VkInstance)p_instance.tok, NULL);
};

gpu::Debugger gpu::initialize_debug_callback(Instance p_instance)
{
    VkDebugUtilsMessengerCreateInfoEXT l_createinfo = {};
    l_createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    l_createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    l_createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    l_createinfo.pfnUserCallback = debugCallback;

    gpu::Debugger l_debugger;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)p_instance.tok, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func((VkInstance)p_instance.tok, &l_createinfo, NULL, (VkDebugUtilsMessengerEXT*)&l_debugger.tok);
    }
    return l_debugger;
};

void gpu::debugger_finalize(Debugger p_debugger, Instance p_instance)
{
#if __DEBUG
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)p_instance.tok, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func((VkInstance)p_instance.tok, (VkDebugUtilsMessengerEXT)p_debugger.tok, NULL);
    }
#endif
};

gpu::physical_device_pick_Return gpu::physical_device_pick(Instance p_instance)
{
    gpu::physical_device_pick_Return l_picked_device = gpu::physical_device_pick_Return::build_default();

    Span<VkPhysicalDevice> l_physical_devices = vk::enumeratePhysicalDevices((VkInstance)p_instance.tok);

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
                    l_picked_device.transfer_queue_family.family = (uint32)j;
                    l_queueus_found += 1;
                    if (l_queueus_found == 2)
                    {
                        break;
                    }
                }

                if (l_queueFamilies.get(j).queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
                {
                    l_picked_device.graphics_queue_family.family = (uint32)j;
                    l_queueus_found += 1;
                    if (l_queueus_found == 2)
                    {
                        break;
                    }
                }
            }

            l_queueFamilies.free();

            l_picked_device.physical_device.tok = (token_t)l_physical_device;
            VkPhysicalDeviceMemoryProperties l_memory_properties;
            vkGetPhysicalDeviceMemoryProperties(l_physical_device, &l_memory_properties);

            l_picked_device.physical_memory_properties.memory_types_size = l_memory_properties.memoryTypeCount;
            for (loop(k, 0, l_memory_properties.memoryTypeCount))
            {
                gpu::MemoryType& l_memory_type = l_picked_device.physical_memory_properties.memory_types.get(k);
                l_memory_type.heap_index.index = l_memory_properties.memoryTypes[k].heapIndex;
                l_memory_type.type = (gpu::MemoryTypeFlag)l_memory_properties.memoryTypes[k].propertyFlags;
            }

            l_picked_device.physical_memory_properties.memory_heaps_size = l_memory_properties.memoryHeapCount;
            for (loop(k, 0, l_memory_properties.memoryHeapCount))
            {
                gpu::MemoryHeap& l_memory_heap = l_picked_device.physical_memory_properties.memory_heaps.get(k);
                l_memory_heap.size = l_memory_properties.memoryHeaps[k].size;
            }

            break;
        }
    }

    l_physical_devices.free();
    return l_picked_device;
};

gpu::PhysicalDeviceMemoryIndex gpu::physical_device_get_memorytype_index(const PhysicalDeviceMemoryProperties& p_memory_properties, const MemoryTypeFlag p_required_memory_type,
                                                                         const MemoryTypeFlag p_memory_type)
{
    MemoryTypeFlag_t l_type_bits = (MemoryTypeFlag_t)p_required_memory_type;
    for (int8 i = 0; i < p_memory_properties.memory_types_size; i++)
    {
        if ((l_type_bits & 1) == 1)
        {
            if (((gpu::MemoryTypeFlag_t)(p_memory_properties.memory_types.get(i).type) & (gpu::MemoryTypeFlag_t)p_memory_type) == (gpu::MemoryTypeFlag_t)p_memory_type)
            {
                gpu::PhysicalDeviceMemoryIndex l_return;
                l_return.index = i;
                return l_return;
            };
        }
        l_type_bits >>= 1;
    }
    gpu::PhysicalDeviceMemoryIndex l_return;
    l_return.index = -1;
    return l_return;
};

gpu::LogicalDevice gpu::logical_device_create(PhysicalDevice p_physical_device, const Slice<LayerConstString>& p_validation_layers, const Slice<GPUExtension>& p_gpu_extensions,
                                              const QueueFamily& p_queue)
{
    VkDeviceQueueCreateInfo l_devicequeue_create_info{};
    l_devicequeue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    l_devicequeue_create_info.queueFamilyIndex = p_queue.family;
    l_devicequeue_create_info.queueCount = 1;
    const float32 l_priority = 1.0f;
    l_devicequeue_create_info.pQueuePriorities = &l_priority;

    VkDeviceCreateInfo l_device_create_info{};
    l_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    l_device_create_info.pQueueCreateInfos = &l_devicequeue_create_info;
    l_device_create_info.queueCreateInfoCount = 1;

#if __DEBUG
    l_device_create_info.enabledLayerCount = (uint32)p_validation_layers.Size;
    l_device_create_info.ppEnabledLayerNames = (const char* const*)&p_validation_layers.Begin;
#endif

    int8 l_window_present_enabled = 0;
    for (loop(i, 0, p_gpu_extensions.Size))
    {
        if (p_gpu_extensions.get(0) == GPUExtension::WINDOW_PRESENT)
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

    gpu::LogicalDevice l_logical_device;
    vk_handle_result(vkCreateDevice((VkPhysicalDevice)p_physical_device.tok, &l_device_create_info, NULL, (VkDevice*)&l_logical_device.tok));
    return l_logical_device;
};

void gpu::logical_device_destroy(LogicalDevice p_logical_device)
{
    vkDestroyDevice((VkDevice)p_logical_device.tok, NULL);
};

gpu::DeviceMemory gpu::allocate_memory(const LogicalDevice p_device, const uimax p_allocation_size, const PhysicalDeviceMemoryIndex p_memory_index)
{
    VkMemoryAllocateInfo l_memoryallocate_info{};
    l_memoryallocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    l_memoryallocate_info.allocationSize = p_allocation_size;
    l_memoryallocate_info.memoryTypeIndex = p_memory_index.index;

    DeviceMemory l_device_memory;
    vkAllocateMemory((VkDevice)p_device.tok, &l_memoryallocate_info, NULL, (VkDeviceMemory*)&l_device_memory.tok);
    return l_device_memory;
};

void gpu::free_memory(const LogicalDevice p_device, const DeviceMemory p_device_memory)
{
    vkFreeMemory((VkDevice)p_device.tok, (VkDeviceMemory)p_device_memory.tok, NULL);
};

int8* gpu::map_memory(const LogicalDevice p_device, const DeviceMemory p_device_memory, const uimax p_offset, const uimax p_size)
{
    int8* l_mapped_memory;
    vk_handle_result(vkMapMemory((VkDevice)p_device.tok, (VkDeviceMemory)p_device_memory.tok, p_offset, p_size, 0, (void**)&l_mapped_memory));
    return l_mapped_memory;
};