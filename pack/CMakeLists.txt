project(config_app LANGUAGES CXX)
project(config_app LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")

set(CONFIG_APP_SRC
    main.cpp
    ../solutions/usb_cam/application/z_base/folderfile.cpp
    ../solutions/usb_cam/application/z_base/fileutils.cpp
    ../solutions/usb_cam/application/_aes/aescrypt.cpp
    ../solutions/usb_cam/application/_aes/aes_ecb.c
    ../solutions/usb_cam/application/_aes/aes_core.c
    ../solutions/usb_cam/application/_base/config_vars.c
    ../solutions/usb_cam/application/_base/upgradebase.cpp
    )

add_executable(config_app ${CONFIG_APP_SRC})
set_target_properties(config_app
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./")
target_link_libraries(config_app -lpthread -ldl)
include_directories(
    ../solutions/usb_cam/application/FaceEngine
    ../solutions/usb_cam/application/_base
    ../solutions/usb_cam/application/_aes
    ../solutions/usb_cam/application/_driver
    ../solutions/usb_cam/application/_zstd/lib
    ../solutions/usb_cam/application/_zstd/lib/common
    ../solutions/usb_cam/application/_zstd/lib/decompress
    )

add_definitions(-D_PACK_OTA_=1)
add_definitions(-D_PACK_FIRM_=1)