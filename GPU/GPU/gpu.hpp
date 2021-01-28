#pragma once

#include "Common2/common2.hpp"
#include "vulkan/vulkan.h"

#include "./interface.hpp"


inline void _vk_handle_result(const VkResult p_result)
{
#if RENDER_BOUND_TEST
	if (p_result != VK_SUCCESS)
	{
		abort();
	};
#endif
};

#if RENDER_BOUND_TEST
#define vk_handle_result(Code) _vk_handle_result(Code)
#else
#define vk_handle_result(Code) Code
#endif


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

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
};

#include "./memory.hpp"


inline GPUInstance GPUInstance::allocate(const Slice<int8*>& p_required_extensions)
{
	GPUInstance l_gpu;

	VkApplicationInfo l_app_info{};
	l_app_info.pApplicationName = "vk";

	VkInstanceCreateInfo l_instance_create_info{};
	l_instance_create_info.pApplicationInfo = &l_app_info;




#if RENDER_DEBUG

	int8* l_validation_layers_str[1];
	l_validation_layers_str[0] = "VK_LAYER_KHRONOS_validation";
	Slice<const int8*> l_validation_layers = Slice<const int8*>::build_memory_elementnb((const int8**)l_validation_layers_str, 1);


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

	Span<int8*>l_extensions;

#if RENDER_DEBUG
	l_extensions = Span<int8*>::allocate(p_required_extensions.Size + 1);
#else
	l_extensions = Span<int8*>::allocate(p_required_extensions.Size);
#endif

	l_extensions.copy_memory(0, p_required_extensions);

#if RENDER_DEBUG
	l_extensions.get(l_extensions.Capacity - 1) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

	l_instance_create_info.enabledExtensionCount = (uint32)l_extensions.Capacity;
	l_instance_create_info.ppEnabledExtensionNames = l_extensions.Memory;

	vk_handle_result(vkCreateInstance(&l_instance_create_info, NULL, &l_gpu.instance));

	l_extensions.free();

#if RENDER_DEBUG
	l_available_layers.free();
#endif

#if RENDER_DEBUG
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

	// l_gpu.pick_physical_device();

	return l_gpu;
};

inline void GPUInstance::free()
{
#if RENDER_DEBUG
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr((VkInstance)instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		func((VkInstance)this->instance, this->debugger, NULL);
	}
#endif

	vkDestroyInstance(this->instance, NULL);
};

inline GraphicsCard GraphicsCard::allocate(const GPUInstance& p_instance)
{
	GraphicsCard l_gc;
	Span<VkPhysicalDevice> l_physical_devices = vk::enumeratePhysicalDevices(p_instance.instance);

	for (loop(i, 0, l_physical_devices.Capacity))
	{
		VkPhysicalDevice& l_physical_device = l_physical_devices.get(i);
		VkPhysicalDeviceProperties l_physical_device_properties;
		vkGetPhysicalDeviceProperties(l_physical_device, &l_physical_device_properties);
		if (l_physical_device_properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			Span<VkQueueFamilyProperties> l_queueFamilies = vk::getPhysicalDeviceQueueFamilyProperties(l_physical_device);

			for (loop(j, 0, l_queueFamilies.Capacity))
			{
				if (l_queueFamilies.get(j).queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				{
					l_gc.physical_device.graphics_queue_family = (uint32)j;
					l_gc.physical_device.present_queue_family = (uint32)j;
					break;
				}
			}
			
			l_queueFamilies.free();

			l_gc.physical_device.device = l_physical_device;
			break;
		}
	}

	l_physical_devices.free();


	VkDeviceQueueCreateInfo l_devicequeue_create_info{};
	l_devicequeue_create_info.queueFamilyIndex = l_gc.physical_device.graphics_queue_family;
	l_devicequeue_create_info.queueCount = 1;
	const float32 l_priority = 1.0f;
	l_devicequeue_create_info.pQueuePriorities = &l_priority;

	// VkPhysicalDeviceFeatures l_devicefeatures;
	VkDeviceCreateInfo l_device_create_info{};
	l_device_create_info.pQueueCreateInfos = &l_devicequeue_create_info;
	l_device_create_info.queueCreateInfoCount = 1;
	

#if RENDER_DEBUG

	int8* l_validation_layers_str[1];
	l_validation_layers_str[0] = "VK_LAYER_KHRONOS_validation";
	Slice<const int8*> l_validation_layers = Slice<const int8*>::build_memory_elementnb((const int8**)l_validation_layers_str, 1);


	l_device_create_info.enabledLayerCount = (uint32)l_validation_layers.Size;
	l_device_create_info.ppEnabledLayerNames =  l_validation_layers.Begin;
#endif

	int8* l_device_extensions_str[1];
	l_device_extensions_str[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	Slice<const int8*> l_device_extensions = Slice<const int8*>::build_memory_elementnb((const int8**)l_device_extensions_str, 1);

	l_device_create_info.enabledExtensionCount = (uint32)l_device_extensions.Size;
	l_device_create_info.ppEnabledExtensionNames = l_device_extensions.Begin;

	vk_handle_result(vkCreateDevice(l_gc.physical_device.device, &l_device_create_info, NULL, &l_gc.device));

	vkGetDeviceQueue(l_gc.device, l_gc.physical_device.graphics_queue_family, 0, &l_gc.graphics_queue);
	vkGetDeviceQueue(l_gc.device, l_gc.physical_device.present_queue_family, 0, &l_gc.present_queue);

	l_gc.heap = GraphicsCardHeap::allocate_default(l_gc.device);
	
	return l_gc;
};

inline void GraphicsCard::free()
{
	this->heap.free(this->device);
	vkDestroyDevice(this->device, NULL);
};

inline GPU GPU::allocate(const Slice<int8*>& p_required_extensions)
{
	GPU l_gpu;
	l_gpu.instance = GPUInstance::allocate(p_required_extensions);
	l_gpu.gc = GraphicsCard::allocate(l_gpu.instance);
	return l_gpu;
};

inline void GPU::free()
{
	this->gc.free();
	this->instance.free();
};


inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	v2::String l_severity = v2::String::allocate(100);

	switch (messageSeverity)
	{
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		l_severity.append(slice_int8_build_rawstr("[Verbose] - "));
		break;
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		l_severity.append(slice_int8_build_rawstr("[Warn] - "));
		break;
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		l_severity.append(slice_int8_build_rawstr("[Error] - "));
		break;
	}
	l_severity.append(slice_int8_build_rawstr("validation layer: "));
	l_severity.append(slice_int8_build_rawstr(pCallbackData->pMessage));
	l_severity.append(slice_int8_build_rawstr("\n"));
	printf(l_severity.get_memory());

	l_severity.free();

	return VK_FALSE;
};