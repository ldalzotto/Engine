
add_library(EngineDLL SHARED ./engine_dll/engine_dll.cpp)
target_link_libraries(EngineDLL Engine)

add_library(AssetCompilerDLL SHARED ./assetcompiler_dll/assetcompiler_dll.cpp)
target_link_libraries(AssetCompilerDLL AssetCompiler)