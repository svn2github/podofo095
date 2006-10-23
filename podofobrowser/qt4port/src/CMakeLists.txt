SET(podofobrowser_UIS
	podofobrowserbase.ui
	podofoaboutdlg.ui
	)

SET(podofobrowser_MOC_HEADERS
	pdflistviewitem.h
	podofobrowser.h
	)

QT4_WRAP_UI(podofobrowser_UIS_H ${podofobrowser_UIS})
QT4_WRAP_CPP(podofobrowser_MOC_SRCS ${podofobrowser_MOC_HEADERS})

ADD_EXECUTABLE(podofobrowser
	main.cpp
	pdflistviewitem.cpp
	podofobrowser.cpp
	${podofobrowser_UIS_H}
	${podofobrowser_MOC_SRCS}
	)
SET_TARGET_PROPERTIES(
	podofobrowser
	PROPERTIES
	COMPILE_FLAGS ${PODOFO_CFLAGS} ${QT_DEFINITIONS}
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
