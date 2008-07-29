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
#include <Sound/ISound.h>
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
    
    
    void Init();

    class OpenALSound: public ISound {
    private:
        ALuint sourceID;
        ALuint bufferID;
        Vector<3,float> pos;
        Quaternion<float> dir;
        ISoundResourcePtr resource;
        
        void PrintAttribute(ALenum e);
        string EnumToString(ALenum e);
    public:
        OpenALSound(ISoundResourcePtr resource);
        ~OpenALSound();
        void Initialize();
        void Play();
        void Stop();
        void Pause();

        void SetSampleOffset(int samples);
        int GetSampleOffset();
        
        void SetTimeOffset(float seconds);
        float GetTimeOffset();
        
        PlaybackState GetPlaybackState();
        
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

public:
    OpenALSoundSystem(ISceneNode* root, IViewingVolume* vv);
    ~OpenALSoundSystem();

    ISound* CreateSound(ISoundResourcePtr resource);
    void SetRoot(ISceneNode* node);

    void Initialize();
    void Process(const float deltaTime, const float percent);
    void Deinitialize();
    bool IsTypeOf(const std::type_info& inf);
};

} // NS Sound
} // NS OpenEngine

#endif
