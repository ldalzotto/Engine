cmake_minimum_required(VERSION 3.12)

add_definitions(/std:c++17)
enable_language(CXX)

set(QT_PATH "E:/qt/6.0.2/msvc2019_64/")

set(QT_LIB ${QT_PATH}lib/Qt6Core.lib ${QT_PATH}lib/Qt6Cored.lib ${QT_PATH}lib/Qt6Widgets.lib ${QT_PATH}lib/Qt6Widgetsd.lib ${QT_PATH}lib/Qt6Gui.lib ${QT_PATH}lib/Qt6Guid.lib)

add_library(QtCommon INTERFACE)
target_include_directories(QtCommon INTERFACE "${QT_PATH}include")
target_include_directories(QtCommon INTERFACE ./QTCommon)
target_link_libraries(QtCommon INTERFACE ${QT_LIB})


add_library(MaterialViewer INTERFACE)
target_include_directories(MaterialViewer INTERFACE ./MaterialViewer)
target_link_libraries(MaterialViewer INTERFACE Engine)
target_link_libraries(MaterialViewer INTERFACE AssetCompiler)
target_link_libraries(MaterialViewer INTERFACE QtCommon)

add_library(AssetCompilerEditor INTERFACE)
target_include_directories(AssetCompilerEditor INTERFACE ./AssetCompiler)
target_link_libraries(AssetCompilerEditor INTERFACE Engine)
target_link_libraries(AssetCompilerEditor INTERFACE AssetCompiler)
target_link_libraries(AssetCompilerEditor INTERFACE QtCommon)

add_executable(MaterialViewerExec MaterialViewer/materialviewer_standalone.cpp)
target_link_libraries(MaterialViewerExec PUBLIC MaterialViewer)

add_executable(MaterialViewerTest ./MaterialViewer/test/material_viewer_test.cpp)
target_link_libraries(MaterialViewerTest PUBLIC MaterialViewer)
target_compile_definitions(MaterialViewerTest PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/MaterialViewer/")

add_executable(AssetCompilerEditorExec ./AssetCompiler/assetcompiler_standalone.cpp)
target_link_libraries(AssetCompilerEditorExec PUBLIC AssetCompilerEditor)