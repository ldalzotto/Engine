#pragma once


inline Slice<int8*> gpu_get_platform_surface_extensions();
inline VkSurfaceKHR gpu_create_surface(VkInstance p_instance, const window_native p_window);