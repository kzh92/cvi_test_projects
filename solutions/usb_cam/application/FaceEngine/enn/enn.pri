
QT       += core

INCLUDEPATH  += $$PWD

HEADERS += \
    $$PWD/enn_activation.h \
    $$PWD/enn_conv.h \
    $$PWD/enn_eltwise.h \
    $$PWD/enn_global.h \
    $$PWD/enn_inner.h \
    $$PWD/enn_pad.h \
    $$PWD/enn_permute.h \
    $$PWD/enn_pool.h \
    $$PWD/enn_reshape.h \
    $$PWD/enn_softmax.h \
    $$PWD/enn_trans.h

SOURCES += \
    $$PWD/enn_activation.cpp \
    $$PWD/enn_conv.cpp \
    $$PWD/enn_eltwise.cpp \
    $$PWD/enn_global.cpp \
    $$PWD/enn_inner.cpp \
    $$PWD/enn_pad.cpp \
    $$PWD/enn_permute.cpp \
    $$PWD/enn_pool.cpp \
    $$PWD/enn_reshape.cpp \
    $$PWD/enn_softmax.cpp \
    $$PWD/enn_trans.cpp
