#pragma once

#include "vulkan_win32.h"

SliceN<int8*, 1> _g_platform_surface_extensions = {VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

inline Slice<int8*> gpu_get_platform_surface_extensions()
{
    return slice_from_slicen(&_g_platform_surface_extensions);
};

inline VkSurfaceKHR gpu_create_surface(VkInstance p_instance, const window_native p_window)
{
    VkWin32SurfaceCreateInfoKHR l_win32_surface_create{};
    l_win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    l_win32_surface_create.hwnd = (HWND)p_window.ptr;
    l_win32_surface_create.hinstance = GetModuleHandle(NULL);

    gcsurface_t l_surface;
    vk_handle_result(vkCreateWin32SurfaceKHR(p_instance, &l_win32_surface_create, NULL, &l_surface));
    return l_surface;
};