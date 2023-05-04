cmake_minimum_required(VERSION 2.8)

#include(../config.cmake)

project(z_ufmaker_console LANGUAGES CXX)
project(z_ufmaker_console LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-unused-result")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unused-result")

set(UFMAKER_SRC
    main.cpp
    ../solutions/usb_cam/application/z_base/folderfile.cpp
    ../solutions/usb_cam/application/z_base/fileutils.cpp
#    ../z_base/aes.cpp
    ../solutions/usb_cam/application/_aes/aescrypt.cpp
    ../solutions/usb_cam/application/_aes/aes_core.c
    ../solutions/usb_cam/application/_aes/aes_ecb.c
#    ../z_base/sha1.cpp
#    ../solutions/usb_cam/application/_base/common_types.c
    ../solutions/usb_cam/application/_base/upgradebase.cpp
    )

add_executable(z_ufmaker_console ${UFMAKER_SRC})
set_target_properties(z_ufmaker_console
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./")
target_link_libraries(z_ufmaker_console -lpthread -ldl)
include_directories(
    ../solutions/usb_cam/application/FaceEngine
    ../solutions/usb_cam/application/FaceEngine/AEngineLib
    ../solutions/usb_cam/application/FaceEngine/HEngineLib
#    ../solutions/usb_cam/application/_driver
    ../solutions/usb_cam/application/_base
#    ../solutions/usb_cam/application/_uart
#    ../solutions/usb_cam/application/_keygen
#    ../solutions/usb_cam/application/_des
#    ../solutions/usb_cam/application/_des/openssl
    ../solutions/usb_cam/application/_aes
#    ../solutions/usb_cam/application/_uart
    )

add_definitions(-D_PACK_OTA_=1)