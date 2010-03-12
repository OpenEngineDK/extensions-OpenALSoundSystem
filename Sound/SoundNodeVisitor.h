// OpenAL sound node visitor implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENAL_SOUND_NODE_VISITOR_H_
#define _OPENAL_SOUND_NODE_VISITOR_H_

#include <Scene/ISceneNodeVisitor.h> 
#include <Scene/TransformationNode.h>
#include <Scene/SoundNode.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>

#include <map>
#include <stack>

namespace OpenEngine {
namespace Sound {

using namespace OpenEngine::Scene;
using std::map;
using std::stack;

class SoundNodeVisitor : public ISceneNodeVisitor {
private:
    //the current position and rotation
    // Vector<3, float> pos;
    // Quaternion<float> dir;
    // map<IMonoSound*,Vector<3,float> > soundmap;
    stack<Vector<3,float> > positions;
    float deltaTime;
public:
    SoundNodeVisitor();
    ~SoundNodeVisitor();

    void SetDeltaTime(float dt);

    void VisitTransformationNode(TransformationNode* node);
    void VisitSoundNode(SoundNode* node);
};

} // NS Sound
} // NS OpenEngine

#endif //_OPENAL_SOUND_NODE_VISITOR_H_
