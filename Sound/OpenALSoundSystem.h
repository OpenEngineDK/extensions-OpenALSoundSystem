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
#include <Resources/ISoundResource.h>

#include <list>

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

class OpenALSoundSystem : public ISoundSystem {
private:
    ISceneNode* theroot;
    IViewingVolume* vv;
    Vector<3,float> prevPos;
	
    void Init();

    class OpenALMonoSound: public IMonoSound {
    private:
        ALuint sourceID;
        ALuint bufferID;
        Vector<3,float> pos;
        Quaternion<float> dir;
        ISoundResourcePtr resource;
        
        void PrintAttribute(ALenum e);
        string EnumToString(ALenum e);
    public:
        OpenALMonoSound(ISoundResourcePtr resource);
        ~OpenALMonoSound();
        void Initialize();
		void Play();
        void Stop();
        void Pause();

        void SetSampleOffset(int samples);
        int GetSampleOffset();
        
        void SetTimeOffset(float seconds);
        float GetTimeOffset();
        
        PlaybackState GetPlaybackState();
        
        void SetVelocity(Vector<3,float> vel);
        Vector<3,float> GetVelocity();

        Vector<3,float> GetPosition();
        void SetPosition(Vector<3,float> pos);
        Quaternion<float> GetRotation();
        void SetRotation(Quaternion<float> rotation);
        unsigned int GetID();
        void SetID(unsigned int);
        ISoundResourcePtr GetResource();
        
        void SetMaxDistance(float dist);
        float GetMaxDistance();
        
        void SetLooping(bool loop);
        bool GetLooping();
        
        void SetMinGain(float gain);
        float GetMinGain();
        
        void SetMaxGain(float gain);
        float GetMaxGain();
        
        void SetReferenceDistance(float dist);
        float GetReferenceDistance();

        void SetRolloffFactor(float rolloff);
        float GetRolloffFactor();

        void SetGain(float gain);
        float GetGain();

        void SetPitch(float pitch);
        float GetPitch();
        
        void SetDirection(Vector<3,float> dir);
        Vector<3,float> GetDirection();
        
        void SetConeInnerAngle(float angle);
        float GetConeInnerAngle();
        
        void SetConeOuterAngle(float angle);
        float GetConeOuterAngle();
        
    };

	class OpenALStereoSound : public IStereoSound {
		private:
			IMonoSound* left;
			IMonoSound* right;
			ISoundResourcePtr ress;

		public:
			OpenALStereoSound(ISoundResourcePtr resource);
			~OpenALStereoSound();
			void Initialize();
		    void Play();
			void Stop();
			void Pause();
			IMonoSound* GetLeft();
			IMonoSound* GetRight();

	};

	class CustomSoundResource : public ISoundResource {
		private:
			char* data;
			unsigned int size;
			int frequancy;
			SoundFormat format;

		public:
			char* GetBuffer();
			unsigned int GetBufferSize();
			int GetFrequency();
			SoundFormat GetFormat();
			void Load();
			void Unload();

			CustomSoundResource(char* newdata, unsigned int newsize, int newfreq, SoundFormat newformat);
			~CustomSoundResource();

	};

public:
    OpenALSoundSystem(ISceneNode* root, IViewingVolume* vv);
    ~OpenALSoundSystem();

    IMonoSound* CreateMonoSound(ISoundResourcePtr resource);
	IStereoSound* CreateStereoSound(ISoundResourcePtr resource);
    void SetRoot(ISceneNode* node);

    void Initialize();
    void Process(const float deltaTime, const float percent);
    void Deinitialize();
    bool IsTypeOf(const std::type_info& inf);
};

} // NS Sound
} // NS OpenEngine

#endif
