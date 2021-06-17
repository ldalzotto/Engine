#pragma once


inline Slice<gpu::LayerConstString> gpu_get_platform_surface_extensions();
inline gpu::Surface gpu_create_surface(gpu::Instance p_instance, const window_native p_window);