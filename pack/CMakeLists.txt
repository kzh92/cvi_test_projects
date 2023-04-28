project(config_app LANGUAGES CXX)
project(config_app LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result")

set(CONFIG_APP_SRC
    main.cpp
    )

add_executable(config_app ${CONFIG_APP_SRC})
set_target_properties(config_app
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./")
target_link_libraries(config_app -lpthread -ldl)
include_directories(
    ../solutions/usb_cam/application/FaceEngine
    ../solutions/usb_cam/application/_base
    )
