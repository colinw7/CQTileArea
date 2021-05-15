TEMPLATE = lib

QT += widgets

TARGET = CQTileArea

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++14

CONFIG += staticlib

# Input
HEADERS += \
../include/CQTileArea.h \
../include/CTileGrid.h \
../include/CQWidgetResizer.h \
../include/CQRubberBand.h \

SOURCES += \
CQTileArea.cpp \
CTileGrid.cpp \
CQWidgetResizer.cpp \
CQRubberBand.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += \
. \
../include \
../../CQTitleBar/include \
../../CQUtil/include \
