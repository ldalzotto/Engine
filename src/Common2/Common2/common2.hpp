#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "sqlite3.h"

#include "./Types/types.hpp"
#include "./Macros/macros.hpp"
#include "./Memory/limits.hpp"

#include "./Include/platform_include.hpp"

#include "./Clock/clock.hpp"
#include "./Thread/thread.hpp"

#include "./Functional/assert.hpp"

#include "./Memory/token.hpp"
#include "./Memory/memory.hpp"
#include "./Memory/slice.hpp"
#include "./Memory/span.hpp"

#include "./Functional/hash.hpp"

#include "./Functional/sort.hpp"

#include "./Container/vector.hpp"
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

#include "./File/file.hpp"

#include "./Serialization/json.hpp"
#include "./Serialization/binary.hpp"
#include "./Serialization/types.hpp"

#include "./Functional/container_iteration.hpp"

#include "./Database/database.hpp"

#include "./AppNativeEvent/app_native_event.hpp"
#include "./Window/window.hpp"