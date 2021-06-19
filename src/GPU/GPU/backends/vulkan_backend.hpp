#pragma once

#include "vk_platform.h"
#include "vulkan_core.h"

using gc_t = VkDevice;
using gcqueue_t = VkQueue;
using gpuinstance_t = VkInstance;

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

gpu::Queue gpu::logical_device_get_queue(const LogicalDevice p_logical_device, const QueueFamily p_queue_family)
{
    Queue l_queue;
    vkGetDeviceQueue((VkDevice)p_logical_device.tok, p_queue_family.family, 0, (VkQueue*)&l_queue.tok);
    return l_queue;
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

gpu::Semaphore gpu::semaphore_allocate(const LogicalDevice p_device)
{
    VkSemaphoreCreateInfo l_semaphore_create_info{};
    l_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore l_vk_semaphore;
    vk_handle_result(vkCreateSemaphore((VkDevice)p_device.tok, &l_semaphore_create_info, NULL, &l_vk_semaphore));
    gpu::Semaphore l_semaphore;
    l_semaphore.tok = (token_t)l_vk_semaphore;
    return l_semaphore;
};

void gpu::semaphore_destroy(const LogicalDevice p_device, const Semaphore p_semaphore)
{
    vkDestroySemaphore((VkDevice)p_device.tok, (VkSemaphore)p_semaphore.tok, NULL);
};

void gpu::queue_wait_idle(Queue p_queue)
{
    vk_handle_result(vkQueueWaitIdle((VkQueue)p_queue.tok));
};

void gpu::command_buffer_begin(CommandBuffer p_command_buffer)
{
    VkCommandBufferBeginInfo l_command_buffer_begin_info{};
    l_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_handle_result(vkBeginCommandBuffer((VkCommandBuffer)p_command_buffer.tok, &l_command_buffer_begin_info));
};

void gpu::command_buffer_end(CommandBuffer p_command_buffer)
{
    vk_handle_result(vkEndCommandBuffer((VkCommandBuffer)p_command_buffer.tok));
};

void gpu::command_buffer_submit(CommandBuffer p_command_buffer, Queue p_queue)
{
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = (VkCommandBuffer*)&p_command_buffer.tok;
    vk_handle_result(vkQueueSubmit((VkQueue)p_queue.tok, 1, &l_wait_for_end_submit, NULL));
};

void gpu::command_buffer_submit_and_notify(CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_notified_semaphore)
{
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = (VkCommandBuffer*)&p_command_buffer.tok;
    l_wait_for_end_submit.signalSemaphoreCount = 1;
    l_wait_for_end_submit.pSignalSemaphores = (VkSemaphore*)&p_notified_semaphore.tok;
    vk_handle_result(vkQueueSubmit((VkQueue)p_queue.tok, 1, &l_wait_for_end_submit, NULL));
};

void gpu::command_buffer_submit_after(CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore)
{
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = (VkCommandBuffer*)&p_command_buffer.tok;
    l_wait_for_end_submit.waitSemaphoreCount = 1;
    l_wait_for_end_submit.pWaitSemaphores = (VkSemaphore*)&p_after_semaphore.tok;
    VkPipelineStageFlags l_stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
    l_wait_for_end_submit.pWaitDstStageMask = &l_stage;
    vk_handle_result(vkQueueSubmit((VkQueue)p_queue.tok, 1, &l_wait_for_end_submit, NULL));
};

void gpu::command_buffer_submit_after_and_notify(CommandBuffer p_command_buffer, Queue p_queue, Semaphore p_after_semaphore, Semaphore p_notify_semaphore)
{
    VkSubmitInfo l_wait_for_end_submit{};
    l_wait_for_end_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    l_wait_for_end_submit.commandBufferCount = 1;
    l_wait_for_end_submit.pCommandBuffers = (VkCommandBuffer*)&p_command_buffer.tok;
    l_wait_for_end_submit.waitSemaphoreCount = 1;
    l_wait_for_end_submit.pWaitSemaphores = (VkSemaphore*)&p_after_semaphore.tok;
    l_wait_for_end_submit.signalSemaphoreCount = 1;
    l_wait_for_end_submit.pSignalSemaphores = (VkSemaphore*)&p_notify_semaphore.tok;
    VkPipelineStageFlags l_stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
    l_wait_for_end_submit.pWaitDstStageMask = &l_stage;
    vk_handle_result(vkQueueSubmit((VkQueue)p_queue.tok, 1, &l_wait_for_end_submit, NULL));
};

void gpu::command_copy_buffer(CommandBuffer p_command_buffer, const Buffer p_source, const uimax p_source_size, Buffer p_target, const uimax p_target_size)
{
    VkBufferCopy l_buffer_copy{};
    l_buffer_copy.size = p_source_size;
    vkCmdCopyBuffer((VkCommandBuffer)token_value(p_command_buffer), (VkBuffer)token_value(p_source), (VkBuffer)token_value(p_target), 1, &l_buffer_copy);
};

void gpu::command_copy_buffer_to_image(CommandBuffer p_command_buffer, const Buffer p_source, const uimax p_source_size, const Image p_target, const ImageFormat& p_target_format)
{
    VkBufferImageCopy l_buffer_image_copy{};
    l_buffer_image_copy.imageSubresource = VkImageSubresourceLayers{(VkImageAspectFlags)p_target_format.imageAspect, 0, 0, (uint32_t)p_target_format.arrayLayers};
    l_buffer_image_copy.imageExtent = VkExtent3D{(uint32_t)p_target_format.extent.x, (uint32_t)p_target_format.extent.y, (uint32_t)p_target_format.extent.z};

    vkCmdCopyBufferToImage((VkCommandBuffer)token_value(p_command_buffer), (VkBuffer)token_value(p_source), (VkImage)token_value(p_target), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &l_buffer_image_copy);
};

void gpu::command_copy_image_to_buffer(CommandBuffer p_command_buffer, const Image p_source, const ImageFormat& p_source_format, const Buffer p_target)
{
    VkBufferImageCopy l_buffer_image_copy{};
    l_buffer_image_copy.imageSubresource = VkImageSubresourceLayers{(VkImageAspectFlags)p_source_format.imageAspect, 0, 0, (uint32_t)p_source_format.arrayLayers};
    l_buffer_image_copy.imageExtent = VkExtent3D{(uint32_t)p_source_format.extent.x, (uint32_t)p_source_format.extent.y, (uint32_t)p_source_format.extent.z};

    vkCmdCopyImageToBuffer((VkCommandBuffer)token_value(p_command_buffer), (VkImage)token_value(p_source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, (VkBuffer)token_value(p_target), 1,
                           &l_buffer_image_copy);
};

void gpu::command_copy_image(CommandBuffer p_command_buffer, const Image p_source, const ImageFormat& p_source_format, const Image p_target, const ImageFormat& p_target_format)
{
    VkImageCopy l_region = {VkImageSubresourceLayers{(VkImageAspectFlags)p_source_format.imageAspect, 0, 0, (uint32_t)p_source_format.arrayLayers}, VkOffset3D{0, 0, 0},
                            VkImageSubresourceLayers{(VkImageAspectFlags)p_target_format.imageAspect, 0, 0, (uint32_t)p_target_format.arrayLayers}, VkOffset3D{0, 0, 0},
                            VkExtent3D{(uint32_t)p_source_format.extent.x, (uint32_t)p_source_format.extent.y, (uint32_t)p_source_format.extent.z}};

    vkCmdCopyImage((VkCommandBuffer)token_value(p_command_buffer), (VkImage)token_value(p_source), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, (VkImage)token_value(p_target),
                   VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &l_region);
};

void gpu::command_image_layout_transition(CommandBuffer p_command_buffer, const Image p_image, const ImageFormat& p_image_format, const GPUPipelineStageFlag p_source_stage,
                                          const ImageLayoutFlag p_source_layout, const GPUAccessFlag p_source_access, const GPUPipelineStageFlag p_target_stage, const ImageLayoutFlag p_target_layout,
                                          const GPUAccessFlag p_target_access)
{
    VkImageMemoryBarrier l_image_memory_barrier{};
    l_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    l_image_memory_barrier.oldLayout = (VkImageLayout)p_source_layout;
    l_image_memory_barrier.newLayout = (VkImageLayout)p_target_layout;
    l_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    l_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    l_image_memory_barrier.image = (VkImage)token_value(p_image);

    l_image_memory_barrier.subresourceRange = VkImageSubresourceRange{(VkImageAspectFlags)p_image_format.imageAspect, 0, (uint32_t)p_image_format.arrayLayers, 0, (uint32_t)p_image_format.arrayLayers};

    l_image_memory_barrier.srcAccessMask = (VkAccessFlags)p_source_access;
    l_image_memory_barrier.dstAccessMask = (VkAccessFlags)p_target_access;

    vkCmdPipelineBarrier((VkCommandBuffer)token_value(p_command_buffer), (VkPipelineStageFlagBits)p_source_stage, (VkPipelineStageFlagBits)p_target_stage, 0, 0, NULL, 0, NULL, 1,
                         &l_image_memory_barrier);
};

gpu::CommandPool gpu::command_pool_allocate(const LogicalDevice p_logical_device, const QueueFamily p_queue_family)
{
    gpu::CommandPool l_command_pool;
    VkCommandPoolCreateInfo l_command_pool_create_info{};
    l_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    l_command_pool_create_info.queueFamilyIndex = p_queue_family.family;
    l_command_pool_create_info.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_handle_result(vkCreateCommandPool((VkDevice)p_logical_device.tok, &l_command_pool_create_info, NULL, (VkCommandPool*)&l_command_pool.tok));
    return l_command_pool;
};

void gpu::command_pool_destroy(const LogicalDevice p_logical_device, CommandPool p_pool)
{
    vkDestroyCommandPool((VkDevice)p_logical_device.tok, (VkCommandPool)p_pool.tok, NULL);
};

gpu::CommandBuffer gpu::command_pool_allocate_command_buffer(const LogicalDevice p_logical_device, CommandPool p_command_pool)
{
    gpu::CommandBuffer l_command_buffer;
    VkCommandBufferAllocateInfo l_command_buffer_allocate_info{};
    l_command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    l_command_buffer_allocate_info.commandPool = (VkCommandPool)p_command_pool.tok;
    l_command_buffer_allocate_info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    l_command_buffer_allocate_info.commandBufferCount = 1;
    vk_handle_result(vkAllocateCommandBuffers((VkDevice)p_logical_device.tok, &l_command_buffer_allocate_info, (VkCommandBuffer*)&l_command_buffer.tok));
    return l_command_buffer;
};

gpu::Buffer gpu::buffer_allocate(const LogicalDevice p_logical_device, const uimax p_size, const BufferUsageFlag p_usage_flag)
{
    gpu::Buffer l_buffer;
    VkBufferCreateInfo l_buffercreate_info{};
    l_buffercreate_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    l_buffercreate_info.usage = (VkBufferUsageFlags)p_usage_flag;
    l_buffercreate_info.size = p_size;

    vk_handle_result(vkCreateBuffer((VkDevice)p_logical_device.tok, &l_buffercreate_info, NULL, (VkBuffer*)&l_buffer.tok));
    return l_buffer;
};

void gpu::buffer_destroy(const LogicalDevice p_logical_device, const Buffer p_buffer)
{
    vkDestroyBuffer((VkDevice)token_value(p_logical_device), (VkBuffer)token_value(p_buffer), NULL);
};

gpu::MemoryRequirements gpu::buffer_get_memory_requirements(const LogicalDevice p_logical_device, const Buffer p_buffer)
{
    VkMemoryRequirements l_requirements;
    vkGetBufferMemoryRequirements((VkDevice)token_value(p_logical_device), (VkBuffer)token_value(p_buffer), &l_requirements);
    gpu::MemoryRequirements l_gpu_requirements;
    l_gpu_requirements.memory_type = (gpu::MemoryTypeFlag)l_requirements.memoryTypeBits;
    l_gpu_requirements.size = l_requirements.size;
    l_gpu_requirements.alignment = l_requirements.alignment;
    return l_gpu_requirements;
};

void gpu::buffer_bind_memory(const LogicalDevice p_logical_device, const Buffer p_buffer, const DeviceMemory p_memory, const uimax p_offset)
{
    vkBindBufferMemory((VkDevice)token_value(p_logical_device), (VkBuffer)token_value(p_buffer), (VkDeviceMemory)token_value(p_memory), p_offset);
};

gpu::Image gpu::image_allocate(const LogicalDevice p_logical_device, const ImageFormat& p_image_format, const ImageTilingFlag p_image_tiling)
{
    VkImageCreateInfo l_image_create_info{};
    l_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    l_image_create_info.imageType = (VkImageType)p_image_format.imageType;
    l_image_create_info.format = (VkFormat)p_image_format.format;
    l_image_create_info.extent = VkExtent3D{(uint32_t)p_image_format.extent.x, (uint32_t)p_image_format.extent.y, (uint32_t)p_image_format.extent.z};
    l_image_create_info.mipLevels = p_image_format.mipLevels;
    l_image_create_info.arrayLayers = p_image_format.arrayLayers;
    l_image_create_info.samples = (VkSampleCountFlagBits)p_image_format.samples;
    l_image_create_info.tiling = (VkImageTiling)p_image_tiling;
    l_image_create_info.usage = (VkImageUsageFlags)p_image_format.imageUsage;
    l_image_create_info.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;

    gpu::Image l_image;
    vk_handle_result(vkCreateImage((VkDevice)token_value(p_logical_device), &l_image_create_info, NULL, (VkImage*)&l_image.tok));
    return l_image;
};

void gpu::image_destroy(const LogicalDevice p_logical_device, const Image p_image)
{
    vkDestroyImage((VkDevice)token_value(p_logical_device), (VkImage)token_value(p_image), NULL);
};

gpu::MemoryRequirements gpu::image_get_memory_requirements(const LogicalDevice p_logical_device, const Image p_image)
{
    VkMemoryRequirements l_requirements;
    vkGetImageMemoryRequirements((VkDevice)token_value(p_logical_device), (VkImage)token_value(p_image), &l_requirements);

    gpu::MemoryRequirements l_gpu_requirements;
    l_gpu_requirements.memory_type = (gpu::MemoryTypeFlag)l_requirements.memoryTypeBits;
    l_gpu_requirements.size = l_requirements.size;
    l_gpu_requirements.alignment = l_requirements.alignment;
    return l_gpu_requirements;
};

void gpu::image_bind_memory(const LogicalDevice p_logical_device, const Image p_image, const DeviceMemory p_memory, const uimax p_offset)
{
    vkBindImageMemory((VkDevice)token_value(p_logical_device), (VkImage)token_value(p_image), (VkDeviceMemory)token_value(p_memory), p_offset);
};