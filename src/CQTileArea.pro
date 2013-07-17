TEMPLATE = lib

TARGET = 

DEPENDPATH += .

CONFIG += staticlib

# Input
HEADERS += \
../include/CQTileArea.h \
../include/CTileGrid.h \
../include/CQWidgetResizer.h \

SOURCES += \
CQTileArea.cpp \
CTileGrid.cpp \
CQWidgetResizer.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += . ../include ../../CQTitleBar/include
