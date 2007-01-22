FIND_PATH(LIBPODOFO_H
	NAMES podofo/podofo.h
	PATHS "${LIBPODOFO_DIR}/include" "${LIBPODOFO_DIR}/src" "${LIBPODOFO_DIR}"
	)
IF(LIBPODOFO_H)
MESSAGE("podofo/podofo.h: ${LIBPODOFO_H}")
ELSE(LIBPODOFO_H)
MESSAGE("podofo/podofo.h: not found")
ENDIF(LIBPODOFO_H)

FIND_LIBRARY(LIBPODOFO_LIB
	NAMES libpodofo podofo
	PATHS "${LIBPODOFO_DIR}/lib" "${LIBPODOFO_DIR}/src" "${LIBPODOFO_DIR}")
IF(LIBPODOFO_LIB)
MESSAGE("podofo lib: ${LIBPODOFO_LIB}")
ELSE(LIBPODOFO_LIB)
MESSAGE("podofo lib: not found")
ENDIF(LIBPODOFO_LIB)

IF(LIBPODOFO_H AND LIBPODOFO_LIB)
	SET(LIBPODOFO_FOUND TRUE CACHE BOOLEAN "Was libpodofo found")
ENDIF(LIBPODOFO_H AND LIBPODOFO_LIB)
