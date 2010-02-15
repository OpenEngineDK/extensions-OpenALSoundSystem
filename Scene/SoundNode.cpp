// SoundNode wrapping an object implementing ISound.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Scene/SoundNode.h>

#include <Sound/ISound.h>

namespace OpenEngine {
namespace Scene {

using OpenEngine::Sound::IMonoSound;

SoundNode::SoundNode()
    : ISceneNode()
    , sound(NULL)
{

}
SoundNode::SoundNode(IMonoSound* sound)
    : ISceneNode()
    , sound(sound)
{

}

/**
 * Copy constructor.
 *
 * @param node Sound node to copy.
 */
SoundNode::SoundNode(const SoundNode& node)
    : ISceneNode(node)
    , sound(node.sound)
{ 

}

SoundNode::~SoundNode() {

}

IMonoSound* SoundNode::GetSound() {
    return sound;
}
    
} //NS Scene
} //NS OpenEngine
