#pragma once

#if _WIN32
#include "_external/Syscall/backends/syscall_window_backend.hpp"
#else
#include "_external/Syscall/backends/syscall_linux_backend.hpp"
#endif