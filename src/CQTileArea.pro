TEMPLATE = lib

QT += widgets

TARGET = CQTileArea

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++14

CONFIG += staticlib

# Input
HEADERS += \
../include/CQRubberBand.h \
../include/CQTileAreaConstants.h \
../include/CQTileArea.h \
../include/CQTileAreaMenuControls.h \
../include/CQTileAreaMenuIcon.h \
../include/CQTileAreaSplitter.h \
../include/CQTileStackedWidget.h \
../include/CQTileWindowArea.h \
../include/CQTileWindow.h \
../include/CQTileWindowTabBar.h \
../include/CQTileWindowTitle.h \
../include/CQWidgetResizer.h \
../include/CTileGrid.h \

SOURCES += \
CQRubberBand.cpp \
CQTileArea.cpp \
CQTileAreaMenuControls.cpp \
CQTileAreaMenuIcon.cpp \
CQTileAreaSplitter.cpp \
CQTileStackedWidget.cpp \
CQTileWindowArea.cpp \
CQTileWindow.cpp \
CQTileWindowTabBar.cpp \
CQTileWindowTitle.cpp \
CQWidgetResizer.cpp \
CTileGrid.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += \
. \
../include \
../../CQTitleBar/include \
../../CQUtil/include \
