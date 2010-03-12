#include <Sound/SoundNodeVisitor.h>

#include <Core/Exceptions.h>
#include <Sound/IMonoSound.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Core::Exception;

SoundNodeVisitor::SoundNodeVisitor() {
    //init to assumed startposition
    positions.push(Vector<3,float>(0.0, 0.0, 0.0));
}

SoundNodeVisitor::~SoundNodeVisitor() {
}

void SoundNodeVisitor::SetDeltaTime(float dt) {
    deltaTime = dt;
}

void SoundNodeVisitor::VisitTransformationNode(TransformationNode* node) {
    positions.push(positions.top() + node->GetPosition());
	node->VisitSubNodes(*this);
    positions.pop();
}

void SoundNodeVisitor::VisitSoundNode(SoundNode* node) {
    //setup the source settings
    IMonoSound* s = node->GetSound();
    // Vector<3,float> prevPos = soundmap[s];
    // soundmap[s] = pos;
    // s->SetVelocity((pos - prevPos)* (1/(deltaTime*1000)));
    s->SetPosition(positions.top());
    //s->SetRotation(dir);
    node->VisitSubNodes(*this);
}

} // NS Sound
} // NS OpenEngine
