#pragma once

// #include "Common2/common2.hpp"
#include "Math2/math.hpp"
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
#if GPU_BOUND_TEST
		if (p_result != VK_SUCCESS)
		{
			abort();
		};
#endif
	};

#if GPU_BOUND_TEST
#define vk_handle_result(Code) _vk_handle_result(Code)
#else
#define vk_handle_result(Code) Code
#endif

	inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
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
		printf(l_severity.get_memory());

		l_severity.free();

		if (is_error)
		{
			abort();
		}

		return VK_FALSE;
	};

}

#include "./command_buffer.hpp"
#include "./instance.hpp"
#include "./memory.hpp"
#include "./graphics.hpp"
#include "./shader_compiler.hpp"


#undef ShadowBuffer_t
#undef ShadowBuffer_member_buffer
#undef ShadowBuffer_member_size

#undef ShadowImage
#undef ShadowImage_member_image
#undef ShadowImage_member_format

#undef ShadowShaderUniformBufferParameter_t
#undef ShadowShaderUniformBufferParameter_member_descriptor_set
#undef ShadowShaderUniformBufferParameter_member_memory
#undef ShadowShaderUniformBufferParameter_method_free
#undef ShadowShaderUniformBufferParameter_methodcall_free