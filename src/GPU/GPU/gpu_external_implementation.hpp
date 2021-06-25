#pragma once

#if _WIN32
#if GPU_BACKEND_VULKAN
#include "./_external/backends/gpu_platform_vulkan_win32_backend.hpp"
#elif GPU_BACKEND_SOFTWARE
#include "./_external/backends/gpu_platform_software_win32_backend.hpp"
#endif
#elif __linux__
#include "./_external/backends/gpu_platform_vulkan_linux_bakcend.hpp"
#endif