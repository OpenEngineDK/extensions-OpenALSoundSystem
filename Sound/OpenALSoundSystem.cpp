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
	alDistanceModel(AL_LINEAR_DISTANCE);
    //    alSpeedOfSound(500);
    
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
        
        // Give camera orientation to openal
        Quaternion<float> rot = vv->GetDirection();
        Vector<3,float> up = rot.RotateVector(Vector<3,float>(0,1,0));
        Vector<3,float> dir = rot.RotateVector(Vector<3,float>(0,0,-1));
        float orientation[6];
        dir.ToArray(orientation);
        up.ToArray(&orientation[3]);
        alListenerfv(AL_ORIENTATION, orientation);

        // update listener velocity
        Vector<3,float> vel = (vvpos - prevPos) * (1/deltaTime*1000) * 0.0001;
        prevPos = vvpos;
        alListener3f(AL_VELOCITY, vel[0], vel[1], vel[2]);
        //logger.info << "listener vel: " << vel << logger.end;
   }

    //@todo optimize this by saving the ref, and reinit pos in visitor
    SoundNodeVisitor* snv = new SoundNodeVisitor(deltaTime);
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
//     PrintAttribute(AL_REFERENCE_DISTANCE);
//     PrintAttribute(AL_CONE_INNER_ANGLE);
//     PrintAttribute(AL_CONE_OUTER_ANGLE);
//     PrintAttribute(AL_ROLLOFF_FACTOR);
//     PrintAttribute(AL_MAX_DISTANCE);
//     PrintAttribute(AL_GAIN);
//     PrintAttribute(AL_ROLLOFF_FACTOR);
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

float OpenALSoundSystem::OpenALSound::GetGain() {
    float gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_GAIN, (ALfloat*)&gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALSound::SetMaxDistance(float dist) {
    ALCenum error;
    alSourcef(sourceID, AL_MAX_DISTANCE, (ALfloat)dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set max distance but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetMaxDistance() {
    float dist;
    ALCenum error;
    alGetSourcef(sourceID, AL_MAX_DISTANCE, (ALfloat*)&dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get max distance but got: "
		      + Convert::ToString(error));
    }
    return dist;
};

void OpenALSoundSystem::OpenALSound::SetLooping(bool loop) {
    ALCenum error;
    alSourcei(sourceID, AL_LOOPING, (ALboolean)loop);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set looping but got: "
		      + Convert::ToString(error));
    }
}

bool OpenALSoundSystem::OpenALSound::GetLooping() {
    ALint loop;
    ALCenum error;
    alGetSourcei(sourceID, AL_LOOPING, &loop);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get looping but got: "
		      + Convert::ToString(error));
    }
    return (loop != AL_FALSE);
}

