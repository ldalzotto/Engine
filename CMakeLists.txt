cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(EngineCPP VERSION 0.0.1)

enable_language(CXX)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-missing-braces)
endif ()

add_compile_definitions(ASSETPATH_DEV=1)

# OR ("${CMAKE_BUILD_TYPE_INIT}" STREQUAL "Debug")
if (("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    set(IS_DEBUG true)
   else()
    set(IS_DEBUG false)
endif ()

# message("${IS_DEBUG}")

if (IS_DEBUG)
    add_compile_definitions(TOKEN_TYPE_SAFETY=1)
    add_compile_definitions(CONTAINER_BOUND_TEST=1)
    add_compile_definitions(CONTAINER_MEMORY_TEST=1)
    add_compile_definitions(MATH_NORMALIZATION_TEST=1)
    add_compile_definitions(SCENE_BOUND_TEST=1)
    add_compile_definitions(GPU_DEBUG=1)
    add_compile_definitions(GPU_BOUND_TEST=1)
    add_compile_definitions(RENDER_BOUND_TEST=1)
    add_compile_definitions(COLLIDER_BOUND_TEST=1)
    add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=1)
    add_compile_definitions(USE_OPTICK=0)
else ()
    add_compile_definitions(TOKEN_TYPE_SAFETY=0)
    add_compile_definitions(CONTAINER_BOUND_TEST=0)
    add_compile_definitions(CONTAINER_MEMORY_TEST=0)
    add_compile_definitions(MATH_BOUND_TEST=0)
    add_compile_definitions(SCENE_BOUND_TEST=0)
    add_compile_definitions(GPU_DEBUG=0)
    add_compile_definitions(GPU_BOUND_TEST=0)
    add_compile_definitions(RENDER_BOUND_TEST=0)
    add_compile_definitions(COLLIDER_BOUND_TEST=0)
    add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=0)
    add_compile_definitions(USE_OPTICK=0)
endif ()

add_library(GLFW INTERFACE)
target_include_directories(GLFW INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW")
target_link_libraries(GLFW INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/lib/win32/glfw3.lib")
# target_link_libraries(GLFW INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/lib/linux/glfw3.lib")

add_library(VULKAN INTERFACE)
target_include_directories(VULKAN INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan")
target_link_libraries(VULKAN INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan/lib/win32/vulkan-1.lib")
# target_link_libraries(VULKAN INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan/lib/linux/vulkan-1.lib")

add_library(GLSLANG INTERFACE)
target_include_directories(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang")

if (IS_DEBUG)
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/GenericCodeGend.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/glslangd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/HLSLd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/MachineIndependentd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/OGLCompilerd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/OSDependentd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/SPIRVd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/SPIRV-Toolsd.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/deb/SPIRV-Tools-optd.lib")
else()
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/GenericCodeGen.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/glslang.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/HLSL.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/MachineIndependent.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/OGLCompiler.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/OSDependent.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/SPIRV.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/SPIRV-Tools.lib")
target_link_libraries(GLSLANG INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/lib/win32/rel/SPIRV-Tools-opt.lib")
endif()

add_library(Common2 INTERFACE)
target_include_directories(Common2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Common2)

add_library(Window INTERFACE)
target_include_directories(Window INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Window)
target_link_libraries(Window INTERFACE GLFW)
target_link_libraries(Window INTERFACE Common2)

add_library(Math2 INTERFACE)
target_include_directories(Math2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Math2)
target_link_libraries(Math2 INTERFACE Common2)

add_library(GPU INTERFACE)
target_include_directories(GPU INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GPU)
target_link_libraries(GPU INTERFACE Common2)
target_link_libraries(GPU INTERFACE Math2)
target_link_libraries(GPU INTERFACE VULKAN)
target_link_libraries(GPU INTERFACE GLSLANG)

add_library(Collision INTERFACE)
target_include_directories(Collision INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Collision)
target_link_libraries(Collision INTERFACE Math2)

add_library(Scene2 INTERFACE)
target_include_directories(Scene2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2)
target_link_libraries(Scene2 INTERFACE Math2)

add_library(Scene2Middleware INTERFACE)
target_include_directories(Scene2Middleware INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2Middleware)
target_link_libraries(Scene2Middleware INTERFACE Scene2)
target_link_libraries(Scene2Middleware INTERFACE Collision)

add_library(Engine INTERFACE)
target_include_directories(Engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/_Engine)
target_link_libraries(Engine INTERFACE Scene2Middleware)


add_executable(Common2Test ${CMAKE_CURRENT_SOURCE_DIR}/Common2/test/common2_test.cpp)
target_link_libraries(Common2Test PUBLIC Common2)

add_executable(GPUTest ${CMAKE_CURRENT_SOURCE_DIR}/GPU/test/gpu_test.cpp)
target_link_libraries(GPUTest PUBLIC GPU)

add_executable(Math2Test ${CMAKE_CURRENT_SOURCE_DIR}/Math2/test/math2_test.cpp)
target_link_libraries(Math2Test PUBLIC Math2)

add_executable(CollisionTest ${CMAKE_CURRENT_SOURCE_DIR}/Collision/test/collision2_test.cpp)
target_link_libraries(CollisionTest PUBLIC Collision)

add_executable(Scene2Test ${CMAKE_CURRENT_SOURCE_DIR}/Scene2/test/scene2_test.cpp)
target_link_libraries(Scene2Test PUBLIC Scene2)

add_executable(Scene2MiddlewareTest ${CMAKE_CURRENT_SOURCE_DIR}/Scene2Middleware/test/scene2_middleware_test.cpp)
target_link_libraries(Scene2MiddlewareTest PUBLIC Scene2Middleware)

add_executable(WindowTest ${CMAKE_CURRENT_SOURCE_DIR}/Window/test/window_test.cpp)
target_link_libraries(WindowTest PUBLIC Window)

add_executable(Sandbox ${CMAKE_CURRENT_SOURCE_DIR}/_Sandbox/sandbox.cpp)
target_link_libraries(Sandbox PUBLIC Engine)

add_custom_target(
        Test_ALL
        COMMAND Common2Test
        COMMAND Math2Test
        COMMAND WindowTest
        COMMAND CollisionTest
        COMMAND Scene2Test
        COMMAND GPUTest
        COMMAND Scene2MiddlewareTest
        COMMAND Sandbox
)