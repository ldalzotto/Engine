#pragma once

// TODO -> implement these functions

inline Slice<gpu::LayerConstString> gpu_get_platform_surface_extensions()
{
    return Slice<gpu::LayerConstString>::build_default();
};

inline gpu::Surface gpu_create_surface(gpu::Instance p_instance, const window_native p_window)
{
    return token_build_default<gpu::_Surface>();
};