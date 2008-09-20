SET(CMAKE_INCLUDE_PATH ${OE_LIB_DIR}/openal/include)
SET(CMAKE_LIBRARY_PATH ${OE_LIB_DIR}/openal/lib/shared)
FIND_PACKAGE(OpenAL)

INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})

IF (NOT OPENAL_FOUND) 
  MESSAGE ("WARNING: Could not find OpenAL - depending targets will be disabled.")
  SET(OE_MISSING_LIBS "${OE_MISSING_LIBS}, OpenAL")
ENDIF (NOT OPENAL_FOUND)
