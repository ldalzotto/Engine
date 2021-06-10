
add_library(EngineFull SHARED ./Core/engine_full.cpp)
target_link_libraries(EngineFull PUBLIC Engine)

add_library(AssetCompiler_API SHARED Core/asset_compiler.cpp)
target_link_libraries(AssetCompiler_API PUBLIC AssetCompiler)

add_library(V8Binding SHARED ./v8_binding/v8_binding.cpp)
target_link_libraries(V8Binding PUBLIC Common2)
target_link_libraries(V8Binding PUBLIC V8_HEADERS)
