#pragma once

#if __PREPROCESS
// clang-format off
@include <stdlib.h>
@include <string.h>
@include <stdio.h>
@include <limits.h>
@include <math.h>
@include "sqlite3.h"
// clang-format on
#else
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "sqlite3.h"
#endif

#if __DEBUG

#define MEM_LEAK_DETECTION 1
#define TOKEN_TYPE_SAFETY 1
#define CONTAINER_BOUND_TEST 1
#define CONTAINER_MEMORY_TEST 1
#define DATABASE_BOUND_TEST 1
#define DATABASE_DEBUG 1
#define MATH_NORMALIZATION_TEST 1
#define SCENE_BOUND_TEST 1
#define GPU_DEBUG 1
#define GPU_BOUND_TEST 1
#define RENDER_BOUND_TEST 1
#define COLLIDER_BOUND_TEST 1
#define ASSET_COMPILER_BOUND_TEST 1
#define STANDARD_ALLOCATION_BOUND_TEST 1
#define USE_OPTICK 0

#elif __RELEASE

#define MEM_LEAK_DETECTION 0
#define TOKEN_TYPE_SAFETY 0
#define CONTAINER_BOUND_TEST 0
#define CONTAINER_MEMORY_TEST 0
#define DATABASE_BOUND_TEST 0
#define DATABASE_DEBUG 0
#define MATH_NORMALIZATION_TEST 0
#define SCENE_BOUND_TEST 0
#define GPU_DEBUG 0
#define GPU_BOUND_TEST 0
#define RENDER_BOUND_TEST 0
#define COLLIDER_BOUND_TEST 0
#define ASSET_COMPILER_BOUND_TEST 0
#define STANDARD_ALLOCATION_BOUND_TEST 0
#define USE_OPTICK 0

#endif