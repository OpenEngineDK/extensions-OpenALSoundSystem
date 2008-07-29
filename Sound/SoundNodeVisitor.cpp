#include <Sound/SoundNodeVisitor.h>

#include <Core/Exceptions.h>
#include <Sound/ISound.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Core::Exception;

SoundNodeVisitor::SoundNodeVisitor(float deltaTime): deltaTime(deltaTime) {
    //init to assumed startposition
    pos = Vector<3,float>(0.0, 0.0, 0.0);

    //init to assumed direction
    dir = new Quaternion<float>();
}

SoundNodeVisitor::~SoundNodeVisitor() {
    delete(dir);
}

void SoundNodeVisitor::VisitTransformationNode(TransformationNode* node) {
	//get the change in position from the transformation and apply to current
	Vector<3, float> transpos = node->GetPosition();
	pos = pos + transpos;

	//get the change in dir from the transformation and apply to current
	Quaternion<float> transdir = node->GetRotation();
	(*dir) = (*dir) * transdir;

	node->VisitSubNodes(*this);

	//take of transformation again
	pos = pos - transpos;

	//take of turn again
	(*dir) = (*dir) * (transdir.GetInverse());
}

void SoundNodeVisitor::VisitSoundNode(SoundNode* node) {
    //setup the source settings
    ISound* s = node->GetSound();
    Vector<3,float> prevPos = soundmap[s];
    soundmap[s] = pos;
    s->SetVelocity((pos - prevPos)* (1/(deltaTime*1000)));
    s->SetPosition(pos);
    s->SetRotation(*dir);

    node->VisitSubNodes(*this);
}

} // NS Sound
} // NS OpenEngine
