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
#include <Sound/ISound.h>
#include <Utils/Convert.h>
#include <Math/Math.h>
#include <Display/Viewport.h>
#include <Display/IViewingVolume.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Core::Exception;
using OpenEngine::Utils::Convert;
using namespace OpenEngine::Core;
using namespace OpenEngine::Math;  
using namespace OpenEngine::Display;

void OpenALSoundSystem::MakeDeviceList() {
    devices.clear();
    const ALCchar* device = alcGetString( NULL, ALC_DEVICE_SPECIFIER );
    unsigned int i = 0;
    while (true) {
        string s = &device[i];
        if (!s.empty()) 
            devices.push_back(s);
        i += s.size() + 1;
        if (device[i] == 0) break;
    }
    // for (vector<string>::iterator it = devices.begin();
    //      it != devices.end();
    //      ++it) { 
    //     logger.info << *it << logger.end;
    // }
}

OpenALSoundSystem::OpenALSoundSystem() 
    : alcDevice(NULL)
    , alcContext(NULL)
    , device(0)
{
    MakeDeviceList();
}
    
OpenALSoundSystem::~OpenALSoundSystem() {

}

void OpenALSoundSystem::UpdatePosition(OpenALMonoSound* sound) {
    if (!alcContext) return;
    float pos[3];
    sound->pos.ToArray(pos);
    alSourcefv(sound->sourceID, AL_POSITION, pos);
    ALCenum error;
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("Error updating sound position: "
		      + Convert::ToString(error));
    }
}

unsigned int OpenALSoundSystem::GetDeviceCount() {
    return devices.size();
}

string OpenALSoundSystem::GetDeviceName(unsigned int device) {
    return devices[device];
}

void OpenALSoundSystem::SetDevice(unsigned int device) {
    this->device = device;
}

ISound* OpenALSoundSystem::CreateSound(ISoundResourcePtr resource) {
    SoundFormat format = resource->GetFormat();
    ISound* sound = NULL;
    if (format == MONO) {
        OpenALMonoSound* msound = new OpenALMonoSound(resource, this);
        sound = msound;
        buffers[resource] = 0;
        msound->e.Attach(*this);
        if (alcContext) {
            InitResource(resource);
            InitSound(msound);
        }
        else {
            monos.push_back(msound);
        }
    } else if (format == STEREO) {
        OpenALStereoSound* ssound = new OpenALStereoSound(resource, this);
        sound = ssound;
        buffers[ssound->left->resource] = 0;
        buffers[ssound->right->resource] = 0;
        ssound->e.Attach(*this);
        if (alcContext) {
            InitResource(ssound->left->resource);
            InitResource(ssound->right->resource);
            InitSound(ssound->left);
            InitSound(ssound->right);
        }
        else {	
            monos.push_back(ssound->left);
            monos.push_back(ssound->right);
        }
    }
    else throw Exception("unsupported sound format");
    return sound;
}

void OpenALSoundSystem::SetMasterGain(float gain) {
	if (!alcContext)
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
	if (!alcContext)
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

void OpenALSoundSystem::Handle(ALMonoEventArg e) {
    if (!alcContext) {
        monoActions.push(e);
        return;
    } 
    ApplyAction(e);
}

void OpenALSoundSystem::ApplyAction(ALMonoEventArg e) {
    ALCenum error;
    string errstr;
    ALuint sourceID = e.sound->sourceID;
    switch (e.action) {
    case ISound::PLAY: 
        alSourcePlay(sourceID);
        break;
    case ISound::STOP: 
        alSourceStop(sourceID);
        break;
    case ISound::PAUSE:
        alSourcePause(sourceID);
        break;
    case ISound::LOOP:
        alSourcei(sourceID, AL_LOOPING, (ALboolean)true);
        break;
    case ISound::NO_LOOP:
        alSourcei(sourceID, AL_LOOPING, (ALboolean)false);
        break;
    }
    if ((error = alGetError()) != AL_NO_ERROR)
        throw Exception("Error applying sound action: " + Convert::ToString(error));
}

void OpenALSoundSystem::Handle(ALStereoEventArg e) {
    if (!alcContext) {
        stereoActions.push(e);
        return;
    } 
    ApplyAction(e);
}

void OpenALSoundSystem::ApplyAction(ALStereoEventArg e) {
    ALCenum error;
    string errstr;
    ALuint list[2];
    list[0] = e.sound->left->GetID();
    list[1] = e.sound->right->GetID();
    switch (e.action) {
    case ISound::PLAY:
        // logger.info << "play sound" << logger.end;
        alSourcePlayv(2, list);
        break;
    case ISound::STOP: 
        alSourceStopv(2, &list[0]);
        break;
    case ISound::PAUSE:
        alSourcePausev(2, &list[0]);
        break;
    default:
        break;
    }
    if ((error = alGetError()) != AL_NO_ERROR)
        throw Exception("Error applying sound action: " + Convert::ToString(error));
}

void OpenALSoundSystem::InitResource(ISoundResourcePtr resource) {
    if (buffers[resource] != 0) {
        return;
    }
    ALuint buffer;
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
            throw Exception("Unknown number of bits per sample.");
    }
    else
        throw Exception("Unknown sound format.");
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, resource->GetBuffer(),
                 resource->GetBufferSize(), resource->GetFrequency());
    buffers[resource] = buffer;
    // logger.info << "buffer: " << buffer << logger.end;
}

