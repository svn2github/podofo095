TEMPLATE = app

# This project file WILL NOT WORK as written.
# you must update it to suit your system and may also need to make
# changes for your Qt version. qmake builds are not tested, see
# INSTALL for instructions on using CMake to build podofobrowser .

CONFIG = uic3 debug qt windows exceptions stl
QT += qt3support
TARGET = 
DEFINES += USING_SHARED_PODOFO
DEPENDPATH += .
INCLUDEPATH += . c:\developer\podofo\src c:\developer\gnuwin32\include c:\developer\gnuwin32\include\freetype2

unix:LIBS += -lpodofo
win32:LIBS += C:/developer/podofo-debug-mingw/src/libpodofo.dll.a

FORMS = \
	src/podofoaboutdlg.ui \
	src/podofobrowserbase.ui
HEADERS += \
	src/pdflistviewitem.h \
	src/podofobrowser.h
SOURCES += \
	src/main.cpp \
	src/pdflistviewitem.cpp \
	src/podofobrowser.cpp

