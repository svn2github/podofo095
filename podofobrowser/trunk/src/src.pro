
SOURCES += main.cpp \
           podofobrowser.cpp \
           pdflistviewitem.cpp
HEADERS += podofobrowser.h \
           pdflistviewitem.h
FORMS   += podofobrowserbase.ui
TEMPLATE = app
CONFIG += debug \
          warn_on \
	  thread \
          qt
TARGET = ../bin/podofobrowser
INCLUDEPATH = /usr/include/fontconfig/ /usr/include/freetype2/
LIBS = -lpodofo
