VERSION = $$(IODATA_VERSION)
TEMPLATE = app
QT -= gui
TARGET = iodata-type2cpp

CONFIG += qmlog

INSTALLS = target

LIBS += -liodata
QMAKE_LIBDIR_FLAGS += -L../src
INCLUDEPATH += ../H

SOURCES = .cpp

target.path = /usr/bin

