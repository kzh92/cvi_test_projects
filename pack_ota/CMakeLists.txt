cmake_minimum_required(VERSION 2.8)

include(../config.cmake)

project(z_ufmaker_console LANGUAGES CXX)
project(z_ufmaker_console LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(UFMAKER_SRC
    main.cpp
    ../z_base/folderfile.cpp
    ../z_base/fileutils.cpp
    ../z_base/aes.cpp
    ../z_base/aescrypt.cpp
    ../z_base/sha1.cpp
    ../_base/common_types.cpp
    )

add_executable(z_ufmaker_console ${UFMAKER_SRC})
set_target_properties(z_ufmaker_console
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "./")
target_link_libraries(z_ufmaker_console -lpthread -ldl)
include_directories(
    ../FaceEngine
    ../FaceEngine/AEngineLib
    ../FaceEngine/HEngineLib
    ../_driver
    ../_base
    ../_uart
    ../_keygen
    ../_des
    ../_des/openssl
    ../_aes
    )
