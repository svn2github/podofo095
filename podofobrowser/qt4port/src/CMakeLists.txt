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

# Determine the right dependency list to add to the linker, depending
# on whether we're linking a shared or static PoDoFo
IF("${PODOFO_DEPEND_TARGET}" STREQUAL "podofo_shared")
	MESSAGE("Linking to shared PoDoFo")
	SET(podofo_DEPENDS ${podofo_shared_LIB_DEPENDS})
ELSE("${PODOFO_DEPEND_TARGET}" STREQUAL "podofo_shared")
	IF("${PODOFO_DEPEND_TARGET}" STREQUAL "podofo_static")
		MESSAGE("Linking to static PoDoFo")
		SET(podofo_DEPENDS ${podofo_static_LIB_DEPENDS})
	ELSE("${PODOFO_DEPEND_TARGET}" STREQUAL "podofo_static")
		MESSAGE(FATAL_ERROR "PODOFO_DEPEND_TARGET '${PODOFO_DEPEND_TARGET}' invalid")
	ENDIF("${PODOFO_DEPEND_TARGET}" STREQUAL "podofo_static")
ENDIF("${PODOFO_DEPEND_TARGET}" STREQUAL "podofo_shared")

TARGET_LINK_LIBRARIES(podofobrowser
	${QT_LIBRARIES}
	${PoDoFo_LIB}
	${podofo_DEPENDS}
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

INCLUDE_DIRECTORIES(
	${QT_INCLUDES}
	${PODOFO_INCLUDES}
	)
