
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
#include <Math/Math.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Core::Exception;
using OpenEngine::Utils::Convert;
using namespace OpenEngine::Core;
using namespace OpenEngine::Math;  

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
        alDistanceModel(AL_LINEAR_DISTANCE);
        //logger.info << "OpenAL has been initialized" << logger.end;
    }
}

OpenALSoundSystem::~OpenALSoundSystem() {
}

ISound* OpenALSoundSystem::CreateSound(ISoundResourcePtr resource) {
    SoundFormat format = resource->GetFormat();
    if (format == MONO) {
        OpenALMonoSound* sound = new OpenALMonoSound(resource, this);
        sound->Initialize();
        return sound;
    } else if (format == STEREO) {
        OpenALStereoSound* sound = new OpenALStereoSound(resource, this);
        sound->Initialize();
        return sound;
    }
    else
        throw Exception("unsupported sound format");
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


    ALuint format = 0;
    if(resource->GetFormat() == MONO) {
        if (resource->GetBitsPerSample() == 8)
            format = AL_FORMAT_MONO8;
        else if (resource->GetBitsPerSample() == 16)
            format = AL_FORMAT_MONO16;
        else
            throw Exception("unknown number of bits per sample");
    }
    else if(resource->GetFormat() == STEREO) {
        if (resource->GetBitsPerSample() == 8)
            format = AL_FORMAT_STEREO8;
        else if (resource->GetBitsPerSample() == 16)
            format = AL_FORMAT_STEREO16;
        else
            throw Exception("unknown number of bits per sample");
    }
    else
        throw Exception("unknown sound format");

    //attach the buffer
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, format, resource->GetBuffer(),
		 resource->GetBufferSize(), resource->GetFrequency());
    alSourcei(source, AL_BUFFER, bufferID);
    
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to bind source and buffer together but got: "
		      + Convert::ToString(error));
    }
    sourceID = source;
    
    alSourcei(sourceID, AL_ROLLOFF_FACTOR, 1.0f);
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to set rolloff factor but got: "
                        + Convert::ToString(error));
    }

    alSourcei(sourceID, AL_REFERENCE_DISTANCE, 50.0f);
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to set rolloff factor but got: "
                        + Convert::ToString(error));
    }

    alSourcei(sourceID, AL_MAX_DISTANCE, 150.0f);
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to set rolloff factor but got: "
                        + Convert::ToString(error));
    }
    

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
	if (!soundsystem->initialized) return false;

    ALint state = 0;
    ALCenum error;
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to get source state but got error: "
                        + Convert::ToString(error));
    }
    return (state == AL_PLAYING);
}

unsigned int OpenALSoundSystem::OpenALMonoSound::GetLengthInSamples() {
    return resource->GetNumberOfSamples();
}


Time OpenALSoundSystem::OpenALMonoSound::GetLength() {
    unsigned int numberOfSamples = GetLengthInSamples();
    unsigned int freq = resource->GetFrequency();

    unsigned int sampleCount = 0;
    unsigned int sec = 0;
    while (numberOfSamples--) {
        sampleCount++;
        if (sampleCount == freq) {
            sec++;
            sampleCount=0;
        }
    }

    unsigned int factor = 1000000; //to get microseconds
    unsigned int gcd = GCD(factor,freq);
    freq /= gcd;
    factor /= gcd;
    return Time(sec, (sampleCount*factor)/freq); //@todo save this calc!
}

void OpenALSoundSystem::OpenALMonoSound::SetRelativePosition(bool rel) {
	if (!soundsystem->initialized)
		return;

    ALCenum error;
    alSourcei(sourceID, AL_SOURCE_RELATIVE, rel);
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to set source relative but got: "
                        + Convert::ToString(error));
    }

}

void OpenALSoundSystem::OpenALMonoSound::SetPosition(Vector<3,float> pos) {
	if (!soundsystem->initialized)
		return;

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
		return Time(0,0);

    unsigned int numberOfSamples = GetElapsedSamples();
    unsigned int freq = resource->GetFrequency();

    unsigned int sampleCount = 0;
    unsigned int sec = 0;
    while (numberOfSamples--) {
        sampleCount++;
        if (sampleCount == freq) {
            sec++;
            sampleCount=0;
        }
    }

    unsigned int factor = 1000000; //to get microseconds
    unsigned int gcd = GCD(factor,freq);
    freq /= gcd;
    factor /= gcd;
    return Time(sec, (sampleCount*factor)/freq); //@todo save this calc!
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
//     if (left->GetElapsedTime() != right->GetElapsedTime())
//         throw Exception("left and right channels is out of sync");
    return left->GetElapsedTime();
}
  
void OpenALSoundSystem::OpenALStereoSound::Initialize() {
// 	if (!soundsystem->initialized)
// 		return;

	SoundFormat format = ress->GetFormat();
	
	if (format == MONO) //@todo: maybe this should duplicate the buffer to both left and right channel
		throw Exception("tried to make a stereo source with a mono sound pointer");

	char* leftbuffer = new char[ress->GetBufferSize()/2]; 
	char* rightbuffer = new char[ress->GetBufferSize()/2]; 

	char* data = ress->GetBuffer();    	  	

	if (ress->GetBitsPerSample() == 8) {
		
		for (unsigned int i=0; i < (ress->GetBufferSize())/2; i++) 
		{ 
			leftbuffer[i] = data[i*2]; // left chan
			rightbuffer[i] = data[i*2+1]; // right chan 
		} 

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), MONO, 8)),soundsystem);
		left->Initialize();
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), MONO, 8)),soundsystem);
		right->Initialize();
	}
	else if (ress->GetBitsPerSample() == 16) {
		
		int j = 0;
		for (unsigned int i=0; i < (ress->GetBufferSize()); i += 4)  
		{ 			
			leftbuffer[j] = data[i]; // left chan
			leftbuffer[j+1] = data[i+1]; // left chan
			rightbuffer[j] = data[i+2]; // right chan 
			rightbuffer[j+1] = data[i+3]; // right chan 
			j += 2;
		} 

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), MONO, 16)), soundsystem);
		left->Initialize();
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, ress->GetBufferSize()/2, ress->GetFrequency(), MONO, 16)),soundsystem);
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
    if (left->IsPlaying() != right->IsPlaying())
        throw Exception("left and right channel state is out of sync");
    return left->IsPlaying();
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

unsigned int OpenALSoundSystem::CustomSoundResource::GetBitsPerSample() {
	return bitsPerSample;
}

unsigned int OpenALSoundSystem::CustomSoundResource::GetFrequency() {
	return frequency;
}

SoundFormat OpenALSoundSystem::CustomSoundResource::GetFormat() {
	return format;
}

OpenALSoundSystem::CustomSoundResource::CustomSoundResource(char* data, unsigned int size, int freq, SoundFormat format, unsigned int bitsPerSample) {
	this->data = data;
	this->size = size;
	this->frequency = freq;
	this->format = format;
    this->bitsPerSample = bitsPerSample;
}

OpenALSoundSystem::CustomSoundResource::~CustomSoundResource() {

}

void OpenALSoundSystem::CustomSoundResource::Load() {

}

void OpenALSoundSystem::CustomSoundResource::Unload() {

}

} // NS Sound
} // NS OpenEngine

