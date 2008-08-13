
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

IMonoSound* OpenALSoundSystem::CreateMonoSound(ISoundResourcePtr resource) {
    OpenALMonoSound* res = new OpenALMonoSound(resource);
    res->Initialize();
    return res;
}

IStereoSound* OpenALSoundSystem::CreateStereoSound(ISoundResourcePtr resource) {
    OpenALStereoSound* res = new OpenALStereoSound(resource);
    res->Initialize();
    return res;
}

void OpenALSoundSystem::SetRoot(ISceneNode* node) {
    theroot = node;
}

void OpenALSoundSystem::Initialize() {}

void OpenALSoundSystem::Init() {

	if (alGetString(AL_VENDOR))
		logger.info << "AL_VENDOR: " << alGetString(AL_VENDOR) << logger.end;
	if (alGetString(AL_RENDERER))
		logger.info << "AL_RENDERER: " << alGetString(AL_RENDERER) << logger.end;
	if (alGetString(AL_VERSION))
		logger.info << "AL_VERSION: " << alGetString(AL_VERSION) << logger.end;
	if (alGetString(AL_EXTENSIONS))
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
        
        // Give camera orientation to openal
        Quaternion<float> rot = vv->GetDirection();
        Vector<3,float> up = rot.RotateVector(Vector<3,float>(0,1,0));
        Vector<3,float> dir = rot.RotateVector(Vector<3,float>(0,0,-1));
        float orientation[6];
        dir.ToArray(orientation);
        up.ToArray(&orientation[3]);
        alListenerfv(AL_ORIENTATION, orientation);

        // update listener velocity
        Vector<3,float> vel = (vvpos - prevPos) * (1/deltaTime*1000);
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

OpenALSoundSystem::OpenALMonoSound::OpenALMonoSound(ISoundResourcePtr resource) : resource(resource) {
}

void OpenALSoundSystem::OpenALMonoSound::Initialize() {
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

OpenALSoundSystem::OpenALMonoSound::~OpenALMonoSound() {
    alDeleteBuffers(1, &bufferID);
    alDeleteSources(1, &sourceID);
}

void OpenALSoundSystem::OpenALMonoSound::Play() {
    alDistanceModel(AL_LINEAR_DISTANCE);
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

void OpenALSoundSystem::OpenALMonoSound::PrintAttribute(ALenum e) {
    float* where = new float[3];
    where[0] = where[1] = where[2] = 0.0;
    alGetSourcefv(sourceID, e, where);
    Vector<3,float> vec = Vector<3,float>(where[0],where[1],where[2]);
    delete where;
    logger.info << "" << EnumToString(e) << ": " << vec << logger.end;
}

string OpenALSoundSystem::OpenALMonoSound::EnumToString(ALenum e) {
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

void OpenALSoundSystem::OpenALMonoSound::Stop() {
    alSourceStop(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to stop source but got error: "
		      + Convert::ToString(error));
    }
}

void OpenALSoundSystem::OpenALMonoSound::Pause() {
    alSourcePause(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to pause source but got error: "
		      + Convert::ToString(error));
    }
}

void OpenALSoundSystem::OpenALMonoSound::SetRotation(Quaternion<float> dir) {
  //@todo apply rotation via openal
    this->dir = dir;
}

Quaternion<float> OpenALSoundSystem::OpenALMonoSound::GetRotation() {
    return dir;
}

void OpenALSoundSystem::OpenALMonoSound::SetPosition(Vector<3,float> pos) {
    //logger.info << "pos: " << pos << logger.end;
    alSource3f(sourceID, AL_POSITION, pos[0], pos[1], pos[2]);

    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set position but got: "
		      + Convert::ToString(error));
    }
    this->pos = pos;
}

Vector<3,float> OpenALSoundSystem::OpenALMonoSound::GetPosition() {
    return pos;
}

ISoundResourcePtr OpenALSoundSystem::OpenALMonoSound::GetResource() {
    return resource;
}

unsigned int OpenALSoundSystem::OpenALMonoSound::GetID() {
    return sourceID;
}

void OpenALSoundSystem::OpenALMonoSound::SetID(unsigned int id) {
    sourceID = id;
}

