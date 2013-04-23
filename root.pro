TEMPLATE = subdirs

SUBDIRS = src tests type-to-cxx

prf.path =  $$[QT_INSTALL_DATA]/mkspecs/features
equals(QT_MAJOR_VERSION, 4): prf.files = iodata.prf
equals(QT_MAJOR_VERSION, 5): prf.files = iodata-qt5.prf

INSTALLS = prf

OTHER_FILES += rpm/*.spec *.prf
