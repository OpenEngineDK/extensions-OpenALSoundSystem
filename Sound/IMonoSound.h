#ifndef _SOUND_IMONOSOUND_H_
#define _SOUND_IMONOSOUND_H_

#include <Sound/ISound.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Resources/ISoundResource.h>

namespace OpenEngine {
namespace Sound {

class IMonoSound : public ISound {
 	        
public:
	virtual ~IMonoSound() {}	

    virtual bool IsStereoSound() { return false;};
    virtual bool IsMonoSound() { return true;};

    virtual ISoundResourcePtr GetResource() = 0;

    virtual void SetPosition(Vector<3,float> pos) = 0;
    virtual Vector<3,float> GetPosition() = 0;

    virtual void SetVelocity(Vector<3,float> vel) = 0;
    virtual Vector<3,float> GetVelocity() = 0;
    

    virtual void SetRelativePosition(bool rel) = 0;

// 	virtual ISound::PlaybackState GetPlaybackState() = 0;
    
    virtual void SetMaxDistance(float dist) = 0;
    virtual float GetMaxDistance() = 0;

//     virtual void SetMinGain(float gain) = 0;
//     virtual float GetMinGain() = 0;

//     virtual void SetMaxGain(float gain) = 0;
//     virtual float GetMaxGain() = 0;

//     virtual void SetReferenceDistance(float dist) = 0;
//     virtual float GetReferenceDistance() = 0;

//     virtual void SetRolloffFactor(float rolloff) = 0;
//     virtual float GetRolloffFactor() = 0;

//     virtual void SetPitch(float pitch) = 0;
//     virtual float GetPitch() = 0;


    // spatial/geometrical attributes
//     virtual void SetDirection(Vector<3,float> dir) = 0;
//     virtual Vector<3,float> GetDirection() = 0;

//     virtual void SetConeInnerAngle(float angle) = 0;
//     virtual float GetConeInnerAngle() = 0;

//     virtual void SetConeOuterAngle(float angle) = 0;
//     virtual float GetConeOuterAngle() = 0;

//     virtual Quaternion<float> GetRotation() = 0;
//     virtual void SetRotation(Quaternion<float> rotation) = 0;
     
};

} // NS Sound
} // NS OpenEngine

#endif
