# viewer lists
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src
)

SET(VIEWER_SRCS
core.cpp
dispatcher.cpp
fontface.cpp
graphicstackmaker.cpp
graphicstate.cpp
graphicstatemanip.cpp
pathitemmaker.cpp
simplecolormaker.cpp
simpletransformmaker.cpp
textmaker.cpp
)

ADD_LIBRARY(viewer STATIC ${VIEWER_SRCS})