#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#if __DEBUG

#define __MEMLEAK 1
#define __TOKEN 1

#elif __RELEASE

#define __MEMLEAK 0
#define __TOKEN 0

#endif

#ifndef __LIB
#define __LIB 0
#endif

// GLOBAL_STATIC is a shared variable across static lib
#if __LIB == 1
#define GLOBAL_STATIC extern
#define GLOBAL_STATIC_INIT(code)
#else
#define GLOBAL_STATIC
#define GLOBAL_STATIC_INIT(code) = code
#endif

#include "./Types/types.hpp"
#include "./Macros/macros.hpp"
#include "./Memory/limits.hpp"
#include "./Functional/binary_operators.hpp"

#include "./_external/Syscall/syscall_interface.hpp"
#include "./_external/Database/database_interface.hpp"

#include "./Functional/equality.hpp"

#include "./Functional/assert.hpp"

#include "./Thread/mutex.hpp"
#include "./Thread/barrier.hpp"

#include "./Memory/token.hpp"
#include "./Memory/memory.hpp"
#include "./Memory/slice.hpp"
#include "./Memory/span.hpp"

#include "./Thread/thread.hpp"
#include "./Clock/clock.hpp"

#include "./Functional/encode.hpp"
#include "./Functional/hash.hpp"

#include "./Functional/sort.hpp"
#include "./Functional/slice_algorithm.hpp"

#include "./Container/Interface/ivector.hpp"
#include "./Container/Interface/ipool.hpp"
#include "./Container/Interface/iheap.hpp"
#include "./Container/Interface/istring.hpp"

#include "./Container/vector.hpp"
#include "./Container/vector_slice.hpp"
#include "./Container/hashmap.hpp"
#include "./Container/pool.hpp"
#include "./Container/varying_slice.hpp"
#include "./Container/varying_vector.hpp"
#include "./Container/string.hpp"

#include "./Container/Specialization/vector_of_vector.hpp"
#include "./Container/Specialization/pool_of_vector.hpp"
#include "./Container/Specialization/pool_indexed.hpp"
#include "./Container/Specialization/pool_hashed_counted.hpp"

#include "./Container/heap.hpp"
#include "./Container/tree.hpp"

#include "./Container/Specialization/heap_memory.hpp"

#include "./Functional/string_functions.hpp"
#include "./Functional/path.hpp"

#include "./File/file.hpp"
#include "./File/shared_lib_loader.hpp"

#include "./Serialization/json.hpp"
#include "./Serialization/binary.hpp"
#include "./Serialization/types.hpp"

#include "./Database/database.hpp"

#include "./AppNativeEvent/app_native_event.hpp"
#include "./Window/window.hpp"

#include "./Socket/socket_v2.hpp"