void OpenALSoundSystem::InitSound(OpenALMonoSound* sound) {
        //generate the source
        ALuint source;
        alGenSources(1, &source);

        ALCenum error;
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("Error generating source: "
                            + Convert::ToString(error));
        }
        
        //attach the buffer
        ALuint buffer = buffers[sound->resource];
        // logger.info << "bufferbind: " << buffer << logger.end;

        alSourcei(source, AL_BUFFER, buffer);
    
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("Error binding buffer: "
                            + Convert::ToString(error));
        }
        sound->sourceID = source;
        sound->bufferID = buffer;
        sound->length = sound->CalculateLength();
        
        // set sound attributes (ugly stuff)...
        alSourcei(source, AL_ROLLOFF_FACTOR, 1.0f);
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("tried to set rolloff factor but got: "
                            + Convert::ToString(error));
        }
        
        alSourcei(source, AL_REFERENCE_DISTANCE, 50.0f);
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("tried to set rolloff factor but got: "
                            + Convert::ToString(error));
        }
        
        alSourcei(source, AL_MAX_DISTANCE, sound->maxdist);
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("tried to set rolloff factor but got: "
                            + Convert::ToString(error));
        }
        alSourcef(source, AL_GAIN, sound->gain);
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("tried to set gain but got: "
                            + Convert::ToString(error));
        }

        alSourcei(source, AL_SOURCE_RELATIVE, sound->rel);
        if ((error = alGetError()) != AL_NO_ERROR) {
            throw Exception("tried to set source relative but got: "
                            + Convert::ToString(error));
        }
        
        UpdatePosition(sound);
}

void OpenALSoundSystem::Handle(InitializeEventArg arg) {
    alcDevice = alcOpenDevice(devices[device].c_str());
    if (!alcDevice) {
        logger.error << "OpenAL not initialized." << logger.end;
        return;
    }
    alcContext = alcCreateContext(alcDevice, NULL);
    alcMakeContextCurrent(alcContext); 
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alDistanceModel(AL_LINEAR_DISTANCE);
    logger.info << "OpenAL has been initialized using device: " << devices[device] << logger.end;

    // init buffers
    map<ISoundResourcePtr, ALuint>::iterator j = buffers.begin();
    for (; j != buffers.end(); ++j) {
        ISoundResourcePtr resource = (*j).first;
        InitResource(resource);
    }
    
    // init sounds
    list<OpenALMonoSound*>::iterator i = monos.begin();
    for (; i != monos.end(); ++i) {
        OpenALMonoSound* sound = (*i);
        InitSound(sound);
    }
    
    // process queued events
    while (!monoActions.empty()) {
        ApplyAction(monoActions.front());
        monoActions.pop();
    }
    while (!stereoActions.empty()) {
        ApplyAction(stereoActions.front());
        stereoActions.pop();
    }
}

