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

SoundNode::SoundNode(ISound* sound) : sound(sound) { }

/**
* Copy constructor.
* Performs a shallow copy.
*
* @param node Sound node to copy.
*/
SoundNode::SoundNode(SoundNode& node) { 
    sound = node.sound;
}

SoundNode::~SoundNode() {}

ISceneNode* SoundNode::CloneSelf() {
  SoundNode* clone = new SoundNode(*this);
  return clone;
}


void SoundNode::Accept(ISceneNodeVisitor& v) {
  v.VisitSoundNode(this);
}

ISound* SoundNode::GetSound() {
    return sound;
}
    
} //NS Scene
} //NS OpenEngine
