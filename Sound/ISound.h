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
    virtual void Initialize() = 0;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void SetPosition(Vector<3,float> pos) = 0;
    virtual Vector<3,float> GetPosition() = 0;
    virtual Quaternion<float> GetRotation() = 0;
    virtual void SetRotation(Quaternion<float> rotation) = 0;

    virtual unsigned int GetID() = 0;
    virtual void SetID(unsigned int) = 0;

    virtual ISoundResourcePtr GetResource() = 0;

    virtual ~ISound() {}
};

} // NS Sound
} // NS OpenEngine

#endif // _ISOUND_H_
