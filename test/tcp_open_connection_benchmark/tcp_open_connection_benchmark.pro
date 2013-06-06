TEMPLATE = app
TARGET = tcp_open_connection_benchmark
CONFIG += qt console
QT += network

INCLUDEPATH += ../../src

HEADERS += tcp_open_connection_benchmark.h
SOURCES += tcp_open_connection_benchmark.cpp
SOURCES += main.cpp


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../release/ -lparalleleventloop
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../debug/ -lparalleleventloop
else:unix: LIBS += -L$$OUT_PWD/../../ -lparalleleventloop

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src
