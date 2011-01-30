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

#include <Utils/Timer.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Math::Vector;
using OpenEngine::Math::Quaternion;
using namespace OpenEngine::Resources;
using OpenEngine::Utils::Time;

/**
 * ISound.
 * Interface for a sound source in OpenEngine.
 *
 * @class ISound ISound.h Sound/ISound.h
 */
class ISound {
private:

public:
    enum Action {
        PLAY, STOP, PAUSE, LOOP, NO_LOOP, FADE_UP, FADE_DOWN
    };

    virtual ~ISound() {}

    // query playback state 

    virtual bool IsPlaying() = 0;

    // playback execution methods
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;

	virtual void SetGain(float gain) = 0;
    virtual float GetGain() = 0;

	virtual void SetLooping(bool loop) = 0;
    virtual bool GetLooping() = 0;
    
    virtual void SetElapsedSamples(unsigned int samples) = 0;
    virtual unsigned int GetElapsedSamples() = 0;

    virtual void SetElapsedTime(Time time) = 0;
    virtual Time GetElapsedTime() = 0;

	virtual unsigned int GetLengthInSamples() = 0;
	virtual Time GetLength() = 0;

    // virtual Event<ActionEventArg>& ActionEvent() = 0;

    Time GetTimeLeft() {
        return GetLength() - GetElapsedTime();
    }

    bool IsAtEnd() {
        return (GetElapsedSamples() == GetLengthInSamples());
    }


    //
    virtual bool IsStereoSound() = 0;
    virtual bool IsMonoSound() = 0;

};


} // NS Sound
} // NS OpenEngine

#endif // _ISOUND_H_
