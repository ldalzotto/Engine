cmake_minimum_required(VERSION 3.12)

project(EngineCPP VERSION 0.0.1)

enable_language(CXX)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wno-missing-braces)
endif()

add_compile_definitions(ASSETPATH_DEV=1)

if("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Debug")
	add_compile_definitions(TOKEN_TYPE_SAFETY=1)
	add_compile_definitions(CONTAINER_BOUND_TEST=1)
	add_compile_definitions(CONTAINER_MEMORY_TEST=1)
	add_compile_definitions(MATH_NORMALIZATION_TEST=1)
	add_compile_definitions(SCENE_BOUND_TEST=1)
	add_compile_definitions(RENDER_BOUND_TEST=1)
	add_compile_definitions(COLLIDER_BOUND_TEST=1)
	add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=1)
	add_compile_definitions(USE_OPTICK=0)
else()
	add_compile_definitions(TOKEN_TYPE_SAFETY=0)
	add_compile_definitions(CONTAINER_BOUND_TEST=0)
	add_compile_definitions(CONTAINER_MEMORY_TEST=0)
	add_compile_definitions(MATH_BOUND_TEST=0)
	add_compile_definitions(SCENE_BOUND_TEST=0)
	add_compile_definitions(RENDER_BOUND_TEST=0)
	add_compile_definitions(COLLIDER_BOUND_TEST=0)
	add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=0)
	add_compile_definitions(USE_OPTICK=0)
endif()


add_library(Common2 INTERFACE)
target_include_directories(Common2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Common2)

add_library(Math2 INTERFACE)
target_include_directories(Math2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Math2)
target_link_libraries(Math2 INTERFACE Common2)

add_library(Collision INTERFACE)
target_include_directories(Collision INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Collision)
target_link_libraries(Collision INTERFACE Math2)

add_library(Scene2 INTERFACE)
target_include_directories(Scene2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2)
target_link_libraries(Scene2 INTERFACE Math2)

add_executable(Common2Test ${CMAKE_CURRENT_SOURCE_DIR}/Common2/test/common2_test.cpp)
target_link_libraries(Common2Test PUBLIC Common2)

add_executable(Math2Test ${CMAKE_CURRENT_SOURCE_DIR}/Math2/test/math2_test.cpp)
target_link_libraries(Math2Test PUBLIC Math2)

add_executable(CollisionTest ${CMAKE_CURRENT_SOURCE_DIR}/Collision/test/collision2_test.cpp)
target_link_libraries(CollisionTest PUBLIC Collision)

add_executable(Scene2Test ${CMAKE_CURRENT_SOURCE_DIR}/Scene2/test/scene2_test.cpp)
target_link_libraries(Scene2Test PUBLIC Scene2)