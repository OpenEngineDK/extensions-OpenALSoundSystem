
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
using namespace OpenEngine::Core;
  

OpenALSoundSystem::OpenALSoundSystem(ISceneNode* root, IViewingVolume* vv): 
    theroot(root), 
    vv(vv), 
    initialized(false) {
    
    ALCdevice* thedevice = alcOpenDevice(NULL);
    if (thedevice) {
        ALCcontext* thecontext = alcCreateContext(thedevice, NULL);
        alcMakeContextCurrent(thecontext);
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        initialized = true;
        //alDistanceModel(AL_EXPONENT_DISTANCE);
        //	logger.info << "OpenAL has been initialized" << logger.end;
    }
}

OpenALSoundSystem::~OpenALSoundSystem() {
}

IMonoSound* OpenALSoundSystem::CreateMonoSound(ISoundResourcePtr resource) {
    OpenALMonoSound* sound = new OpenALMonoSound(resource, this);
    sound->Initialize();
    return sound;
}

IStereoSound* OpenALSoundSystem::CreateStereoSound(ISoundResourcePtr resource) {
    OpenALStereoSound* sound = new OpenALStereoSound(resource, this);
    sound->Initialize();
    return sound;
}

void OpenALSoundSystem::SetRoot(ISceneNode* node) {
    theroot = node;
}

void OpenALSoundSystem::SetMasterGain(float gain) {
	if (!initialized)
		return;

	if (gain < 0.0)
		gain = 0.0;

    ALCenum error;
    alListenerf(AL_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set gain but got: "
		      + Convert::ToString(error));
    }
    
}

float OpenALSoundSystem::GetMasterGain() {
	if (!initialized)
		return 0.0;

    float gain;
    ALCenum error;
    alGetListenerf(AL_GAIN, (ALfloat*)&gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
   
}

void OpenALSoundSystem::Handle(InitializeEventArg agr) {}

void OpenALSoundSystem::Init() {

    // 	if (alGetString(AL_VENDOR))
    // 		logger.info << "AL_VENDOR: " << alGetString(AL_VENDOR) << logger.end;
    // 	if (alGetString(AL_RENDERER))
    // 		logger.info << "AL_RENDERER: " << alGetString(AL_RENDERER) << logger.end;
    // 	if (alGetString(AL_VERSION))
    // 		logger.info << "AL_VERSION: " << alGetString(AL_VERSION) << logger.end;
    // 	if (alGetString(AL_EXTENSIONS))
    // 		logger.info << "AL_EXTENSIONS: " << alGetString(AL_EXTENSIONS) << logger.end;
    
    //     const ALCchar* defaultdevice = 
    //       alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );
    //     logger.info << "default audio device: " << defaultdevice << logger.end;
    
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void OpenALSoundSystem::Handle(ProcessEventArg arg) {
	if (!initialized)
		return;

    const float deltaTime = arg.approx;
	
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
        // @todo: salomon alListener3f(AL_VELOCITY, vel[0], vel[1], vel[2]);
        //logger.info << "listener vel: " << vel << logger.end;
   }

    //@todo optimize this by saving the ref, and reinit pos in visitor
    SoundNodeVisitor* snv = new SoundNodeVisitor(deltaTime);
    theroot->Accept(*snv);
    delete snv;
}

void OpenALSoundSystem::Handle(DeinitializeEventArg arg) {
	if (!initialized)
		return;

    ALCcontext* thecontext = alcGetCurrentContext();
    ALCdevice* thedevice = alcGetContextsDevice(thecontext);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(thecontext);
    alcCloseDevice(thedevice);

    initialized = false;
}

OpenALSoundSystem::OpenALMonoSound::OpenALMonoSound(ISoundResourcePtr resource, OpenALSoundSystem* soundsystem) : 
    resource(resource),
    soundsystem(soundsystem) {}
    
void OpenALSoundSystem::OpenALMonoSound::Initialize() {
	if (!soundsystem->initialized)
		return;

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

//     int totalsize = 0;
//     alGetBufferi(bufferID, AL_SIZE, &totalsize);
//     if ((error = alGetError()) != AL_NO_ERROR) {
//         throw Exception("tried to get the size of the buffer but got: "
// 		      + Convert::ToString(error));
//     }

//     int freq = resource->GetFrequency();

//     SoundFormat format = resource->GetFormat();
	
//     if (format == Resources::MONO_16BIT)
//       totalsize = totalsize/2;

//     length = totalsize/freq;

}

OpenALSoundSystem::OpenALMonoSound::~OpenALMonoSound() {
	if (!soundsystem->initialized)
		return;

    // TODO: is it ok to delete buffer here?
    alDeleteBuffers(1, &bufferID);
    alDeleteSources(1, &sourceID);
}

void OpenALSoundSystem::OpenALMonoSound::Play() {
	if (!soundsystem->initialized)
		return;

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

void OpenALSoundSystem::OpenALMonoSound::PrintAttribute(ALenum e) {
	if (!soundsystem->initialized)
		return;

    float* where = new float[3];
    where[0] = where[1] = where[2] = 0.0;
    alGetSourcefv(sourceID, e, where);
    Vector<3,float> vec = Vector<3,float>(where[0],where[1],where[2]);
    delete where;
    //logger.info << "" << EnumToString(e) << ": " << vec << logger.end;
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
	if (!soundsystem->initialized)
		return;

    alSourceStop(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to stop source but got error: "
		      + Convert::ToString(error));
    }
}

void OpenALSoundSystem::OpenALMonoSound::Pause() {
	if (!soundsystem->initialized)
		return;

    alSourcePause(sourceID);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to pause source but got error: "
		      + Convert::ToString(error));
    }
}

