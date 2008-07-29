
# Create the extension library
ADD_LIBRARY(Extensions_OpenALSoundSystem
  Scene/SoundNode.cpp
  Sound/OpenALSoundSystem.cpp
  Sound/SoundNodeVisitor.cpp
  Sound/SoundRenderer.cpp
)

TARGET_LINK_LIBRARIES(Extensions_OpenALSoundSystem
  Extensions_VorbisResource
  ${OPENAL_LIBRARY}
)
