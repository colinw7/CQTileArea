TEMPLATE = app

TARGET = 

DEPENDPATH += .

#CONFIG += debug

# Input
SOURCES += \
CQTileAreaTest.cpp \

HEADERS += \
CQTileAreaTest.h \

DESTDIR     = .
OBJECTS_DIR = .

INCLUDEPATH += \
../include \
../../CQTitleBar/include \
.

unix:LIBS += \
-L../lib -L../../CQTitleBar/lib \
-lCQTileArea -lCQTitleBar
