TEMPLATE = subdirs

SUBDIRS = src tests type-to-cxx

equals(QT_MAJOR_VERSION, 4) {
    prf.files = iodata.prf
    prf.path = /usr/share/qt4/mkspecs/features
}
equals(QT_MAJOR_VERSION, 5) {
    prf.files = iodata-qt5.prf
    prf.path = /usr/share/qt5/mkspecs/features
}

INSTALLS = prf
