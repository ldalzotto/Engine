#pragma once

#if _WIN32
#include "_external/Syscall/backends/syscall_window_backend.hpp"
#else
#include "_external/Syscall/backends/syscall_linux_backend.hpp"
#endif

// #include "_external/Database/backends/database_sqlite3_backend.hpp"