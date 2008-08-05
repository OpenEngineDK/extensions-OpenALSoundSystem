// Interface for a sound source.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _ISOUND_H_
#define _ISOUND_H_

#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Resources/ISoundResource.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Math::Vector;
using OpenEngine::Math::Quaternion;
using namespace OpenEngine::Resources;



/**
 * ISound.
 * Interface for a sound source in OpenEngine.
 *
 * @class ISound ISound.h Sound/ISound.h
 */
class ISound {
private:

public:
    enum PlaybackState {
        INITIAL,
        PLAYING,
        PAUSED,
        STOPPED
    };

    virtual ~ISound() {}

    virtual void Initialize() = 0;
 
    // playback execution methods
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;

    virtual void SetLooping(bool loop) = 0;
    virtual bool GetLooping() = 0;
    
    virtual void SetSampleOffset(int samples) = 0;
    virtual int GetSampleOffset() = 0;

    virtual void SetTimeOffset(float seconds) = 0;
    virtual float GetTimeOffset() = 0;


    virtual PlaybackState GetPlaybackState() = 0;

    virtual unsigned int GetID() = 0;
    virtual void SetID(unsigned int) = 0;

    virtual ISoundResourcePtr GetResource() = 0;

    virtual void SetMaxDistance(float dist) = 0;
    virtual float GetMaxDistance() = 0;

    virtual void SetMinGain(float gain) = 0;
    virtual float GetMinGain() = 0;

    virtual void SetMaxGain(float gain) = 0;
    virtual float GetMaxGain() = 0;

    virtual void SetPitch(float pitch) = 0;
    virtual float GetPitch() = 0;

	virtual void SetGain(float gain) = 0;
    virtual float GetGain() = 0;

    virtual void SetVelocity(Vector<3,float> vel) = 0;
    virtual Vector<3,float> GetVelocity() = 0;
    

    // spatial/geometrical attributes
    virtual void SetDirection(Vector<3,float> dir) = 0;
    virtual Vector<3,float> GetDirection() = 0;

    virtual void SetConeInnerAngle(float angle) = 0;
    virtual float GetConeInnerAngle() = 0;

    virtual void SetConeOuterAngle(float angle) = 0;
    virtual float GetConeOuterAngle() = 0;

    virtual void SetPosition(Vector<3,float> pos) = 0;
    virtual Vector<3,float> GetPosition() = 0;

    virtual Quaternion<float> GetRotation() = 0;
    virtual void SetRotation(Quaternion<float> rotation) = 0;

};

} // NS Sound
} // NS OpenEngine

#endif // _ISOUND_H_
