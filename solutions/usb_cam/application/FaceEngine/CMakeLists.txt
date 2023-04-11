include(../config.cmake)

project(FaceEngine LANGUAGES CXX)
project(FaceEngine LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -O3 -fsigned-char -mfpu=neon-fp16 -mfp16-format=ieee -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb -O3 -fsigned-char -mfpu=neon-fp16 -mfp16-format=ieee -std=c++11 -fopenmp")

add_definitions(-DFE_ENGINE_LIB)

set(FACEENGINE_SRC
    ImageProcessing.cpp
    FaceRetrievalSystem.cpp
    sha1.cpp
    gammacorrection.cpp
    manageEnvironment.cpp
    convert.cpp
    Pose.cpp
    Matd.cpp
    FaceRetrievalSystem_dnn.cpp
    FaceRetrievalSystem_h.cpp
    manageIRCamera.cpp
    convertBayer2Y_cm.cpp
    ComboRetrievalSystem.cpp
    ../_base/DBManager.cpp
    ../_base/common_types.cpp
#    ./hand/hand.cpp
#    ./hand/HandRetrival.cpp
    ./hand/HandRetrival_.cpp
)

set(DNN_SRC
    _dnn/KDNN_EngineInterface_liveness.cpp
    _dnn/KDNN_EngineInterface_feat.cpp
    _dnn/KDNN_handValid.cpp
    _dnn/HAlign.cpp
    _dnn/UltraFace.cpp
    _dnn/modeling_interface.cpp
    _dnn/esn_detection.cpp
    _dnn/occ_detection.cpp
    _dnn/esn.cpp
    _dnn/livemnse.c
    _dnn/livemnse3.c
    _dnn/occ.cpp
    _dnn/modeling.c
    _dnn/detect.c
    _dnn/feat.cpp
    _dnn/hand_feat.c
    _dnn/dic_manage.cpp
)

set(ENN_SRC
    enn/enn_activation.c
    enn/enn_conv.c
    enn/enn_eltwise.c
    enn/enn_global.c
    enn/enn_inner.c
    enn/enn_pad.c
    enn/enn_permute.c
    enn/enn_pool.c
    enn/enn_reshape.c
    enn/enn_softmax.c
    enn/enn_trans.c
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
    target_link_libraries(faceengine -L../../FaceEngine/HEngineLib -lhengine)
endif()

include_directories(./ ./HEngineLib ./AEngineLib  ./_dnn ./_nqmfnr ./enn ./ennq ../_base ../_uart ./hand)

#/root/QuickBoot/a20/
