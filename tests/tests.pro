VERSION = $$(IODATA_VERSION)
TEMPLATE = app
QT -= gui

equals(QT_MAJOR_VERSION, 4): PACKAGENAME = iodata
equals(QT_MAJOR_VERSION, 5): PACKAGENAME = iodata-qt5

TARGET = $${PACKAGENAME}-test

LIBS += -l$${PACKAGENAME}
QMAKE_LIBDIR_FLAGS += -L../src

SOURCES = iodata-test.cpp

target.path = /usr/bin

tests_xml.target = tests.xml
tests_xml.depends = $$PWD/tests.xml.in
tests_xml.commands = sed -e "s:@PACKAGENAME@:$${PACKAGENAME}:g" -e \'s%@PATH@%$${target.path}%\' $<  > $@
tests_xml.CONFIG += no_check_exist

QMAKE_EXTRA_TARGETS = tests_xml
QMAKE_CLEAN += $$tests_xml.target
PRE_TARGETDEPS += $$tests_xml.target

tests_install.path = /usr/share/$${PACKAGENAME}-tests
tests_install.files = $$tests_xml.target
tests_install.CONFIG += no_check_exist

INSTALLS = target tests_install

