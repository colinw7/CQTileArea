TEMPLATE = lib

TARGET = 

DEPENDPATH += .

CONFIG += staticlib

# Input
HEADERS += \
../include/CQTileArea.h \
../include/CTileGrid.h \

SOURCES += \
CQTileArea.cpp \
CTileGrid.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += . ../include ../../CQTitleBar/include
