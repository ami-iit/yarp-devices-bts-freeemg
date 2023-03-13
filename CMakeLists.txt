cmake_minimum_required(VERSION 3.5)

set(GLOBAL_PROJECT_VERSION "0.0.1")

project(FreeEmg VERSION ${GLOBAL_PROJECT_VERSION}
                    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Export all symbols in Windows
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Libraries type
if(MSVC)
    option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" OFF)
else()
    option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)
endif()

# Control where binaries and libraries are placed in the build folder.
# This simplifies tests running in Windows.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(YARP 3.7.2 QUIET)
set(YARP_FORCE_DYNAMIC_PLUGINS ON)

add_subdirectory(devices)
