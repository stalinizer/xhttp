PROJECT (xhttp)
cmake_minimum_required(VERSION 2.8)
SET(SRC_DIR ./)

SET(SRC_LIST 
    ${SRC_DIR}/multipart_parser.c
    ${SRC_DIR}/lua_multipart_parser.c
    ${SRC_DIR}/lua_xhttp.c
)
INCLUDE_DIRECTORIES(${SRC_DIR} ../luaheaders)
IF(APPLE)
set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
ENDIF(APPLE)
ADD_LIBRARY(xhttp SHARED ${SRC_LIST})
SET_TARGET_PROPERTIES(xhttp PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(xhttp PROPERTIES SUFFIX .so)