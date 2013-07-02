TEMPLATE = lib

TARGET = 

DEPENDPATH += .

CONFIG += staticlib

# Input
HEADERS += \
CQTileArea.h \
CTileGrid.h \

SOURCES += \
CQTileArea.cpp \
CTileGrid.h \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += . ../../CQTitleBar/src
