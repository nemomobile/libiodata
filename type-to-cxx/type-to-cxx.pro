VERSION = $$(IODATA_VERSION)
TEMPLATE = app
QT -= gui
equals(QT_MAJOR_VERSION, 4): TARGET = iodata-type-to-c++
equals(QT_MAJOR_VERSION, 5): TARGET = iodata-qt5-type-to-c++

INSTALLS = target

equals(QT_MAJOR_VERSION, 4): LIBS += -liodata -lcrypt
equals(QT_MAJOR_VERSION, 5): LIBS += -liodata-qt5 -lcrypt
QMAKE_LIBDIR_FLAGS += -L../src

SOURCES = type-to-cxx.cpp

target.path = /usr/bin
