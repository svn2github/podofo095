ADD_EXECUTABLE(helloworld helloworld.cpp)

TARGET_LINK_LIBRARIES(helloworld ${PODOFO_LIB})
SET_TARGET_PROPERTIES(helloworld PROPERTIES COMPILE_FLAGS "${PODOFO_CFLAGS}")
ADD_DEPENDENCIES(helloworld ${PODOFO_DEPEND_TARGET})
INCLUDE_DIRECTORIES(${PoDoFo_SOURCE_DIR})