void OpenALSoundSystem::OpenALSound::SetMinGain(float gain) {
    ALCenum error;
    alSourcef(sourceID, AL_MIN_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set min gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetMinGain() {
    ALfloat gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_MIN_GAIN, &gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get min gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALSound::SetMaxGain(float gain) {
    ALCenum error;
    alSourcef(sourceID, AL_MAX_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set max gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetMaxGain() {
    ALfloat gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_MAX_GAIN, &gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get max gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALSound::SetRolloffFactor(float rolloff) {
    ALCenum error;
    alSourcef(sourceID, AL_ROLLOFF_FACTOR, (ALfloat)rolloff);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set rolloff factor but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetRolloffFactor() {
    ALfloat rolloff;
    ALCenum error;
    alGetSourcef(sourceID, AL_ROLLOFF_FACTOR, &rolloff);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get rolloff factor but got: "
		      + Convert::ToString(error));
    }
    return rolloff;
}


void OpenALSoundSystem::OpenALSound::SetReferenceDistance(float dist) {
    ALCenum error;
    alSourcef(sourceID, AL_REFERENCE_DISTANCE, (ALfloat)dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set reference distance but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetReferenceDistance() {
    ALfloat dist;
    ALCenum error;
    alGetSourcef(sourceID, AL_REFERENCE_DISTANCE, &dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get reference distance but got: "
		      + Convert::ToString(error));
    }
    return dist;
}


void OpenALSoundSystem::OpenALSound::SetPitch(float pitch) {
    ALCenum error;
    alSourcef(sourceID, AL_PITCH, (ALfloat)pitch);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set pitch but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetPitch() {
    ALfloat pitch;
    ALCenum error;
    alGetSourcef(sourceID, AL_PITCH, &pitch);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get pitch but got: "
		      + Convert::ToString(error));
    }
    return pitch;
}

void OpenALSoundSystem::OpenALSound::SetDirection(Vector<3,float> dir) {
    ALCenum error;
    alSource3f(sourceID, AL_DIRECTION, (ALfloat)dir[0],(ALfloat)dir[1],(ALfloat)dir[2]);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set direction but got: "
		      + Convert::ToString(error));
    }
}

Vector<3,float> OpenALSoundSystem::OpenALSound::GetDirection() {
    ALfloat dir[3];
    ALCenum error;
    alGetSourcefv(sourceID, AL_PITCH, dir);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get direction but got: "
		      + Convert::ToString(error));
    }
    return Vector<3,float>(dir[0],dir[1],dir[2]);
}

void OpenALSoundSystem::OpenALSound::SetConeInnerAngle(float angle) {
    ALCenum error;
    alSourcef(sourceID, AL_CONE_INNER_ANGLE, (ALfloat)angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set cone inner angle but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetConeInnerAngle() {
    ALfloat angle;
    ALCenum error;
    alGetSourcefv(sourceID, AL_CONE_INNER_ANGLE, &angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get cone inner angle but got: "
		      + Convert::ToString(error));
    }
    return angle;
}

void OpenALSoundSystem::OpenALSound::SetConeOuterAngle(float angle) {
    ALCenum error;
    alSourcef(sourceID, AL_CONE_OUTER_ANGLE, (ALfloat)angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set cone outer angle but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetConeOuterAngle() {
    ALfloat angle;
    ALCenum error;
    alGetSourcefv(sourceID, AL_CONE_OUTER_ANGLE, &angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get cone outer angle but got: "
		      + Convert::ToString(error));
    }
    return angle;
}

ISound::PlaybackState OpenALSoundSystem::OpenALSound::GetPlaybackState() {
    ALint state;
    ALCenum error;
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get playback state but got: "
		      + Convert::ToString(error));
    }
    
    switch (state) {
    case AL_INITIAL: return INITIAL;
    case AL_PLAYING: return PLAYING;
    case AL_PAUSED: return PAUSED;
    case AL_STOPPED: return STOPPED;
    }
    return INITIAL; // TODO: what todo here???
}

void OpenALSoundSystem::OpenALSound::SetSampleOffset(int samples) {
    ALCenum error;
    alSourcei(sourceID, AL_SAMPLE_OFFSET, (ALint)samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by sample but got: "
		      + Convert::ToString(error));
    }
}

int OpenALSoundSystem::OpenALSound::GetSampleOffset() {
    ALint samples;
    ALCenum error;
    alGetSourcei(sourceID, AL_SAMPLE_OFFSET, &samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by sample but got: "
		      + Convert::ToString(error));
    }
    return samples;
}

void OpenALSoundSystem::OpenALSound::SetTimeOffset(float seconds) {
    ALCenum error;
    alSourcei(sourceID, AL_SEC_OFFSET, (ALfloat)seconds);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by seconds but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALSound::GetTimeOffset() {
    ALint seconds;
    ALCenum error;
    alGetSourcei(sourceID, AL_SEC_OFFSET, &seconds);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by time but got: "
		      + Convert::ToString(error));
    }
    return seconds;
}

void OpenALSoundSystem::OpenALSound::SetVelocity(Vector<3,float> vel) {
    ALCenum error;
    ALfloat v[3];
    vel.ToArray(v);
    alSourcefv(sourceID, AL_VELOCITY, v);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set velocity but got: "
		      + Convert::ToString(error));
    }
    //logger.info << "source vel: " << vel << logger.end;
}

Vector<3,float> OpenALSoundSystem::OpenALSound::GetVelocity() {
    ALfloat v[3];
    ALCenum error;
    alGetSourcefv(sourceID, AL_VELOCITY, v);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by time but got: "
		      + Convert::ToString(error));
    }
    return Vector<3,float>(v[0],v[1],v[2]);
}


} // NS Sound
} // NS OpenEngine
