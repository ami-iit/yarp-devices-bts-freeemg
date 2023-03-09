IF (NOT WIN32)
    message(FATAL_ERROR "The device is available only on Windows")
ENDIF()

find_package(IWear REQUIRED)

if(NOT DEFINED ENV{FreeEMG_SDK_DIR})
    message(FATAL_ERROR "Cannot find environment variable FeeEMG_SDK_DIR")
endif()

# Register DLLs
GET_FILENAME_COMPONENT(DOT_NET_INSTALL_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full;InstallPath]" ABSOLUTE)
message(WARNING "DOT_NET_INSTALL_PATH ${DOT_NET_INSTALL_PATH}")
cmake_path(CONVERT ${DOT_NET_INSTALL_PATH} TO_NATIVE_PATH_LIST DOT_NET_INSTALL_NATIVE_PATH)
cmake_path(CONVERT ${CMAKE_CURRENT_SOURCE_DIR} TO_NATIVE_PATH_LIST CMAKE_CURRENT_SOURCE_DIR_NATIVE)

add_custom_target(
    PreBuild
    COMMAND cmd /k "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\build_scripts\\PreBuild.bat" "$<CONFIG>" "${DOT_NET_INSTALL_NATIVE_PATH}" "$ENV{FreeEMG_SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}" "x64"
)

# Compile the plugin by default
yarp_prepare_plugin(free_emg  TYPE wearable::devices::FreeEmgDevice
                    INCLUDE include/FreeEmgDevice.h
                    CATEGORY device
                    ADVANCED
                    DEFAULT ON
                    )

yarp_add_plugin(free_emg_device src/FreeEmgDevice.cpp src/BioDAQUtils.cpp
                include/FreeEmgDevice.h)

target_link_libraries(free_emg_device
                    PUBLIC
                    IWear::IWear YARP::YARP_dev YARP::YARP_os
                    )

target_include_directories(free_emg_device 
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE include_private)

# Run the DLLs registering script before build
add_dependencies(free_emg_device PreBuild)

# Install the device
yarp_install(TARGETS free_emg_device
            COMPONENT runtime
            LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
            ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
            YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})


# Install configuration files and applications
add_subdirectory(conf)
add_subdirectory(applications)
