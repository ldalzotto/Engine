
if (IS_DEBUG)
    add_compile_definitions(MEM_LEAK_DETECTION=1)
    add_compile_definitions(TOKEN_TYPE_SAFETY=1)
    add_compile_definitions(CONTAINER_BOUND_TEST=1)
    add_compile_definitions(CONTAINER_MEMORY_TEST=1)
    add_compile_definitions(DATABASE_BOUND_TEST=1)
    add_compile_definitions(DATABASE_DEBUG=1)
    add_compile_definitions(MATH_NORMALIZATION_TEST=1)
    add_compile_definitions(SCENE_BOUND_TEST=1)
    add_compile_definitions(GPU_DEBUG=1)
    add_compile_definitions(GPU_BOUND_TEST=1)
    add_compile_definitions(RENDER_BOUND_TEST=1)
    add_compile_definitions(COLLIDER_BOUND_TEST=1)
    add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=1)
    add_compile_definitions(USE_OPTICK=0)
else ()
    add_compile_definitions(MEM_LEAK_DETECTION=0)
    add_compile_definitions(TOKEN_TYPE_SAFETY=0)
    add_compile_definitions(CONTAINER_BOUND_TEST=0)
    add_compile_definitions(CONTAINER_MEMORY_TEST=0)
    add_compile_definitions(DATABASE_BOUND_TEST=0)
    add_compile_definitions(DATABASE_DEBUG=0)
    add_compile_definitions(MATH_BOUND_TEST=0)
    add_compile_definitions(SCENE_BOUND_TEST=0)
    add_compile_definitions(GPU_DEBUG=0)
    add_compile_definitions(GPU_BOUND_TEST=0)
    add_compile_definitions(RENDER_BOUND_TEST=0)
    add_compile_definitions(COLLIDER_BOUND_TEST=0)
    add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=0)
    add_compile_definitions(USE_OPTICK=0)
endif ()

add_library(Common2 INTERFACE)
target_include_directories(Common2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Common2)
target_link_libraries(Common2 INTERFACE SQLite)

add_library(AssetDatabase INTERFACE)
target_include_directories(AssetDatabase INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AssetDatabase)
target_link_libraries(AssetDatabase INTERFACE Common2)

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

add_library(Collision INTERFACE)
target_include_directories(Collision INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Collision)
target_link_libraries(Collision INTERFACE Math2)

add_library(Render INTERFACE)
target_include_directories(Render INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Render)
target_link_libraries(Render INTERFACE GPU)

add_library(Scene2 INTERFACE)
target_include_directories(Scene2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2)
target_link_libraries(Scene2 INTERFACE Math2)

add_library(Scene2Middleware INTERFACE)
target_include_directories(Scene2Middleware INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2Middleware)
target_link_libraries(Scene2Middleware INTERFACE Scene2)
target_link_libraries(Scene2Middleware INTERFACE Collision)
target_link_libraries(Scene2Middleware INTERFACE Render)
target_link_libraries(Scene2Middleware INTERFACE AssetDatabase)

add_library(Engine INTERFACE)
target_include_directories(Engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/_Engine)
target_link_libraries(Engine INTERFACE Scene2Middleware)


add_library(Test_Renderdoc INTERFACE)
target_include_directories(Test_Renderdoc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Test/Renderdoc)


add_library(Test_ShaderCompile INTERFACE)
target_include_directories(Test_ShaderCompile INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Test/ShaderCompile)
target_link_libraries(Test_ShaderCompile INTERFACE GLSLANG)

add_executable(Common2Test ${CMAKE_CURRENT_SOURCE_DIR}/Common2/test/common2_test.cpp)
target_link_libraries(Common2Test PUBLIC Common2)
target_compile_definitions(Common2Test PUBLIC ASSET_FOLDER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Common2/test/.asset/")


add_executable(AssetDatabaseTest ${CMAKE_CURRENT_SOURCE_DIR}/AssetDatabase/test/asset_database_test.cpp)
target_link_libraries(AssetDatabaseTest PUBLIC AssetDatabase)
target_compile_definitions(AssetDatabaseTest PUBLIC ASSET_FOLDER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/AssetDatabase/test/.asset/")

add_executable(GPUTest ${CMAKE_CURRENT_SOURCE_DIR}/GPU/test/gpu_test.cpp)
target_link_libraries(GPUTest PUBLIC GPU)
target_link_libraries(GPUTest PUBLIC Test_Renderdoc)
target_link_libraries(GPUTest PUBLIC Test_ShaderCompile)

add_executable(Math2Test ${CMAKE_CURRENT_SOURCE_DIR}/Math2/test/math2_test.cpp)
target_link_libraries(Math2Test PUBLIC Math2)

add_executable(CollisionTest ${CMAKE_CURRENT_SOURCE_DIR}/Collision/test/collision2_test.cpp)
target_link_libraries(CollisionTest PUBLIC Collision)

add_executable(Scene2Test ${CMAKE_CURRENT_SOURCE_DIR}/Scene2/test/scene2_test.cpp)
target_link_libraries(Scene2Test PUBLIC Scene2)

add_executable(Scene2MiddlewareTest ${CMAKE_CURRENT_SOURCE_DIR}/Scene2Middleware/test/scene2_middleware_test.cpp)
target_link_libraries(Scene2MiddlewareTest PUBLIC Scene2Middleware)
target_link_libraries(Scene2MiddlewareTest PUBLIC Test_ShaderCompile)

add_executable(RenderTest ${CMAKE_CURRENT_SOURCE_DIR}/Render/test/render_test.cpp)
target_link_libraries(RenderTest PUBLIC Render)
target_link_libraries(RenderTest PUBLIC Test_Renderdoc)
target_link_libraries(RenderTest PUBLIC Test_ShaderCompile)

add_executable(WindowTest ${CMAKE_CURRENT_SOURCE_DIR}/Window/test/window_test.cpp)
target_link_libraries(WindowTest PUBLIC Window)

add_executable(Sandbox ${CMAKE_CURRENT_SOURCE_DIR}/_Sandbox/sandbox.cpp)
target_link_libraries(Sandbox PUBLIC Engine)
target_compile_definitions(Sandbox PUBLIC ASSET_FOLDER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/_Sandbox/.asset/")

add_custom_target(
        Test_ALL
        COMMAND Common2Test
        COMMAND AssetDatabaseTest
        COMMAND Math2Test
        COMMAND WindowTest
        COMMAND CollisionTest
        COMMAND RenderTest
        COMMAND Scene2Test
        COMMAND GPUTest
        COMMAND Scene2MiddlewareTest
        COMMAND Sandbox
)