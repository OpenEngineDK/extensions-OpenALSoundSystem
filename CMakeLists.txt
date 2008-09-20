SET( EXTENSION_NAME "Extensions_OpenALSoundSystem")

ADD_LIBRARY( ${EXTENSION_NAME}
  Scene/SoundNode.cpp
  Sound/OpenALSoundSystem.cpp
  Sound/SoundNodeVisitor.cpp
  Sound/SoundRenderer.cpp
)

TARGET_LINK_LIBRARIES( ${EXTENSION_NAME}
  Extensions_VorbisResource
  ${OPENAL_LIBRARY}
)