bool OpenALSoundSystem::OpenALMonoSound::IsPlaying() {
    return true;
}

unsigned int OpenALSoundSystem::OpenALMonoSound::GetLengthInSamples() {
    return 0;
}


Time OpenALSoundSystem::OpenALMonoSound::GetLength() {
    return Time();
}

void OpenALSoundSystem::OpenALMonoSound::SetPosition(Vector<3,float> pos) {
	if (!soundsystem->initialized)
		return;

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

// void OpenALSoundSystem::OpenALMonoSound::SetID(unsigned int id) {
//     sourceID = id;
// }

void OpenALSoundSystem::OpenALMonoSound::SetGain(float gain) {
	if (!soundsystem->initialized)
		return;

    ALCenum error;
    alSourcef(sourceID, AL_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetGain() {
	if (!soundsystem->initialized)
		return 0.0;

    float gain;
    ALCenum error;
    alGetSourcef(sourceID, AL_GAIN, (ALfloat*)&gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get gain but got: "
		      + Convert::ToString(error));
    }
    return gain;
}

void OpenALSoundSystem::OpenALMonoSound::SetLooping(bool loop) {
	if (!soundsystem->initialized)
		return;

    ALCenum error;
    alSourcei(sourceID, AL_LOOPING, (ALboolean)loop);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set looping but got: "
		      + Convert::ToString(error));
    }
}

bool OpenALSoundSystem::OpenALMonoSound::GetLooping() {
	if (!soundsystem->initialized)
		return false;

    ALint loop;
    ALCenum error;
    alGetSourcei(sourceID, AL_LOOPING, &loop);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get looping but got: "
		      + Convert::ToString(error));
    }
    return (loop != AL_FALSE);
}

void OpenALSoundSystem::OpenALMonoSound::SetElapsedSamples(unsigned int samples) {
	if (!soundsystem->initialized)
		return;

    ALCenum error;
    alSourcei(sourceID, AL_SAMPLE_OFFSET, (ALint)samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by sample but got: "
		      + Convert::ToString(error));
    }
}

unsigned int OpenALSoundSystem::OpenALMonoSound::GetElapsedSamples() {
	if (!soundsystem->initialized)
		return 0;

    ALint samples;
    ALCenum error;
    alGetSourcei(sourceID, AL_SAMPLE_OFFSET, &samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by sample but got: "
		      + Convert::ToString(error));
    }
    return samples;
}

void OpenALSoundSystem::OpenALMonoSound::SetElapsedTime(Time time) {
	if (!soundsystem->initialized)
		return;
    
    float seconds = ((float)time.AsInt())/1000000.0;
    ALCenum error;
    alSourcef(sourceID, AL_SEC_OFFSET, (ALfloat)seconds);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by seconds but got: "
		      + Convert::ToString(error));
    }
}

Time OpenALSoundSystem::OpenALMonoSound::GetElapsedTime() {
	if (!soundsystem->initialized)
		return 0.0;

    ALfloat seconds;
    ALCenum error;
    alGetSourcef(sourceID, AL_SEC_OFFSET, &seconds);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by time but got: "
		      + Convert::ToString(error));
    }
    uint64_t sec = (int)seconds; 
    uint32_t usec = (int)((seconds - sec)*1000000.0);
    return Time(sec,usec);
}

void OpenALSoundSystem::OpenALMonoSound::SetMaxDistance(float distance) {
	if (!soundsystem->initialized)
		return;
    
    ALCenum error;
    alSourcef(sourceID, AL_MAX_DISTANCE, (ALfloat)distance);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set max distance but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetMaxDistance() {
	if (!soundsystem->initialized)
		return 0.0;

    ALfloat distance;
    ALCenum error;
    alGetSourcef(sourceID, AL_SEC_OFFSET, &distance);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get max distance but got: "
		      + Convert::ToString(error));
    }
    return distance;
}


void OpenALSoundSystem::OpenALMonoSound::SetVelocity(Vector<3,float> vel) {
	if (!soundsystem->initialized)
		return;

    ALCenum error;
    ALfloat v[3];
    vel.ToArray(v);
    // @todo: salomon alSourcefv(sourceID, AL_VELOCITY, v);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set velocity but got: "
		      + Convert::ToString(error));
    }
    //logger.info << "source vel: " << vel << logger.end;
}

