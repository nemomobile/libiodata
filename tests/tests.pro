VERSION = $$(IODATA_VERSION)
TEMPLATE = app
QT -= gui
equals(QT_MAJOR_VERSION, 4): TARGET = iodata-test
equals(QT_MAJOR_VERSION, 5): TARGET = iodata-qt5-test

INSTALLS = target tests

equals(QT_MAJOR_VERSION, 4): LIBS += -liodata
equals(QT_MAJOR_VERSION, 5): LIBS += -liodata-qt5
QMAKE_LIBDIR_FLAGS += -L../src

SOURCES = iodata-test.cpp

equals(QT_MAJOR_VERSION, 4): tests.path = /usr/share/iodata-tests
equals(QT_MAJOR_VERSION, 5): tests.path = /usr/share/iodata-qt5-tests

tests.files = tests.xml

target.path = /usr/bin
