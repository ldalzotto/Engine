#pragma once

#include "vulkan_xlib.h"

SliceN<int8*, 1> _g_platform_surface_extensions = {VK_KHR_XLIB_SURFACE_EXTENSION_NAME};

inline Slice<int8*> gpu_get_platform_surface_extensions(){
    return slice_from_slicen(&_g_platform_surface_extensions);
};

inline VkSurfaceKHR gpu_create_surface(VkInstance p_instance, const window_native p_window)
{
    VkXlibSurfaceCreateInfoKHR l_xlib_surface_create{};
    l_xlib_surface_create.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    l_xlib_surface_create.window = (Window)p_window.ptr;
    l_xlib_surface_create.dpy = g_display_info.display;

    VkSurfaceKHR l_surface;
    vk_handle_result(vkCreateXlibSurfaceKHR(p_instance, &l_xlib_surface_create, NULL, &l_surface));
    return l_surface;
};