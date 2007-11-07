#
# Find the native FREETYPE includes and library
#

# This module defines
# FREETYPE_INCLUDE_DIR, where to find ft2build.h, ftheader.h, ...
# FREETYPE_LIBRARIES, the libraries to link against to use FREETYPE.
# FREETYPE_FOUND, If false, do not try to use FREETYPE.

# also defined, but not for general use are
# FREETYPE_LIBRARY, where to find the FREETYPE library.

SET(FREETYPE_FIND_QUIETLY 1)

FIND_PATH(FREETYPE_INCLUDE_DIR_FT2BUILD ft2build.h
  /usr/include/
  /usr/local/include/
  NO_CMAKE_SYSTEM_PATH
)

FIND_PATH(FREETYPE_INCLUDE_DIR_FTHEADER freetype/config/ftheader.h
  /usr/include/freetype2
  /usr/local/include/freetype2
  ${FREETYPE_INCLUDE_DIR_FT2BUILD}
  ${FREETYPE_INCLUDE_DIR_FT2BUILD}/freetype2
  NO_CMAKE_SYSTEM_PATH
)

IF ( FREETYPE_INCLUDE_DIR_FTHEADER AND FREETYPE_INCLUDE_DIR_FT2BUILD )
	SET(FREETYPE_INCLUDE_DIR
		${FREETYPE_INCLUDE_DIR_FTHEADER}
		${FREETYPE_INCLUDE_DIR_FT2BUILD})
ENDIF ( FREETYPE_INCLUDE_DIR_FTHEADER AND FREETYPE_INCLUDE_DIR_FT2BUILD )

IF(NOT FREETYPE_FIND_QUIETLY)
  MESSAGE("FREETYPE_INCLUDE_DIR_FT2BUILD ${FREETYPE_INCLUDE_DIR_FT2BUILD}")
  MESSAGE("FREETYPE_INCLUDE_DIR_FTHEADER ${FREETYPE_INCLUDE_DIR_FTHEADER}")
  MESSAGE("FREETYPE_INCLUDE_DIR ${FREETYPE_INCLUDE_DIR}")
ENDIF(NOT FREETYPE_FIND_QUIETLY)

SET(FREETYPE_LIBRARY_NAMES_DEBUG ${FREETYPE_LIBRARY_NAMES_DEBUG} freetyped libfreetyped)
SET(FREETYPE_LIBRARY_NAMES_RELEASE ${FREETYPE_LIBRARY_NAMES_RELEASE} freetype libfreetype)

SET(FREETYPE_LIB_PATHS /usr/lib /usr/local/lib)

FIND_LIBRARY(FREETYPE_LIBRARY_RELEASE
  ${FREETYPE_LIBRARY_NAMES_RELEASE}
  ${FREETYPE_LIBRARY_NAMES}
  PATHS
  ${FREETYPE_LIB_PATHS}
  NO_CMAKE_SYSTEM_PATH
)
FIND_LIBRARY(FREETYPE_LIBRARY_DEBUG
  ${FREETYPE_LIBRARY_NAMES_DEBUG}
  PATHS
  ${FREETYPE_LIB_PATHS}
  NO_CMAKE_SYSTEM_PATH
)

INCLUDE(LibraryDebugAndRelease)
SET_LIBRARY_FROM_DEBUG_AND_RELEASE(FREETYPE)
SET(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})

IF(NOT FREETYPE_FIND_QUIETLY)
  MESSAGE("FREETYPE_LIBRARY_DEBUG ${FREETYPE_LIBRARY_DEBUG}")
  MESSAGE("FREETYPE_LIBRARY_RELEASE ${FREETYPE_LIBRARY_RELEASE}")
  MESSAGE("FREETYPE_LIBRARY ${FREETYPE_LIBRARY}")
ENDIF(NOT FREETYPE_FIND_QUIETLY)

# MESSAGE(STATUS "ft lib ${FREETYPE_LIBRARY}")
# MESSAGE(STATUS "ft2 build ${FREETYPE_INCLUDE_DIR_FT2BUILD}")
# MESSAGE(STATUS "ft header ${FREETYPE_INCLUDE_DIR_FTHEADER}")

INCLUDE(PoDoFoFindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FREETYPE DEFAULT_MSG FREETYPE_LIBRARY FREETYPE_INCLUDE_DIR)

IF (NOT FREETYPE_FIND_QUIETLY)
MESSAGE(STATUS "Found Freetype2: ${FREETYPE_LIBRARY}")
ENDIF (NOT FREETYPE_FIND_QUIETLY)
