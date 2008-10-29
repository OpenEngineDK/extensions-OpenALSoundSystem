// SoundNode wrapping an object implementing ISound.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_SOUND_NODE_H_
#define _OE_SOUND_NODE_H_

#include <Scene/ISceneNode.h>

namespace OpenEngine {

//forward declarations
namespace Sound{ class IMonoSound; }

namespace Scene {

/**
 * Sound node.
 * Acts as a simple node wrapping a sound source.
 *
 * @class SoundNode SoundNode.h Scene/SoundNode.h
 */
class SoundNode : public ISceneNode {
    OE_SCENE_NODE(SoundNode, ISceneNode)
    
public:
    SoundNode(Sound::IMonoSound* sound);
    SoundNode(const SoundNode& node);
    virtual ~SoundNode();

    Sound::IMonoSound* GetSound();

private:
    Sound::IMonoSound* sound;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        // serialize base class information
        ar & boost::serialization::base_object<ISceneNode>(*this);
    }

};

} // NS Scene
} // NS OpenEngine

BOOST_CLASS_EXPORT(OpenEngine::Scene::SoundNode)

#endif // _OE_SOUND_NODE_H_
