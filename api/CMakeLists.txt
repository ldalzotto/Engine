
add_library(EngineFull SHARED ./EngineFull/engine_full.cpp)
target_link_libraries(EngineFull PUBLIC Engine)

# add_library(NodeJsBinding SHARED ./nodejs_binding/nodejs_binding.cpp)
# target_link_libraries(NodeJsBinding PUBLIC Common2)