void OpenALSoundSystem::Handle(RenderingEventArg arg) {
	if (!alcContext)
		return;
    
    const float deltaTime = arg.approx;

    IViewingVolume* vv = arg.renderer.GetViewport().GetViewingVolume();
	
    Vector<3,float> vvpos = vv->GetPosition();
    alListener3f(AL_POSITION, vvpos[0], vvpos[1], vvpos[2]);
    
    // Give camera orientation to openal
    Quaternion<float> rot = vv->GetDirection();
    Vector<3,float> up = rot.RotateVector(Vector<3,float>(0,1,0));
    Vector<3,float> dir = rot.RotateVector(Vector<3,float>(0,0,-1));
    float orientation[6];
    dir.ToArray(orientation);
    up.ToArray(&orientation[3]);
    alListenerfv(AL_ORIENTATION, orientation);
    
    visitor.SetDeltaTime(deltaTime);
    arg.renderer.GetSceneRoot()->Accept(visitor);
}

void OpenALSoundSystem::Handle(ProcessEventArg arg) {
}

void OpenALSoundSystem::Handle(DeinitializeEventArg arg) {
    alcMakeContextCurrent(NULL);
    if (alcContext != NULL) {
        alcDestroyContext(alcContext);
        alcContext = NULL;
    }
    if (alcDevice != NULL) {
        alcCloseDevice (alcDevice);
        alcDevice = NULL;
    }
}

OpenALSoundSystem::OpenALMonoSound::OpenALMonoSound(ISoundResourcePtr resource, 
                                                    OpenALSoundSystem* soundsystem) 
    : resource(resource)
    , soundsystem(soundsystem)
    , maxdist(1000.0)
    , gain(10.0)
    , pos(Vector<3,float>(0.0,0.0,0.0))
    , rel(false)
{}
    

OpenALSoundSystem::OpenALMonoSound::~OpenALMonoSound() {
}

void OpenALSoundSystem::OpenALMonoSound::Play() {
    e.Notify(ALMonoEventArg(PLAY, this));
}

void OpenALSoundSystem::OpenALMonoSound::Stop() {
    e.Notify(ALMonoEventArg(STOP, this));
}

void OpenALSoundSystem::OpenALMonoSound::Pause() {
    e.Notify(ALMonoEventArg(PAUSE, this));
}

bool OpenALSoundSystem::OpenALMonoSound::IsPlaying() {
	if (!soundsystem->alcContext) return false;

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

Time OpenALSoundSystem::OpenALMonoSound::CalculateLength() {
    //@todo optimize these calculations
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
    return Time(sec, (sampleCount*factor)/freq);
}

Time OpenALSoundSystem::OpenALMonoSound::GetLength() {
    return length;
}

void OpenALSoundSystem::OpenALMonoSound::SetRelativePosition(bool rel) {
    this->rel = rel;
	if (!soundsystem->alcContext)
		return;

    ALCenum error;
    alSourcei(sourceID, AL_SOURCE_RELATIVE, rel);
    if ((error = alGetError()) != AL_NO_ERROR) {
        throw Exception("tried to set source relative but got: "
                        + Convert::ToString(error));
    }

}

void OpenALSoundSystem::OpenALMonoSound::SetPosition(Vector<3,float> pos) {
    this->pos = pos;
    soundsystem->UpdatePosition(this);
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
    this->gain = gain;
	if (!soundsystem->alcContext)
		return;

    ALCenum error;
    alSourcef(sourceID, AL_GAIN, (ALfloat)gain);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set gain but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetGain() {
    return gain;
}

void OpenALSoundSystem::OpenALMonoSound::SetLooping(bool loop) {
    if (loop) 
        e.Notify(ALMonoEventArg(LOOP, this));
    else
        e.Notify(ALMonoEventArg(NO_LOOP, this));
}

bool OpenALSoundSystem::OpenALMonoSound::GetLooping() {
	if (!soundsystem->alcContext)
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
	if (!soundsystem->alcContext)
		return;

    ALCenum error;
    alSourcei(sourceID, AL_SAMPLE_OFFSET, (ALint)samples);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set offset by sample but got: "
		      + Convert::ToString(error));
    }
}

