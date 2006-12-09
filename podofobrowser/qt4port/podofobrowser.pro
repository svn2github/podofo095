TEMPLATE = app

CONFIG += debug qt windows exceptions stl uic3
QT += qt3support
TARGET = src/podofobrowser
DEFINES += USING_SHARED_PODOFO
DEPENDPATH += .
INCLUDEPATH += . c:\developer\podofo\src c:\developer\gnuwin32\include c:\developer\gnuwin32\include\freetype2

unix:LIBS += -lpodofo
win32:LIBS += C:/developer/podofo-debug-mingw/src/libpodofo.dll.a

FORMS3 = \
	src/podofoaboutdlg.ui \
	src/podofobrowserbase.ui
HEADERS += \
	src/pdflistviewitem.h \
	src/podofobrowser.h
SOURCES += \
	src/main.cpp \
	src/pdflistviewitem.cpp \
	src/podofobrowser.cpp

