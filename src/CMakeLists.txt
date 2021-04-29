
add_library(Common2 INTERFACE)
target_include_directories(Common2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Common2)
target_link_libraries(Common2 INTERFACE SQLite)
target_link_libraries(Common2 INTERFACE SHA1)

add_library(AssetDatabase INTERFACE)
target_include_directories(AssetDatabase INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AssetDatabase)
target_link_libraries(AssetDatabase INTERFACE Common2)

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

add_library(AssetResource INTERFACE)
target_include_directories(AssetResource INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AssetResource)
target_link_libraries(AssetResource INTERFACE Render)
target_link_libraries(AssetResource INTERFACE AssetDatabase)

add_library(AssetCompiler INTERFACE)
target_include_directories(AssetCompiler INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AssetCompiler)
target_link_libraries(AssetCompiler INTERFACE AssetResource)
target_link_libraries(AssetCompiler INTERFACE GLSLANG)
target_link_libraries(AssetCompiler INTERFACE STB_IMAGE)

add_library(Scene2 INTERFACE)
target_include_directories(Scene2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2)
target_link_libraries(Scene2 INTERFACE Math2)

add_library(Scene2Middleware INTERFACE)
target_include_directories(Scene2Middleware INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Scene2Middleware)
target_link_libraries(Scene2Middleware INTERFACE Scene2)
target_link_libraries(Scene2Middleware INTERFACE Collision)
target_link_libraries(Scene2Middleware INTERFACE Render)
target_link_libraries(Scene2Middleware INTERFACE AssetResource)

add_library(Engine INTERFACE)
target_include_directories(Engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/_Engine)
target_link_libraries(Engine INTERFACE Scene2Middleware)

add_library(Test_Renderdoc INTERFACE)
target_include_directories(Test_Renderdoc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Test/Renderdoc)


add_library(Test_AssetDatabase INTERFACE)
target_include_directories(Test_AssetDatabase INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Test/AssetDatabase)

add_executable(Common2Test ${CMAKE_CURRENT_SOURCE_DIR}/Common2/test/common2_test.cpp)
target_link_libraries(Common2Test PUBLIC Common2)
target_compile_definitions(Common2Test PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/Common2/")


add_executable(AssetDatabaseTest ${CMAKE_CURRENT_SOURCE_DIR}/AssetDatabase/test/asset_database_test.cpp)
target_link_libraries(AssetDatabaseTest PUBLIC AssetDatabase)
target_link_libraries(AssetDatabaseTest PUBLIC Test_AssetDatabase)
target_compile_definitions(AssetDatabaseTest PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/AssetDatabase/")

add_executable(AssetCompilerTest ${CMAKE_CURRENT_SOURCE_DIR}/AssetCompiler/test/asset_compiler_test.cpp)
target_link_libraries(AssetCompilerTest PUBLIC AssetCompiler)
target_link_libraries(AssetCompilerTest PUBLIC Test_AssetDatabase)
target_compile_definitions(AssetCompilerTest PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/AssetCompiler/")

add_executable(GPUTest ${CMAKE_CURRENT_SOURCE_DIR}/GPU/test/gpu_test.cpp)
target_link_libraries(GPUTest PUBLIC GPU)
target_link_libraries(GPUTest PUBLIC Test_Renderdoc)
target_link_libraries(GPUTest PUBLIC AssetCompiler)

add_executable(Math2Test ${CMAKE_CURRENT_SOURCE_DIR}/Math2/test/math2_test.cpp)
target_link_libraries(Math2Test PUBLIC Math2)

add_executable(CollisionTest ${CMAKE_CURRENT_SOURCE_DIR}/Collision/test/collision2_test.cpp)
target_link_libraries(CollisionTest PUBLIC Collision)

add_executable(RenderTest ${CMAKE_CURRENT_SOURCE_DIR}/Render/test/render_test.cpp)
target_link_libraries(RenderTest PUBLIC Render)
target_link_libraries(RenderTest PUBLIC Test_Renderdoc)
target_link_libraries(RenderTest PUBLIC AssetCompiler)

add_executable(AssetResourceTest ${CMAKE_CURRENT_SOURCE_DIR}/AssetResource/test/asset_ressource_test.cpp)
target_link_libraries(AssetResourceTest PUBLIC AssetResource)
target_link_libraries(AssetResourceTest PUBLIC AssetCompiler)
target_link_libraries(AssetResourceTest PUBLIC Test_AssetDatabase)
target_compile_definitions(AssetResourceTest PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/AssetResource/")

add_executable(Scene2Test ${CMAKE_CURRENT_SOURCE_DIR}/Scene2/test/scene2_test.cpp)
target_link_libraries(Scene2Test PUBLIC Scene2)

add_executable(Scene2MiddlewareTest ${CMAKE_CURRENT_SOURCE_DIR}/Scene2Middleware/test/scene2_middleware_test.cpp)
target_link_libraries(Scene2MiddlewareTest PUBLIC Scene2Middleware)
target_link_libraries(Scene2MiddlewareTest PUBLIC AssetCompiler)
target_link_libraries(Scene2MiddlewareTest PUBLIC Test_AssetDatabase)
target_compile_definitions(Scene2MiddlewareTest PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/Scene2Middleware/")

add_executable(Sandbox ${CMAKE_CURRENT_SOURCE_DIR}/_Sandbox/sandbox.cpp)
target_link_libraries(Sandbox PUBLIC Engine)
target_compile_definitions(Sandbox PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/sandbox/")
target_link_libraries(Sandbox PUBLIC AssetCompiler)


