VERSION = $$(IODATA_VERSION)
TEMPLATE = app
QT -= gui
TARGET = iodata-type-to-c++

CONFIG += qmlog

INSTALLS = target

LIBS += -liodata -lcrypt
QMAKE_LIBDIR_FLAGS += -L../src
INCLUDEPATH += ../H

SOURCES = type-to-cxx.cpp

target.path = /usr/bin
