ADD_CUSTOM_COMMAND(
	OUTPUT ui_podofoaboutdlg.h ui_podofoaboutdlg.cpp
	COMMAND ${QT_UIC3_EXECUTABLE} -o ui_podofoaboutdlg.h
				      podofoaboutdlg.ui
	COMMAND ${QT_UIC3_EXECUTABLE} -o ui_podofoaboutdlg.cpp
				      -impl ui_podofoaboutdlg.h
				      podofoaboutdlg.ui
	COMMENT "Generating ui_podofoaboutdlg.{cpp,h}"
	)


ADD_CUSTOM_COMMAND(
	OUTPUT ui_podofobrowserbase.h ui_podofobrowserbase.cpp
	COMMAND ${QT_UIC3_EXECUTABLE} -o ui_podofobrowserbase.h
				      podofobrowserbase.ui
	COMMAND ${QT_UIC3_EXECUTABLE} -o ui_podofobrowserbase.cpp
				      -impl ui_podofobrowserbase.h
				      podofobrowserbase.ui
	COMMENT "Generating ui_podofobrowserbase.{cpp,h}"
	)

SET(podofobrowser_MOC_HEADERS
	podofobrowser.h
	ui_podofoaboutdlg.h
	ui_podofobrowserbase.h
	)
QT4_WRAP_CPP(podofobrowser_MOC_SRCS ${podofobrowser_MOC_HEADERS})

ADD_EXECUTABLE(podofobrowser
	ui_podofobrowserbase.cpp
	ui_podofoaboutdlg.cpp
	podofobrowser.cpp
	pdflistviewitem.cpp
	main.cpp
	${podofobrowser_MOC_SRCS}
	)

IF(NOT WIN32)
	SET(extra_libs ${LIBFONTCONFIG_LIB})
ELSE(NOT WIN32)
	SET(extra_libs)
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES(podofobrowser
	${QT_LIBRARIES}
	${LIBPODOFO_LIB}
	${LIBFREETYPE_LIB}
	${extra_libs}
	${LIBJPEG_LIB}
	${LIBZ_LIB}
	)

ADD_DEFINITIONS(
	${LIBPODOFO_CFLAGS}
	${QT_DEFINITIONS}
	-DQT_QT3SUPPORT_LIB
	-DQT3_SUPPORT
	-DQT_CORE_LIB
	-DQT_GUI_LIB
	-DQT_THREAD_SUPPORT
	)

INCLUDE_DIRECTORIES(
	${QT_INCLUDES}
	${PODOFO_INCLUDES}
	)

INSTALL(TARGETS podofobrowser
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	)
