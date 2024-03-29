IF (NOT WIN32)
    message(FATAL_ERROR "The device is available only on Windows")
ENDIF()

find_package(IWear REQUIRED)

if(NOT DEFINED FREEEMG_SDK_DIR)
    if(DEFINED ENV{FREEEMG_SDK_DIR})
        set(FREEEMG_SDK_DIR $ENV{FREEEMG_SDK_DIR})
    else()
        message(FATAL_ERROR "Cannot find FREEEMG_SDK_DIR!")
    endif()
endif()

# Register DLLs
get_filename_component(DOT_NET_INSTALL_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full;InstallPath]" ABSOLUTE)
if(DOT_NET_INSTALL_PATH STREQUAL "/registry")
    message(FATAL_ERROR "Unable to locate .NET Framework v4 installation!")
endif()
cmake_path(CONVERT ${DOT_NET_INSTALL_PATH} TO_NATIVE_PATH_LIST DOT_NET_INSTALL_NATIVE_PATH)
cmake_path(CONVERT ${CMAKE_CURRENT_SOURCE_DIR} TO_NATIVE_PATH_LIST CMAKE_CURRENT_SOURCE_DIR_NATIVE)

add_custom_target(
    PreBuild
    COMMAND cmd /k "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\build_scripts\\PreBuild.bat" "$<CONFIG>" "${DOT_NET_INSTALL_NATIVE_PATH}" "${FREEEMG_SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}" "x64"
)

# Compile the plugin by default
yarp_prepare_plugin(freeemg TYPE wearable::devices::FreeEmgDevice
                    INCLUDE include/FreeEmgDevice.h
                    CATEGORY device
                    ADVANCED
                    DEFAULT ON
                    )

yarp_add_plugin(freeemg_device src/FreeEmgDevice.cpp src/BioDAQUtils.cpp
                include/FreeEmgDevice.h)

target_link_libraries(freeemg_device
                    PUBLIC
                    IWear::IWear YARP::YARP_dev YARP::YARP_os
                    )

target_include_directories(freeemg_device 
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE include_private)

# Run the DLLs registering script before build
add_dependencies(freeemg_device PreBuild)

# Install the device
yarp_install(TARGETS freeemg_device
            COMPONENT runtime
            LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
            ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
            YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})


# Install configuration files and applications
add_subdirectory(conf)
add_subdirectory(applications)
