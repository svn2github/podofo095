# fontconfig often lives in a subdir off the include path
# called just `fontconfig'
FIND_PATH(LIBFONTCONFIG_INCLUDEPATH NAMES fontconfig.h)
IF(LIBFONTCONFIG_INCLUDEPATH)
    MESSAGE("Found fontconfig.h at ${LIBFONTCONFIG_H}/fontconfig.h}")
    SET(LIBFONTCONFIG_H ${LIBFONTCONFIG_INCLUDEPATH})
ELSE(LIBFONTCONFIG_INCLUDEPATH)
    FIND_PATH(LIBFONTCONFIG_INCLUDEPATH_PARENTDIR NAMES fontconfig/fontconfig.h)
    MESSAGE("Found fontconfig.h at ${LIBFONTCONFIG_INCLUDEPATH_PARENTDIR}/fontconfig/fontconfig.h")
    SET(LIBFONTCONFIG_H ${LIBFONTCONFIG_INCLUDEPATH_PARENTDIR}/fontconfig)
ENDIF(LIBFONTCONFIG_INCLUDEPATH)
# The fontconfig library is generally easy to find
FIND_LIBRARY(LIBFONTCONFIG_LIB NAMES fontconfig)
MESSAGE("Found fontconfig library at ${LIBFONTCONFIG_LIB}")

IF(LIBFONTCONFIG_INCLUDEPATH AND LIBFONTCONFIG_LIB)
        SET(LIBFONTCONFIG_FOUND TRUE CACHE BOOLEAN "Was fontconfig found")
ELSE(LIBFONTCONFIG_INCLUDEPATH AND LIBFONTCONFIG_LIB)
        SET(LIBFONTCONFIG_FOUND FALSE CACHE BOOLEAN "Was fontconfig found")
ENDIF(LIBFONTCONFIG_INCLUDEPATH AND LIBFONTCONFIG_LIB)