void OpenALSoundSystem::OpenALMonoSound::SetGain(float gain) {
    ALCenum error;
    alSourcef(sourceID, AL_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetGain() {
    float gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_GAIN, (ALfloat*)&gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALMonoSound::SetMaxDistance(float dist) {
    ALCenum error;
    alSourcef(sourceID, AL_MAX_DISTANCE, (ALfloat)dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set max distance but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetMaxDistance() {
    float dist;
    ALCenum error;
    alGetSourcef(sourceID, AL_MAX_DISTANCE, (ALfloat*)&dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get max distance but got: "
		      + Convert::ToString(error));
    }
    return dist;
};

void OpenALSoundSystem::OpenALMonoSound::SetLooping(bool loop) {
    ALCenum error;
    alSourcei(sourceID, AL_LOOPING, (ALboolean)loop);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set looping but got: "
		      + Convert::ToString(error));
    }
}

bool OpenALSoundSystem::OpenALMonoSound::GetLooping() {
    ALint loop;
    ALCenum error;
    alGetSourcei(sourceID, AL_LOOPING, &loop);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get looping but got: "
		      + Convert::ToString(error));
    }
    return (loop != AL_FALSE);
}

void OpenALSoundSystem::OpenALMonoSound::SetMinGain(float gain) {
    ALCenum error;
    alSourcef(sourceID, AL_MIN_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set min gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetMinGain() {
    ALfloat gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_MIN_GAIN, &gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get min gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALMonoSound::SetMaxGain(float gain) {
    ALCenum error;
    alSourcef(sourceID, AL_MAX_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set max gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetMaxGain() {
    ALfloat gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_MAX_GAIN, &gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get max gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALMonoSound::SetRolloffFactor(float rolloff) {
    ALCenum error;
    alSourcef(sourceID, AL_ROLLOFF_FACTOR, (ALfloat)rolloff);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set rolloff factor but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetRolloffFactor() {
    ALfloat rolloff;
    ALCenum error;
    alGetSourcef(sourceID, AL_ROLLOFF_FACTOR, &rolloff);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get rolloff factor but got: "
		      + Convert::ToString(error));
    }
    return rolloff;
}


void OpenALSoundSystem::OpenALMonoSound::SetReferenceDistance(float dist) {
    ALCenum error;
    alSourcef(sourceID, AL_REFERENCE_DISTANCE, (ALfloat)dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set reference distance but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetReferenceDistance() {
    ALfloat dist;
    ALCenum error;
    alGetSourcef(sourceID, AL_REFERENCE_DISTANCE, &dist);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get reference distance but got: "
		      + Convert::ToString(error));
    }
    return dist;
}


void OpenALSoundSystem::OpenALMonoSound::SetPitch(float pitch) {
    ALCenum error;
    alSourcef(sourceID, AL_PITCH, (ALfloat)pitch);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set pitch but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetPitch() {
    ALfloat pitch;
    ALCenum error;
    alGetSourcef(sourceID, AL_PITCH, &pitch);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get pitch but got: "
		      + Convert::ToString(error));
    }
    return pitch;
}

void OpenALSoundSystem::OpenALMonoSound::SetDirection(Vector<3,float> dir) {
    ALCenum error;
    alSource3f(sourceID, AL_DIRECTION, (ALfloat)dir[0],(ALfloat)dir[1],(ALfloat)dir[2]);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set direction but got: "
		      + Convert::ToString(error));
    }
}

Vector<3,float> OpenALSoundSystem::OpenALMonoSound::GetDirection() {
    ALfloat dir[3];
    ALCenum error;
    alGetSourcefv(sourceID, AL_PITCH, dir);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get direction but got: "
		      + Convert::ToString(error));
    }
    return Vector<3,float>(dir[0],dir[1],dir[2]);
}

void OpenALSoundSystem::OpenALMonoSound::SetConeInnerAngle(float angle) {
    ALCenum error;
    alSourcef(sourceID, AL_CONE_INNER_ANGLE, (ALfloat)angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set cone inner angle but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetConeInnerAngle() {
    ALfloat angle;
    ALCenum error;
    alGetSourcefv(sourceID, AL_CONE_INNER_ANGLE, &angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get cone inner angle but got: "
		      + Convert::ToString(error));
    }
    return angle;
}

void OpenALSoundSystem::OpenALMonoSound::SetConeOuterAngle(float angle) {
    ALCenum error;
    alSourcef(sourceID, AL_CONE_OUTER_ANGLE, (ALfloat)angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set cone outer angle but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetConeOuterAngle() {
    ALfloat angle;
    ALCenum error;
    alGetSourcefv(sourceID, AL_CONE_OUTER_ANGLE, &angle);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get cone outer angle but got: "
		      + Convert::ToString(error));
    }
    return angle;
}

ISound::PlaybackState OpenALSoundSystem::OpenALMonoSound::GetPlaybackState() {
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

void OpenALSoundSystem::OpenALMonoSound::SetSampleOffset(int samples) {
    ALCenum error;
    alSourcei(sourceID, AL_SAMPLE_OFFSET, (ALint)samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by sample but got: "
		      + Convert::ToString(error));
    }
}

int OpenALSoundSystem::OpenALMonoSound::GetSampleOffset() {
    ALint samples;
    ALCenum error;
    alGetSourcei(sourceID, AL_SAMPLE_OFFSET, &samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by sample but got: "
		      + Convert::ToString(error));
    }
    return samples;
}

