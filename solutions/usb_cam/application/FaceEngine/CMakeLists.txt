include(../config.cmake)

project(FaceEngine LANGUAGES CXX)
project(FaceEngine LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -O3 -fsigned-char -mfpu=neon -std=c++11 -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb -O3 -fsigned-char -mfpu=neon -std=c++11 -fopenmp")

add_definitions(-DFE_ENGINE_LIB)

set(FACEENGINE_SRC
    ImageProcessing.cpp
    FaceRetrievalSystem.cpp
    sha1.cpp
    gammacorrection.cpp
    removebackground.cpp
    manageEnvironment.cpp
    convert.cpp
    Pose.cpp
    Matd.cpp
    FaceRetrievalSystem_dnn.cpp
    FaceRetrievalSystem_h.cpp
    manageIRCamera.cpp
    convertBayer2Y.cpp
    ../_base/DBManager.cpp
    ../_base/common_types.cpp
)

set(DNN_SRC
    _dnn/KDNN_EngineInterface_liveness.cpp
    _dnn/KDNN_EngineInterface_feat.cpp
    _dnn/HAlign.cpp
    _dnn/UltraFace.cpp
    _dnn/modeling_interface.cpp
    _dnn/livemnse3.cpp
    _dnn/esn_detection.cpp
    _dnn/occ_detection.cpp
    _dnn/esn.cpp
    _dnn/livemn.cpp
    _dnn/livemnse.cpp
    _dnn/occ.cpp
    _dnn/modeling.cpp
    _dnn/detect.cpp
    _dnn/feat.cpp
    _dnn/dic_manage.cpp
)

set(ENN_SRC
    enn/enn_activation.cpp
    enn/enn_conv.cpp
    enn/enn_eltwise.cpp
    enn/enn_global.cpp
    enn/enn_inner.cpp
    enn/enn_pad.cpp
    enn/enn_permute.cpp
    enn/enn_pool.cpp
    enn/enn_reshape.cpp
    enn/enn_softmax.cpp
    enn/enn_trans.cpp
    )

set(ENNQ_SRC
    ennq/ennq_activation.cpp
    ennq/ennq_conv.cpp
    ennq/ennq_global.cpp
    ennq/ennq_inner.cpp
    ennq/ennq_normal.cpp
    ennq/ennq_pad.cpp
    ennq/ennq_pool.cpp
    ennq/ennq_quantize.cpp
    ennq/ennq_seblock.cpp
    ennq/ennq_slel.cpp
    ennq/ennq_trans.cpp
    )

add_library(faceengine SHARED ${FACEENGINE_SRC} ${DNN_SRC} ${ENN_SRC} ${ENNQ_SRC})
set_target_properties(faceengine
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "../PusinLock_Product"
    LINKER_LANGUAGE CXX)

target_link_libraries(faceengine -lpthread -ldl -lgomp)

if (DEFINED MY_USE_TWIN_ENGINE)
    target_link_libraries(faceengine -L../FaceEngine/HEngineLib -lhengine)
endif()

include_directories(./ ./HEngineLib ./AEngineLib  ./_dnn ./_nqmfnr ./enn ./ennq ../_base ../_uart)

#/root/QuickBoot/a20/