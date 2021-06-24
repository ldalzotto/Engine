#pragma once

#include "vulkan_win32.h"

SliceN<gpu::LayerConstString, 1> _g_platform_surface_extensions = {VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

inline Slice<gpu::LayerConstString> gpu_get_platform_surface_extensions()
{
    return slice_from_slicen(&_g_platform_surface_extensions);
};

inline gpu::Surface gpu_create_surface(gpu::Instance p_instance, const window_native p_window)
{
    VkWin32SurfaceCreateInfoKHR l_win32_surface_create{};
    l_win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    l_win32_surface_create.hwnd = (HWND)p_window.ptr;
    l_win32_surface_create.hinstance = GetModuleHandle(NULL);

    gcsurface_t l_vksurface;
    vk_handle_result(vkCreateWin32SurfaceKHR((VkInstance)token_value(p_instance), &l_win32_surface_create, NULL, &l_vksurface));

    gpu::Surface l_surface = token_build<gpu::_Surface>((token_t)l_vksurface);
    return l_surface;
};