void OpenALSoundSystem::OpenALMonoSound::SetTimeOffset(float seconds) {
    ALCenum error;
    alSourcei(sourceID, AL_SEC_OFFSET, (ALfloat)seconds);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by seconds but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetTimeOffset() {
    ALint seconds;
    ALCenum error;
    alGetSourcei(sourceID, AL_SEC_OFFSET, &seconds);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by time but got: "
		      + Convert::ToString(error));
    }
    return seconds;
}

void OpenALSoundSystem::OpenALMonoSound::SetVelocity(Vector<3,float> vel) {
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

Vector<3,float> OpenALSoundSystem::OpenALMonoSound::GetVelocity() {
    ALfloat v[3];
    ALCenum error;
    alGetSourcefv(sourceID, AL_VELOCITY, v);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by time but got: "
		      + Convert::ToString(error));
    }
    return Vector<3,float>(v[0],v[1],v[2]);
}

OpenALSoundSystem::OpenALStereoSound::OpenALStereoSound(ISoundResourcePtr resource) {
	ress = resource;
}

OpenALSoundSystem::OpenALStereoSound::~OpenALStereoSound() {
	left->~IMonoSound();
	right->~IMonoSound();
}

void OpenALSoundSystem::OpenALStereoSound::Initialize() {

	SoundFormat format = ress->GetFormat();
	
	if (format == Resources::MONO_8BIT || format == Resources::MONO_16BIT)
		throw Exception("tried to make a stereo source with a mono sound pointer");

	char* leftbuffer = new char[ress->GetBufferSize()/2]; 
	char* rightbuffer = new char[ress->GetBufferSize()/2]; 

	char* data = ress->GetBuffer();    	  	

	if (format == Resources::STEREO_8BIT) {
		
		for (unsigned int i=0; i < (ress->GetBufferSize())/2; i++) 
		{ 
			leftbuffer[i] = data[i*2]; // left chan
			rightbuffer[i] = data[i*2+1]; // right chan 
		} 

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_8BIT)));
		left->Initialize();
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_8BIT)));
		right->Initialize();
	}
	else if (format == Resources::STEREO_16BIT) {
		
		int j = 0;
		for (unsigned int i=0; i < (ress->GetBufferSize()); i += 4)  
		{ 			
			leftbuffer[j] = data[i]; // left chan
			leftbuffer[j+1] = data[i+1]; // left chan
			rightbuffer[j] = data[i+2]; // right chan 
			rightbuffer[j+1] = data[i+3]; // right chan 
			j += 2;
		} 

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_16BIT)));
		left->Initialize();
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_16BIT)));
		right->Initialize();
	}
	
}

void OpenALSoundSystem::OpenALStereoSound::Play() {

	ALuint list[2];
	list[0] = left->GetID();
	list[1] = right->GetID();

	alSourcePlayv(2, &list[0]);

}

void OpenALSoundSystem::OpenALStereoSound::Stop() {

	ALuint list[2];
	list[0] = left->GetID();
	list[1] = right->GetID();

	alSourceStopv(2, &list[0]);

}

void OpenALSoundSystem::OpenALStereoSound::Pause() {

	ALuint list[2];
	list[0] = left->GetID();
	list[1] = right->GetID();

	alSourcePausev(2, &list[0]);

}

IMonoSound* OpenALSoundSystem::OpenALStereoSound::GetLeft() {
	return left;
}

IMonoSound* OpenALSoundSystem::OpenALStereoSound::GetRight() {
	return right;
}

char* OpenALSoundSystem::CustomSoundResource::GetBuffer() {
	return data;
}

unsigned int OpenALSoundSystem::CustomSoundResource::GetBufferSize() {
	return size;
}

int OpenALSoundSystem::CustomSoundResource::GetFrequency() {
	return frequancy;
}

SoundFormat OpenALSoundSystem::CustomSoundResource::GetFormat() {
	return format;
}

OpenALSoundSystem::CustomSoundResource::CustomSoundResource(char* newdata, unsigned int newsize, int newfreq, SoundFormat newformat) {
	data = newdata;
	size = newsize;
	frequancy = newfreq;
	format = newformat;
}

OpenALSoundSystem::CustomSoundResource::~CustomSoundResource() {

}

void OpenALSoundSystem::CustomSoundResource::Load() {

}

void OpenALSoundSystem::CustomSoundResource::Unload() {

}

} // NS Sound
} // NS OpenEngine

