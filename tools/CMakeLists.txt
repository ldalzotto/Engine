
include("../_cmake/functions.cmake")

set(QT_PATH $ENV{QT_SDK})

add_library(QtInterface STATIC ./QtInterface/qt_interface.cpp)
target_include_directories(QtInterface PRIVATE "${QT_PATH}/include")
target_include_directories(QtInterface INTERFACE ./QtInterface)
target_link_libraries(QtInterface Common2)
target_link_name(QtInterface "" ${QT_PATH}/lib Qt6Core false ${PARAM_STATIC})
target_link_name(QtInterface "" ${QT_PATH}/lib Qt6Widgets false ${PARAM_STATIC})
target_link_name(QtInterface "" ${QT_PATH}/lib Qt6Gui false ${PARAM_STATIC})
target_link_name(QtInterface "" ${QT_PATH}/lib Qt6UiTools false ${PARAM_STATIC})

add_library(QtCommon INTERFACE)
target_include_directories(QtCommon INTERFACE "${QT_PATH}/include")
target_include_directories(QtCommon INTERFACE ./QTCommon)
target_link_name(QtCommon INTERFACE ${QT_PATH}/lib Qt6Core false ${PARAM_STATIC})
target_link_name(QtCommon INTERFACE ${QT_PATH}/lib Qt6Widgets false ${PARAM_STATIC})
target_link_name(QtCommon INTERFACE ${QT_PATH}/lib Qt6Gui false ${PARAM_STATIC})
target_link_name(QtCommon INTERFACE ${QT_PATH}/lib Qt6UiTools false ${PARAM_STATIC})

add_library(MaterialViewer INTERFACE)
target_include_directories(MaterialViewer INTERFACE ./MaterialViewer)
target_link_libraries(MaterialViewer INTERFACE Engine)
target_link_libraries(MaterialViewer INTERFACE AssetCompiler)
target_link_libraries(MaterialViewer INTERFACE QtInterface)

add_library(AssetCompilerEditor INTERFACE)
target_include_directories(AssetCompilerEditor INTERFACE ./AssetCompiler)
target_link_libraries(AssetCompilerEditor INTERFACE Engine)
target_link_libraries(AssetCompilerEditor INTERFACE AssetCompiler)
target_link_libraries(AssetCompilerEditor INTERFACE QtCommon)

add_library(ImguiIntegration INTERFACE)
target_include_directories(ImguiIntegration INTERFACE ./ImguiIntegration)
target_link_libraries(ImguiIntegration INTERFACE Engine)
target_link_libraries(ImguiIntegration INTERFACE IMGUI)


add_executable(MaterialViewerExec MaterialViewer/materialviewer_standalone.cpp)
target_link_libraries(MaterialViewerExec MaterialViewer)

add_executable(MaterialViewerTest ./MaterialViewer/test/material_viewer_test.cpp)
target_link_libraries(MaterialViewerTest PUBLIC MaterialViewer)
target_compile_definitions(MaterialViewerTest PUBLIC ASSET_FOLDER_PATH="${ASSET_FOLDER_VAR}/MaterialViewer/")


add_executable(AssetCompilerEditorExec ./AssetCompiler/assetcompiler_standalone.cpp)
target_link_libraries(AssetCompilerEditorExec PUBLIC AssetCompilerEditor)
target_compile_definitions(AssetCompilerEditorExec PUBLIC ASSET_FOLDER_ROOT="${ASSET_FOLDER_VAR}/")

add_executable(AssetCompilerEditorTest ./AssetCompiler/test/assetcompiler_tool_test.cpp)
target_link_libraries(AssetCompilerEditorTest PUBLIC AssetCompilerEditor)
target_compile_definitions(AssetCompilerEditorTest PUBLIC ASSET_FOLDER_ROOT="${ASSET_FOLDER_VAR}/AssetCompilerTool")

