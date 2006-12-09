SET(podofobrowser_UIS
	podofobrowserbase.ui
	podofoaboutdlg.ui
	)

SET(podofobrowser_MOC_HEADERS
	podofobrowser.h
	)

# We use UIC3, which isn't supported by QT4_WRAP_UI
#QT4_WRAP_UI(podofobrowser_UIS_H ${podofobrowser_UIS})

QT4_WRAP_CPP(podofobrowser_MOC_SRCS ${podofobrowser_MOC_HEADERS})

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

ADD_EXECUTABLE(podofobrowser
	ui_podofobrowserbase.cpp
	ui_podofoaboutdlg.cpp
	podofobrowser.cpp
	pdflistviewitem.cpp
	main.cpp
	${podofobrowser_MOC_SRCS}
	)

ADD_DEFINITIONS(
	${PODOFO_CFLAGS}
	${QT_DEFINITIONS}
	-DQT_QT3SUPPORT_LIB
	-DQT3_SUPPORT
	-DQT_CORE_LIB
	-DQT_GUI_LIB
	-DQT_THREAD_SUPPORT
	)
IF(${PODOFO_DEPEND_TARGET} EQUAL "podofo_shared")
	SET(podofo_DEPENDS ${podofo_shared_LIB_DEPENDS})
ELSE(${PODOFO_DEPEND_TARGET} EQUAL "podofo_shared")
	SET(podofo_DEPENDS ${podofo_static_LIB_DEPENDS})
ENDIF(${PODOFO_DEPEND_TARGET} EQUAL "podofo_shared")

TARGET_LINK_LIBRARIES(podofobrowser
	podofo
	${podofo_DEPENDS}
	${QT_LIBRARIES}
	)

INCLUDE_DIRECTORIES(
	${QT_INCLUDES}
	${PODOFO_INCLUDES}
	)
