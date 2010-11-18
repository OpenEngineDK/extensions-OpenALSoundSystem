SET(CMAKE_INCLUDE_PATH ${OE_LIB_DIR}/openal/include
		       ${OE_LIB_DIR}/openal/include/AL)
SET(CMAKE_LIBRARY_PATH ${OE_LIB_DIR}/openal/lib
		       ${OE_LIB_DIR}/openal/lib)

FIND_PACKAGE(OpenAL)

INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})

IF (NOT OPENAL_FOUND) 
  MESSAGE ("WARNING: Could not find OpenAL - depending targets will be disabled.")
  SET(OE_MISSING_LIBS "${OE_MISSING_LIBS}, OpenAL")
ENDIF (NOT OPENAL_FOUND)

OE_ADD_SCENE_NODES(Extensions_OpenALSoundSystem
  Scene/SoundNode
)