unsigned int OpenALSoundSystem::OpenALMonoSound::GetElapsedSamples() {
	if (!soundsystem->alcContext)
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
	if (!soundsystem->alcContext)
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
	if (!soundsystem->alcContext)
		return Time(0,0);

    // unsigned int numberOfSamples = GetElapsedSamples();
    // unsigned int freq = resource->GetFrequency();

    // unsigned int sampleCount = 0;
    // unsigned int sec = 0;
    // while (numberOfSamples--) {
    //     sampleCount++;
    //     if (sampleCount == freq) {
    //         sec++;
    //         sampleCount=0;
    //     }
    // }

    // unsigned int factor = 1000000; //to get microseconds
    // unsigned int gcd = GCD(factor,freq);
    // freq /= gcd;
    // factor /= gcd;
    // return Time(sec, (sampleCount*factor)/freq); //@todo save this calc!
    float t; 
    alGetSourcef(sourceID, AL_SEC_OFFSET, &t);
    // logger.info << "t: " << t << logger.end;
    uint64_t s = t;
    // logger.info << "s: " << s << logger.end;
    t = t - s;
    uint32_t us = t * 1000000;
    // logger.info << "us: " << us << logger.end;
    return Time(s,us);
}

void OpenALSoundSystem::OpenALMonoSound::SetMaxDistance(float distance) {
    maxdist = distance;
	if (!soundsystem->alcContext)
        return;
    ALCenum error;
    alSourcef(sourceID, AL_MAX_DISTANCE, (ALfloat)distance);
    if ((error = alGetError()) != AL_NO_ERROR) {
      throw Exception("tried to set max distance but got: "
		      + Convert::ToString(error));
    }
}

float OpenALSoundSystem::OpenALMonoSound::GetMaxDistance() {
    return maxdist;
}


void OpenALSoundSystem::OpenALMonoSound::SetVelocity(Vector<3,float> vel) {
	if (!soundsystem->alcContext)
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
	if (!soundsystem->alcContext)
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

    OpenALSoundSystem::OpenALStereoSound::OpenALStereoSound(ISoundResourcePtr resource, OpenALSoundSystem* soundsystem): soundsystem(soundsystem), res(resource) {
	SoundFormat format = res->GetFormat();
	
	if (format == MONO) //@todo: maybe this should duplicate the buffer to both left and right channel
		throw Exception("tried to make a stereo source with a mono sound pointer");

	char* leftbuffer = new char[res->GetBufferSize()/2]; 
	char* rightbuffer = new char[res->GetBufferSize()/2]; 

	char* data = res->GetBuffer();    	  	

	if (res->GetBitsPerSample() == 8) {
		
		for (unsigned int i=0; i < (res->GetBufferSize())/2; i++) 
		{ 
			leftbuffer[i] = data[i*2]; // left chan
			rightbuffer[i] = data[i*2+1]; // right chan 
		} 

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, res->GetBufferSize()/2, res->GetFrequency(), MONO, 8)), soundsystem);
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, res->GetBufferSize()/2, res->GetFrequency(), MONO, 8)), soundsystem);
	}
	else if (res->GetBitsPerSample() == 16) {
		
		int j = 0;
		for (unsigned int i=0; i < (res->GetBufferSize()); i += 4)  
		{ 			
			leftbuffer[j] = data[i]; // left chan
			leftbuffer[j+1] = data[i+1]; // left chan
			rightbuffer[j] = data[i+2]; // right chan 
			rightbuffer[j+1] = data[i+3]; // right chan 
			j += 2;
		} 

		left = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(leftbuffer, res->GetBufferSize()/2, res->GetFrequency(), MONO, 16)), soundsystem);
		right = new OpenALMonoSound(ISoundResourcePtr(new CustomSoundResource(rightbuffer, res->GetBufferSize()/2, res->GetFrequency(), MONO, 16)),soundsystem);
	}
}

OpenALSoundSystem::OpenALStereoSound::~OpenALStereoSound() {
	delete left;
	delete right;
}

void OpenALSoundSystem::OpenALStereoSound::SetGain(float gain) {
    if (!soundsystem->alcContext)
 		return;

    left->SetGain(gain);
    right->SetGain(gain);
}

float OpenALSoundSystem::OpenALStereoSound::GetGain() {
	if (!soundsystem->alcContext)
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
  
void OpenALSoundSystem::OpenALStereoSound::Play() {
    e.Notify(ALStereoEventArg(PLAY, this));
}

void OpenALSoundSystem::OpenALStereoSound::Stop() {
    e.Notify(ALStereoEventArg(STOP, this));
}

void OpenALSoundSystem::OpenALStereoSound::Pause() {
    e.Notify(ALStereoEventArg(PAUSE, this));
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

