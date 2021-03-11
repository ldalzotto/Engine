#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "sqlite3.h"

#include "./Types/types.h"
#include "./Macros/macros.h"
#include "./Memory/limits.h"

#include "./Include/platform_include.h"

#include "./Clock/clock.h"
#include "./Thread/thread.h"

#include "./Functional/assert.h"

#include "./Memory/token.h"
#include "./Memory/tokenT.hpp"
#include "./Memory/memory.h"
#include "./Memory/slice.h"
#include "./Memory/sliceT.hpp"
#include "./Memory/span.h"
#include "./Memory/spanT.hpp"

#include "./Functional/hash.h"
#include "./Functional/hashT.hpp"

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