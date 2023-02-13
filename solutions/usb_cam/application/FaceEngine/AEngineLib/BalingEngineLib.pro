#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T22:21:30
#
#-------------------------------------------------


CONFIG   += console staticlib
CONFIG   -= app_bundle qt

TARGET = balingenginelib
TEMPLATE = lib

QMAKE_CXXFLAGS += -fexceptions -mthumb -mhard-float -mfloat-abi=hard -mfpu=neon -fsigned-char -O3 -std=c++0x #-ffunction-sections #-fdata-sections
QMAKE_CFLAGS += -fexceptions -mthumb -mhard-float -mfloat-abi=hard -mfpu=neon -fsigned-char -O3 #-ffunction-sections #-fdata-sections

QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-switch -Wno-maybe-uninitialized
QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-delete-non-virtual-dtor -Wno-write-strings -Wno-sign-compare -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-strict-aliasing #-Wno-char-subscripts
QMAKE_CFLAGS += -Wno-write-strings -Wno-missing-field-initializers -Wno-comment -Wno-narrowing -Wno-maybe-uninitialized
QMAKE_CFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-sign-compare -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-strict-aliasing #-Wno-char-subscripts


DESTDIR = ../BalingLock_Product

DEFINES += FACEENGINE_LIBRARY _A10_

SOURCES += \
    wav_trf.c \
    wave_denoise.c \
    wav_basic.c \
    util.cpp \
    type.cpp \
    morph.cpp \
    LinerGeometry.cpp \
    inpaint.cpp \
    histogram.cpp \
    GlassesRemove.cpp \
    armPatternMatcher.c \
    armFeaturePattern.c \
    armFeatureMatcher.c \
    armFaceModelDetector.c \
    armFaceFeatureExtractor.c \
    armFaceDetector.c \
    armCommon.c \
    alloc.c

HEADERS += \
    wav_trf.h \
    wav_gen.h \
    wav_filters_extern.h \
    wav_filters.h \
    wave_denoise.h \
    wav_basic.h \
    util.h \
    type.h \
    morph.h \
    macros.h \
    LinerGeometry.h \
    inpaint.h \
    histogram.h \
    GlassRemove.h \
    armType.h \
    armPatternMatcher.h \
    armFeaturePattern.h \
    armFeatureMatcher.h \
    armFaceModelDetectorTable.h \
    armFaceModelDetector.h \
    armFaceFeatureExtractorTable.h \
    armFaceFeatureExtractor.h \
    armFaceDetectorTable.h \
    armFaceDetector.h \
    armCommon.h \
    alloc.h \
    ../_base/appdef.h


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

