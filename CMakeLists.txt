cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(EngineCPP VERSION 0.0.1)

enable_language(C CXX)

add_compile_options(/std:c++latest)


# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-missing-braces)
endif ()


# OR ("${CMAKE_BUILD_TYPE_INIT}" STREQUAL "Debug")
if (("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    set(IS_DEBUG true)
else()
    set(IS_DEBUG false)
endif ()

add_subdirectory(ThirdParty2)

if (IS_DEBUG)
    add_compile_definitions(__DEBUG=1)
    add_compile_definitions(__RELEASE=0)
else ()
    add_compile_definitions(__DEBUG=0)
    add_compile_definitions(__RELEASE=1)
endif ()

add_subdirectory(src)
add_subdirectory(exe)

add_executable(Preprocessed ./preprocess/test.c)
target_link_libraries(Preprocessed INTERFACE SQLite)
