add_executable(AssetCompilerExe ./AssetCompilerExe/asset_compiler_exe.cpp)
target_link_libraries(AssetCompilerExe PUBLIC AssetCompiler)

INSTALL(TARGETS AssetCompilerExe)
ADD_CUSTOM_TARGET(install.AssetCompilerExe
        ${CMAKE_COMMAND}
        -DBUILD_TYPE=${CMAKE_BUILD_TYPE}
        -P ${CMAKE_BINARY_DIR}/cmake_install.cmake)
ADD_DEPENDENCIES(install.AssetCompilerExe AssetCompilerExe)