// OpenAL sound manager implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Sound/OpenALSoundSystem.h>

#include <Logging/Logger.h>
#include <Core/Exceptions.h>
#include <Sound/SoundNodeVisitor.h>
#include <Sound/ISound.h>
#include <Utils/Convert.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Core::Exception;
using OpenEngine::Utils::Convert;

OpenALSoundSystem::OpenALSoundSystem(ISceneNode* root, IViewingVolume* vv): 
    theroot(root), 
    vv(vv) {
    Init();
}

OpenALSoundSystem::~OpenALSoundSystem() {
}

ISound* OpenALSoundSystem::CreateSound(ISoundResourcePtr resource) {
    OpenALSound* res = new OpenALSound(resource);
    res->Initialize();
    return res;
}

void OpenALSoundSystem::SetRoot(ISceneNode* node) {
    theroot = node;
}

void OpenALSoundSystem::Initialize() {}

void OpenALSoundSystem::Init() {
    logger.info << "AL_VENDOR: " << alGetString(AL_VENDOR) << logger.end;
    logger.info << "AL_RENDERER: " << alGetString(AL_RENDERER) << logger.end;
    logger.info << "AL_VERSION: " << alGetString(AL_VERSION) << logger.end;
    logger.info << "AL_EXTENSIONS: " << alGetString(AL_EXTENSIONS) << logger.end;

    const ALCchar* defaultdevice = 
      alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );
    logger.info << "default audio device: " << defaultdevice << logger.end;

    ALCdevice* thedevice = alcOpenDevice(NULL);
    if (thedevice) {
        ALCcontext* thecontext = alcCreateContext(thedevice, NULL);
	alcMakeContextCurrent(thecontext);
	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	//alDistanceModel(AL_EXPONENT_DISTANCE);
	logger.info << "OpenAL has been initialized" << logger.end;
    }
    else
      throw new Exception("Could not initalize sound module");
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void OpenALSoundSystem::Process(const float deltaTime, const float percent) {
    if (vv != NULL) {
        Vector<3,float> vvpos = vv->GetPosition();
        alListener3f(AL_POSITION, vvpos[0], vvpos[1], vvpos[2]);
	//logger.info << "viewing from: " << vvpos << logger.end;
    }

    //@todo optimize this by saving the ref, and reinit pos in visitor
    SoundNodeVisitor* snv = new SoundNodeVisitor();
    theroot->Accept(*snv);
    delete snv;
}

void OpenALSoundSystem::Deinitialize() {
    ALCcontext* thecontext = alcGetCurrentContext();
    ALCdevice* thedevice = alcGetContextsDevice(thecontext);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(thecontext);
    alcCloseDevice(thedevice);
}

bool OpenALSoundSystem::IsTypeOf(const std::type_info& inf) {
    return ((typeid(OpenALSoundSystem) == inf) || ISoundSystem::IsTypeOf(inf));
}

OpenALSoundSystem::OpenALSound::OpenALSound(ISoundResourcePtr resource) : resource(resource) {
}

void OpenALSoundSystem::OpenALSound::Initialize() {
    //generate the source
    ALuint source;
    alGenSources(1, &source);

    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to gen source but got: "
			+ Convert::ToString(error));
    }

    //attach the buffer
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, resource->GetFormat(), resource->GetBuffer(),
		 resource->GetBufferSize(), resource->GetFrequency());
    alSourcei(source, AL_BUFFER, bufferID);
    
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to bind source and buffer together but got: "
		      + Convert::ToString(error));
    }
    sourceID = source;
}

OpenALSoundSystem::OpenALSound::~OpenALSound() {
    alDeleteBuffers(1, &bufferID);
    alDeleteSources(1, &sourceID);
}

void OpenALSoundSystem::OpenALSound::Play() {
    alSourcePlay(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to play source but got error: "
		      + Convert::ToString(error));
    }

    //alSourcef(sourceID, AL_MAX_DISTANCE, 0.001 );
    PrintAttribute(AL_REFERENCE_DISTANCE);
    PrintAttribute(AL_CONE_INNER_ANGLE);
    PrintAttribute(AL_CONE_OUTER_ANGLE);
    PrintAttribute(AL_ROLLOFF_FACTOR);
    PrintAttribute(AL_MAX_DISTANCE);
    PrintAttribute(AL_GAIN);
    PrintAttribute(AL_ROLLOFF_FACTOR);
}

void OpenALSoundSystem::OpenALSound::PrintAttribute(ALenum e) {
    float* where = new float[3];
    where[0] = where[1] = where[2] = 0.0;
    alGetSourcefv(sourceID, e, where);
    Vector<3,float> vec = Vector<3,float>(where[0],where[1],where[2]);
    delete where;
    logger.info << "" << EnumToString(e) << ": " << vec << logger.end;
}

string OpenALSoundSystem::OpenALSound::EnumToString(ALenum e) {
  string str;
  switch(e) {
  case AL_REFERENCE_DISTANCE:
    str = "AL_REFERENCE_DISTANCE";
    break;
  case AL_CONE_INNER_ANGLE:
    str = "AL_CONE_INNER_ANGLE";
    break;
  case AL_CONE_OUTER_ANGLE:
    str = "AL_CONE_OUTER_ANGLE";
    break;
  case AL_ROLLOFF_FACTOR:
    str = "AL_ROLLOFF_FACTOR";
    break;
  case AL_MAX_DISTANCE:
    str = "AL_MAX_DISTANCE";
    break;
  case AL_GAIN:
    str = "AL_GAIN";
    break;
  default:
    str = "unknown";
  }
  return str;
}

void OpenALSoundSystem::OpenALSound::Stop() {
    alSourceStop(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to stop source but got error: "
		      + Convert::ToString(error));
    }
}

void OpenALSoundSystem::OpenALSound::Pause() {
    alSourcePause(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to pause source but got error: "
		      + Convert::ToString(error));
    }
}

void OpenALSoundSystem::OpenALSound::SetRotation(Quaternion<float> dir) {
  //@todo apply rotation via openal
    this->dir = dir;
}

Quaternion<float> OpenALSoundSystem::OpenALSound::GetRotation() {
    return dir;
}

void OpenALSoundSystem::OpenALSound::SetPosition(Vector<3,float> pos) {
    //logger.info << "pos: " << pos << logger.end;
    alSource3f(sourceID, AL_POSITION, pos[0], pos[1], pos[2]);

    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set position but got: "
		      + Convert::ToString(error));
    }
    this->pos = pos;
}

Vector<3,float> OpenALSoundSystem::OpenALSound::GetPosition() {
    return pos;
}

ISoundResourcePtr OpenALSoundSystem::OpenALSound::GetResource() {
    return resource;
}

unsigned int OpenALSoundSystem::OpenALSound::GetID() {
    return sourceID;
}

void OpenALSoundSystem::OpenALSound::SetID(unsigned int id) {
    sourceID = id;
}

void OpenALSoundSystem::OpenALSound::SetGain(float gain) {
    ALCenum error;
    alSourcef(sourceID, AL_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set gain but got: "
		      + Convert::ToString(error));
    }
}

} // NS Sound
} // NS OpenEngine
