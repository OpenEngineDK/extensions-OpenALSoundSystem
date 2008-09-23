#ifndef _SOUND_ISTEREOSOUND_H_
#define _SOUND_ISTEREOSOUND_H_

#include <Sound/ISound.h>

namespace OpenEngine {
namespace Sound {

class IStereoSound : public ISound {
public:
    virtual ~IStereoSound() {};
    
    virtual IMonoSound* GetLeft() = 0;
    virtual IMonoSound* GetRight() = 0;

    virtual bool IsStereoSound() { return true;};
    virtual bool IsMonoSound() { return false;};

};

} // NS Sound
} // NS OpenEngine

#endif
