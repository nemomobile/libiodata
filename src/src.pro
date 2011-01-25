VERSION = 0.$$(IODATA_VERSION)
TEMPLATE=lib
QT -= gui

CONFIG += qmlog

INCLUDEPATH += ../H
SOURCES = iodata.cpp validator.cpp storage.cpp

TARGET = iodata
target.path = /usr/lib

devheaders.files = iodata.h validator.h storage.h iodata validator storage
devheaders.path  = /usr/include/iodata

prf.files = iodata.prf
prf.path = /usr/share/qt4/mkspecs/features

INSTALLS = target devheaders prf

FLEXSOURCES = datalang.l
BISONSOURCES = datalang.y
QMAKE_EXTRA_COMPILERS += bison flex bisonh

flex.commands = flex -o tokens.cpp ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = tokens.cpp
flex.variable_out = SOURCES
flex.depends = parser.hpp
flex.name = flex

bison.commands = bison -v -d -o parser.cpp ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output = parser.cpp
bison.variable_out = SOURCES
bison.name = bison

bisonh.commands = true
bisonh.input = BISONSOURCES
bisonh.output = parser.hpp
bisonh.variable_out = HEADERS
bisonh.name = bison header
bisonh.depends = parser.cpp
