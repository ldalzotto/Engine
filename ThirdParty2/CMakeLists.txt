
include("../_cmake/functions.cmake")

add_library(VULKAN INTERFACE)
target_include_directories(VULKAN INTERFACE "./ThirdParty/vulkan/include")
target_link_name(VULKAN INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan/lib" vulkan-1 false true)

if (IS_DEBUG)
    set(GLSLANG_include_path "./ThirdParty/glslang/debug/include")
    set(GLSLANG_lib_path ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/debug/lib)
else ()
    set(GLSLANG_include_path "./ThirdParty/glslang/release/include")
    set(GLSLANG_lib_path ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslang/release/lib)
endif ()

add_library(GLSLANG INTERFACE)
target_include_directories(GLSLANG INTERFACE ${GLSLANG_include_path})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} HLSL ${IS_DEBUG} ${PARAM_STATIC})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} glslang ${IS_DEBUG} ${PARAM_STATIC})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} MachineIndependent ${IS_DEBUG} ${PARAM_STATIC})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} OSDependent ${IS_DEBUG} ${PARAM_STATIC})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} OGLCompiler ${IS_DEBUG} ${PARAM_STATIC})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} GenericCodeGen ${IS_DEBUG} ${PARAM_STATIC})
target_link_name(GLSLANG INTERFACE ${GLSLANG_lib_path} SPIRV ${IS_DEBUG} ${PARAM_STATIC})


add_library(SQLite INTERFACE)
target_include_directories(SQLite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/sqlite/release/include")
target_link_name(SQLite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/sqlite/release/lib" SQLite false ${PARAM_STATIC})


add_library(STB_IMAGE INTERFACE)
target_include_directories(STB_IMAGE INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb_image/public/")


add_library(IMGUI STATIC ./ThirdParty/imgui/imgui.cpp ./ThirdParty/imgui/imgui_draw.cpp ./ThirdParty/imgui/imgui_demo.cpp ./ThirdParty/imgui/imgui_tables.cpp ./ThirdParty/imgui/imgui_widgets.cpp ./ThirdParty/imgui/backends/imgui_impl_vulkan.cpp)
target_include_directories(IMGUI PUBLIC ./ThirdParty/imgui)
target_link_libraries(IMGUI PUBLIC VULKAN)

add_library(V8_HEADERS INTERFACE)
target_include_directories(V8_HEADERS INTERFACE ./ThirdParty/v8_headers)