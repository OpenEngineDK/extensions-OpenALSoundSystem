
#ifndef _SOUND_ISTEREOSOUND_H_
#define _SOUND_ISTEREOSOUND_H_

#include <AL/al.h>
#include <AL/alc.h>
#include <Sound/ISound.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Logging/Logger.h>
#include <Resources/ISoundResource.h>

namespace OpenEngine {
namespace Sound {

class IStereoSound : public ISound {
        
    public:
		~IStereoSound() {};
        
	    virtual IMonoSound* GetLeft() = 0;
		virtual IMonoSound* GetRight() = 0;
     
};

} // NS Sound
} // NS OpenEngine

#endif