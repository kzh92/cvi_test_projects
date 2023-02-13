#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T22:21:30
#
#-------------------------------------------------

include(../config.pri)

QT       =

QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle qt

TARGET = faceengine
TEMPLATE = lib

QMAKE_CXXFLAGS += -mthumb -O3 -fsigned-char -mfpu=neon -std=c++11 #-std=c++0x -ffast-math -fopenmp#-finstrument-functions -ffunction-sections
QMAKE_CFLAGS += -mthumb -O3 -fsigned-char -mfpu=neon #-ffast-math -fopenmp#-finstrument-functions -ffunction-sections

QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-unknown-pragmas -Wno-maybe-uninitialized
QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-delete-non-virtual-dtor -Wno-write-strings -Wno-sign-compare -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-strict-aliasing #-Wno-char-subscripts
QMAKE_CFLAGS += -Wno-write-strings -Wno-missing-field-initializers -Wno-comment -Wno-narrowing
QMAKE_CFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-sign-compare -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-strict-aliasing #-Wno-char-subscripts

DEFINES += "ENGINE_DEV_V3S"
DEFINES += "ENGINE_THREAD_COUNT=1"

DESTDIR = ../PusinLock_Product

equals(ENGINE_PROTECT, 1) {
    DEFINES += PROTECT_ENGINE
    equals(USE_TWIN_ENGINE, 1) {
        LIBS += -L../PusinLock_Product -lhengine
    }
} else {
    equals(USE_TWIN_ENGINE, 1) {
        LIBS += -L../PusinLock_Product -lhengine
    }
}

equals(ENGINE_FOR_DESSMAN, 1) {
    DEFINES += ENGINE_FOR_DESSMAN
}
equals(ENGINE_IS_DUALCAMERA, 1) {
    DEFINES += ENGINE_IS_DUALCAMERA
}
equals(ENGINE_USE_DevMemInit, 1) {
    DEFINES += ENGINE_USE_DevMemInit
}

DEFINES += FACEENGINE_LIBRARY _A10_

INCLUDEPATH += ./HEngineLib ./AEngineLib ../_base ../_uart

include(_dnn/dnn.pri)
include(enn/enn.pri)
include(ennq/ennq.pri)

SOURCES += ImageProcessing.cpp \
    FaceRetrievalSystem.cpp \
    sha1.cpp \
    gammacorrection.cpp \
    manageEnvironment.cpp \
    convert.cpp \
    Pose.cpp \
    Matd.cpp \
    FaceRetrievalSystem_dnn.cpp \
    FaceRetrievalSystem_h.cpp \
    manageIRCamera.cpp \
    convertBayer2Y.cpp\
    ../_base/DBManager.cpp \
    ../_base/common_types.cpp \
    ../_base/mmap_base.cpp

HEADERS += \
    ImageProcessing.h \
    FaceRetrievalSystem.h \
    EngineStruct.h \
    EngineDef.h \
    DBManager.h \
    jpge.h \
    sha1.h \
    gammacorrection.h \
    ../_base/common_types.h \
    manageEnvironment.h \
    mmap_base.h \
    convert.h \
    Matd.h \
    FaceRetrievalSystem_base.h \
    FaceRetrievalSystem_dnn.h \
    FaceRetrievalSystem_h.h \
    engine_inner_param.h\
    manageIRCamera.h\
    convertBayer2Y.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

