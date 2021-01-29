#pragma once

#include "Common2/common2.hpp"
#include "vulkan/vulkan.h"

namespace v2
{

	using gc_t = VkDevice;
	using gcqueue_t = VkQueue;
	using gcmemory_t = VkDeviceMemory;
	using gpuinstance_t = VkInstance;
	using gpuinstance_debugger_t = VkDebugUtilsMessengerEXT;
}

namespace v2
{
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


}

#include "./instance.hpp"
#include "./memory.hpp"