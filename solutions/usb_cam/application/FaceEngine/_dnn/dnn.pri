
QT       += core

INCLUDEPATH  += $$PWD

HEADERS += \
    $$PWD/KDNN_EngineInterface.h \
    $$PWD/HAlign.h \
    $$PWD/UltraFace.hpp \
    $$PWD/modeling_interface.h\
    $$PWD/esn_detection.h \
    $$PWD/occ_detection.h \
    $$PWD/esn.h \
    $$PWD/livemn.h \
    $$PWD/livemnse.h \
    $$PWD/occ.h \
    $$PWD/modeling.h \
    $$PWD/detect.h \
    $$PWD/feat.h \
    $$PWD/dic_manage.h

SOURCES += \
    $$PWD/KDNN_EngineInterface_liveness.cpp \
    $$PWD/KDNN_EngineInterface_feat.cpp \
    $$PWD/HAlign.cpp \
    $$PWD/UltraFace.cpp \
    $$PWD/modeling_interface.cpp\
    $$PWD/esn_detection.cpp \
    $$PWD/occ_detection.cpp \
    $$PWD/esn.cpp \
    $$PWD/livemn.cpp \
    $$PWD/livemnse.cpp \
    $$PWD/livemnse3.cpp \
    $$PWD/occ.cpp \
    $$PWD/modeling.cpp \
    $$PWD/detect.cpp \
    $$PWD/feat.cpp \
    $$PWD/dic_manage.cpp

