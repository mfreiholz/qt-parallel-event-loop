TEMPLATE = lib
TARGET = paralleleventloop
CONFIG += qt dll
QT += network

INCLUDEPATH += ./src

DEFINES += QT_PEL_EXPORT

HEADERS += src/apidef.h
HEADERS += src/paralleleventloop.h
SOURCES += src/paralleleventloop.cpp
