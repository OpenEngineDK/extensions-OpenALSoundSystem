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
#include <Sound/TimedExecutioner.h>
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
#include <Resources/IStreamingSoundResource.h>

#include <list>
#include <set>
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
using OpenEngine::Resources::IStreamingSoundResourcePtr;

using std::list;
using std::set;
using std::vector;
using std::string;
using std::queue;

class ALMonoEventArg;
class ALStereoEventArg;
class ALStreamEventArg;

class OpenALSoundSystem : public ISoundSystem
                        , private IListener<ALMonoEventArg>                          
                        , private IListener<ALStereoEventArg>
                        , private IListener<ALStreamEventArg> {
    friend class ALMonoEventArg;
    friend class ALStereoEventArg;
    friend class ALStreamEventArg;
private:

    // ISceneNode* root;
    IViewingVolume* vv;
    Vector<3,float> prevPos;	

    ALCdevice*  alcDevice;
    ALCcontext* alcContext;
    vector<string> devices;
    unsigned int device;
    
    SoundNodeVisitor visitor;
    TimedExecutioner<float> timedExecutioner;
    Time fadeTime;

    inline void MakeDeviceList();

    class OpenALMonoSound: public IMonoSound {
    public:

    private:
        ALuint sourceID;
        ALuint bufferID;

        ISoundResourcePtr resource;
        OpenALSoundSystem* soundsystem;

        // state
        float maxdist;
        float gain;
        Vector<3,float> pos;
        bool rel;
        
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

    class OpenALStreamingSound : public ISound {
    private:
        ALuint sourceID;
        vector<ALuint> bufferIDs;
        Time length;
        IStreamingSoundResourcePtr resource;
        OpenALSoundSystem *soundsystem;
        Event<ALStreamEventArg> e;

        float maxdist;
        float gain;
        Vector<3,float> pos;
        bool rel;

        int last_offset;

        friend class OpenALSoundSystem;
    public:
        OpenALStreamingSound(IStreamingSoundResourcePtr resource, 
                             OpenALSoundSystem* soundsystem);
        ~OpenALStreamingSound();
        
        bool IsStereoSound();
        bool IsMonoSound();

        void Play();
        void Stop();
        void Pause();

        bool IsPlaying();

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

        Time CalculateLength();

    };

	class CustomSoundResource : public ISoundResource {
		private:
			char* data;
			unsigned int size, frequency, bitsPerSample;
			SoundFormat format;

		public:
			char* GetBuffer();
            char* GetBuffer(unsigned int offset, unsigned int size);
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
    list<OpenALStreamingSound*> streams;

    set<OpenALStreamingSound*> playingStreams;

    map<ISoundResourcePtr, ALuint> buffers;
    map<IStreamingSoundResourcePtr, vector<ALuint> > bufferList;
    queue<ALMonoEventArg> monoActions;
    queue<ALStereoEventArg> stereoActions;
    queue<ALStreamEventArg> streamActions;

    inline void InitResource(ISoundResourcePtr resource);
    inline void InitResource(IStreamingSoundResourcePtr resource);
    inline void InitSound(OpenALStreamingSound* sound);
    inline void InitSound(OpenALMonoSound* sound);
    void UpdatePosition(OpenALMonoSound* sound);
    void UpdatePosition(OpenALStreamingSound* sound);

public:
    OpenALSoundSystem(/*ISceneNode* root, IViewingVolume* vv*/);
    ~OpenALSoundSystem();

    ISound* CreateSound(ISoundResourcePtr resource);
    ISound* CreateSound(IStreamingSoundResourcePtr resource);
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
    void Handle(ALStreamEventArg e);
    inline void ApplyAction(ALMonoEventArg e);
    inline void ApplyAction(ALStereoEventArg e);
    inline void ApplyAction(ALStreamEventArg e);
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

class ALStreamEventArg {
public:
    ISound::Action action;
    OpenALSoundSystem::OpenALStreamingSound* sound;
    ALStreamEventArg(ISound::Action action, OpenALSoundSystem::OpenALStreamingSound* sound)
        : action(action), sound(sound) {};
    virtual ~ALStreamEventArg() {};
};

} // NS Sound
} // NS OpenEngine

#endif
