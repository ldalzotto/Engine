#pragma once

#if _WIN32
#include "./_external/backends/gpu_platform_vulkan_win32_backend.hpp"
#elif __linux__
#include "./_external/backends/gpu_platform_vulkan_linux_bakcend.hpp"
#endif