Vector<3,float> OpenALSoundSystem::OpenALMonoSound::GetVelocity() {
	if (!soundsystem->initialized)
		return Vector<3,float>(0, 0, 0);

    ALfloat v[3];
    v[0] = v[1] = v[2] = 0;
    ALCenum error;
    // @todo: salomon alGetSourcefv(sourceID, AL_VELOCITY, v);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to get offset by time but got: "
		      + Convert::ToString(error));
    }
    return Vector<3,float>(v[0],v[1],v[2]);
}

    OpenALSoundSystem::OpenALStereoSound::OpenALStereoSound(ISoundResourcePtr resource, OpenALSoundSystem* soundsystem): soundsystem(soundsystem) {
	ress = resource;
}

OpenALSoundSystem::OpenALStereoSound::~OpenALStereoSound() {
	delete left;
	delete right;
}

void OpenALSoundSystem::OpenALStereoSound::SetGain(float gain) {
    if (!soundsystem->initialized)
 		return;

    left->SetGain(gain);
    right->SetGain(gain);

//     ALCenum error;
//     alSourcef(left->GetID(), AL_GAIN, (ALfloat)gain);
//     alSourcef(right->GetID(), AL_GAIN, (ALfloat)gain);
//     if ((error = alGetError()) != AL_NO_ERROR) {
//       throw Exception("tried to set gain but got: "
// 		      + Convert::ToString(error));
//     }
}

float OpenALSoundSystem::OpenALStereoSound::GetGain() {
	if (!soundsystem->initialized)
		return 0.0;

    //@TODO: what if left and right are not the same
    return left->GetGain();
}
    
void OpenALSoundSystem::OpenALStereoSound::SetLooping(bool loop) {
    left->SetLooping(loop);
    right->SetLooping(loop);
}

bool OpenALSoundSystem::OpenALStereoSound::GetLooping() {
    return left->GetLooping();
}

unsigned int OpenALSoundSystem::OpenALStereoSound::GetLengthInSamples() {
    return left->GetLengthInSamples();
}

Time OpenALSoundSystem::OpenALStereoSound::GetLength() {
    return left->GetLength();
}

void OpenALSoundSystem::OpenALStereoSound::SetElapsedSamples(unsigned int samples) {
    //@todo: synchronization issues
    left->SetElapsedSamples(samples);
    right->SetElapsedSamples(samples);
}

unsigned int OpenALSoundSystem::OpenALStereoSound::GetElapsedSamples() {
    return left->GetElapsedSamples();
}

void OpenALSoundSystem::OpenALStereoSound::SetElapsedTime(Time time) {
    //@todo: synchronization issues
    left->SetElapsedTime(time);
    right->SetElapsedTime(time);
}
    
Time OpenALSoundSystem::OpenALStereoSound::GetElapsedTime() {
    return left->GetElapsedTime();
}

  
void OpenALSoundSystem::OpenALStereoSound::Initialize() {
	if (!soundsystem->initialized)
		return;

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

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_8BIT)),soundsystem);
		left->Initialize();
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_8BIT)),soundsystem);
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

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_16BIT)), soundsystem);
		left->Initialize();
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), Resources::MONO_16BIT)),soundsystem);
		right->Initialize();
	}
	
}

void OpenALSoundSystem::OpenALStereoSound::Play() {
	if (!soundsystem->initialized)
		return;

	ALuint list[2];
	list[0] = left->GetID();
	list[1] = right->GetID();

	alSourcePlayv(2, &list[0]);

}

void OpenALSoundSystem::OpenALStereoSound::Stop() {
	if (!soundsystem->initialized)
		return;

	ALuint list[2];
	list[0] = left->GetID();
	list[1] = right->GetID();

	alSourceStopv(2, &list[0]);

}

void OpenALSoundSystem::OpenALStereoSound::Pause() {
	if (!soundsystem->initialized)
		return;

	ALuint list[2];
	list[0] = left->GetID();
	list[1] = right->GetID();

	alSourcePausev(2, &list[0]);

}

bool OpenALSoundSystem::OpenALStereoSound::IsPlaying() {
	if (!soundsystem->initialized)
		return false;

    return false;
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
	return frequency;
}

SoundFormat OpenALSoundSystem::CustomSoundResource::GetFormat() {
	return format;
}

OpenALSoundSystem::CustomSoundResource::CustomSoundResource(char* data, unsigned int size, int freq, SoundFormat format) {
	this->data = data;
	this->size = size;
	this->frequency = freq;
	this->format = format;
}

OpenALSoundSystem::CustomSoundResource::~CustomSoundResource() {

}

void OpenALSoundSystem::CustomSoundResource::Load() {

}

void OpenALSoundSystem::CustomSoundResource::Unload() {

}

} // NS Sound
} // NS OpenEngine

