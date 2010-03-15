SET( EXTENSION_NAME "Extensions_OpenALSoundSystem")

ADD_LIBRARY( ${EXTENSION_NAME}
  Scene/SoundNode.h
  Scene/SoundNode.cpp
  Sound/OpenALSoundSystem.h
  Sound/OpenALSoundSystem.cpp
  Sound/SoundNodeVisitor.h
  Sound/SoundNodeVisitor.cpp
#  Sound/SoundRenderer.cpp
)

TARGET_LINK_LIBRARIES( ${EXTENSION_NAME}
  OpenEngine_Math
  Extensions_VorbisResource
  ${OPENAL_LIBRARY}
)
