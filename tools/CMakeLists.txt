cmake_minimum_required(VERSION 3.12)

add_definitions(/std:c++17)
enable_language(CXX)

set(QT_PATH "E:/qt/6.0.2/msvc2019_64/")

set(QT_LIB ${QT_PATH}lib/Qt6Core.lib ${QT_PATH}lib/Qt6Cored.lib ${QT_PATH}lib/Qt6Widgets.lib ${QT_PATH}lib/Qt6Widgetsd.lib)

add_executable(MaterialViewer ./material_viewer/main.cpp)
target_include_directories(MaterialViewer PUBLIC "${QT_PATH}include")
target_link_libraries(MaterialViewer PUBLIC ${QT_LIB})
target_link_libraries(MaterialViewer PUBLIC Engine)
target_link_libraries(MaterialViewer PUBLIC AssetCompiler)