TEMPLATE = app
QT -= Gui
TARGET = iodata-test

CONFIG += qmlog

INSTALLS = target tests

LIBS += -liodata
QMAKE_LIBDIR_FLAGS += -L../src
INCLUDEPATH += ../H

SOURCES = iodata-test.cpp

tests.path = /usr/share/iodata-tests
tests.files = tests.xml

target.path = /usr/bin
