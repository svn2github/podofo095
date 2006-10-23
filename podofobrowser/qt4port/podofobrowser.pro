TEMPLATE = app
CONFIG += qt uic3
QT += qt3support
TARGET = 
DEFINES += USING_SHARED_PODOFO
DEPENDPATH += .
INCLUDEPATH += . src d:\developer\podofo\src d:\developer\scribus\gnuwin32\include d:\developer\scribus\gnuwin32\include\freetype2

unix:LIBS +=
win32:LIBS += d:/developer/podofo-mingw/src/libpodofo.dll.a

HEADERS += src/pdflistviewitem.h src/podofobrowser.h
FORMS3 += src/podofoaboutdlg.ui src/podofobrowserbase.ui
SOURCES += src/main.cpp src/pdflistviewitem.cpp src/podofobrowser.cpp
