// OpenAL sound manager implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENAL_SOUND_SYSTEM_H_
#define _OPENAL_SOUND_SYSTEM_H_

#include <Sound/ISoundSystem.h>
#include <Display/IViewingVolume.h>
#include <Scene/ISceneNode.h>
#include <Core/Event.h>
#include <Logging/Logger.h>

#include <Meta/OpenAL.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

#include <Core/IModule.h>
#include <Core/QueuedEvent.h>
#include <Core/IListener.h>
#include <Scene/SoundNode.h>
#include <Sound/IMonoSound.h>
#include <Sound/IStereoSound.h>
#include <Sound/SoundNodeVisitor.h>
#include <Resources/ISoundResource.h>

#include <list>
#include <vector>
#include <string>
#include <queue>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Core::IModule;
using OpenEngine::Core::QueuedEvent;
using OpenEngine::Core::IListener;
using OpenEngine::Display::IViewingVolume;
using OpenEngine::Math::Vector;
using OpenEngine::Math::Quaternion;
using OpenEngine::Scene::ISceneNode;
using OpenEngine::Scene::SoundNode;
using OpenEngine::Resources::ISoundResourcePtr;

using std::list;
using std::vector;
using std::string;
using std::queue;

class ALMonoEventArg;
class ALStereoEventArg;

class OpenALSoundSystem : public ISoundSystem, private IListener<ALMonoEventArg>, 
                          private IListener<ALStereoEventArg> {
    friend class ALMonoEventArg;
    friend class ALStereoEventArg;
private:

    // ISceneNode* root;
    IViewingVolume* vv;
    Vector<3,float> prevPos;	

    ALCdevice*  alcDevice;
    ALCcontext* alcContext;
    vector<string> devices;
    unsigned int device;
    
    SoundNodeVisitor visitor;

    inline void MakeDeviceList();
    class OpenALMonoSound: public IMonoSound {
    public:

    private:
        ALuint sourceID;
        ALuint bufferID;

        // state
        float gain;
        float maxdist;
        bool rel;
        Vector<3,float> pos;
        
        ISoundResourcePtr resource;
        OpenALSoundSystem* soundsystem;
        Time length;
        Time CalculateLength();
        Event<ALMonoEventArg> e;
        friend class OpenALSoundSystem;
    public:
        OpenALMonoSound(ISoundResourcePtr resource, OpenALSoundSystem* soundsystem);
        virtual ~OpenALMonoSound();

        void SetMaxDistance(float dist);
        float GetMaxDistance();
        ALuint GetID();
        bool IsPlaying();
        void Play();
        void Stop();
        void Pause();
        void SetLooping(bool loop);
        bool GetLooping();
        void SetGain(float gain);
        float GetGain();
        unsigned int GetLengthInSamples();
        Time GetLength();
        void SetElapsedSamples(unsigned int samples);
        unsigned int GetElapsedSamples();
        void SetElapsedTime(Time time);
        Time GetElapsedTime();
        void SetVelocity(Vector<3,float> vel);
        Vector<3,float> GetVelocity();
        Vector<3,float> GetPosition();
        void SetPosition(Vector<3,float> pos);
        void SetRelativePosition(bool rel);
        ISoundResourcePtr GetResource();
    };

	class OpenALStereoSound : public IStereoSound {
    private:
        OpenALMonoSound* left;
        OpenALMonoSound* right;
        OpenALSoundSystem* soundsystem;
        ISoundResourcePtr res;
        Event<ALStereoEventArg> e;
        friend class OpenALSoundSystem;
     public:
        OpenALStereoSound(ISoundResourcePtr resource, OpenALSoundSystem* soundsystem);
        ~OpenALStereoSound();
        void Play();
        void Stop();
        void Pause();

        bool IsPlaying();

        IMonoSound* GetLeft();
        IMonoSound* GetRight();

        void SetLooping(bool loop);
        bool GetLooping();
  
        void SetGain(float gain);
        float GetGain();

        unsigned int GetLengthInSamples();
        Time GetLength();

        void SetElapsedSamples(unsigned int samples);
        unsigned int GetElapsedSamples();

        void SetElapsedTime(Time time);
        Time GetElapsedTime();
    };

	class CustomSoundResource : public ISoundResource {
		private:
			char* data;
			unsigned int size, frequency, bitsPerSample;
			SoundFormat format;

		public:
			char* GetBuffer();
			unsigned int GetBufferSize();
			unsigned int GetFrequency();
            unsigned int GetBitsPerSample();
			SoundFormat GetFormat();
			void Load();
			void Unload();

			CustomSoundResource(char* newdata, unsigned int newsize, int newfreq, SoundFormat newformat, unsigned int bitsPerSample);
			~CustomSoundResource();

	};
    list<OpenALMonoSound*> monos;
    list<OpenALStereoSound*> stereos;
    map<ISoundResourcePtr, ALuint> buffers;
    queue<ALMonoEventArg> monoActions;
    queue<ALStereoEventArg> stereoActions;

    inline void InitResource(ISoundResourcePtr resource);
    inline void InitSound(OpenALMonoSound* sound);
    void UpdatePosition(OpenALMonoSound* sound);

public:
    OpenALSoundSystem(/*ISceneNode* root, IViewingVolume* vv*/);
    ~OpenALSoundSystem();

    ISound* CreateSound(ISoundResourcePtr resource);
    // void SetRoot(ISceneNode* node);
	void SetMasterGain(float gain);
	float GetMasterGain();

    unsigned int GetDeviceCount();
    string GetDeviceName(unsigned int device);
    void SetDevice(unsigned int device);

    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);
    void Handle(RenderingEventArg arg);

    void Handle(ALMonoEventArg e);
    void Handle(ALStereoEventArg e);
    inline void ApplyAction(ALMonoEventArg e);
    inline void ApplyAction(ALStereoEventArg e);
};

class ALMonoEventArg {
public:
    ISound::Action action;
    OpenALSoundSystem::OpenALMonoSound* sound;
    ALMonoEventArg(ISound::Action action, OpenALSoundSystem::OpenALMonoSound* sound): action(action), sound(sound) {};
    virtual ~ALMonoEventArg() {};
};

class ALStereoEventArg {
public:
    ISound::Action action;
    OpenALSoundSystem::OpenALStereoSound* sound;
    ALStereoEventArg(ISound::Action action, OpenALSoundSystem::OpenALStereoSound* sound): action(action), sound(sound) {};
    virtual ~ALStereoEventArg() {};
};

} // NS Sound
} // NS OpenEngine

#endif
