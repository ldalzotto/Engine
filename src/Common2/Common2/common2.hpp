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

#ifndef __SQLITE_ENABLED
#define __SQLITE_ENABLED 1
#endif

#ifndef __NATIVE_WINDOW_ENABLED
#define __NATIVE_WINDOW_ENABLED 1
#endif

#ifndef __SOCKET_ENABLED
#define __SOCKET_ENABLED 0
#endif

#include "./Types/types.hpp"
#include "./Macros/macros.hpp"
#include "./Memory/limits.hpp"

#include "./Include/platform_include.hpp"

#include "./Functional/equality.hpp"

#include "./Functional/assert.hpp"

#include "./Clock/clock.hpp"
#include "./Thread/mutex.hpp"
#include "./Thread/barrier.hpp"

#include "./Memory/token.hpp"
#include "./Memory/memory.hpp"
#include "./Memory/slice.hpp"
#include "./Memory/span.hpp"

#include "./Thread/thread.hpp"

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

#if __SQLITE_ENABLED
#include "sqlite3.h"
#include "./Database/database.hpp"
#endif

#if __NATIVE_WINDOW_ENABLED
#include "./AppNativeEvent/app_native_event.hpp"
#include "./Window/window.hpp"
#endif

#if __SOCKET_ENABLED
#include "./Socket/socket_v2.hpp"
#endif

#undef __SQLITE_ENABLED
#undef __NATIVE_WINDOW_ENABLED