TEMPLATE = lib

TARGET = CQTileArea

DEPENDPATH += .

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

INCLUDEPATH += . ../include ../../CQTitleBar/include
