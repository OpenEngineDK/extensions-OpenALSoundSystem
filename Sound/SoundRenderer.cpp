// preprocesser for visualizing sound.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Sound/SoundRenderer.h>

#include <Scene/TransformationNode.h>
#include <Scene/SphereNode.h>
#include <Scene/SoundNode.h>
#include <Math/Matrix.h>
#include <Sound/ISound.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Scene::SphereNode;
using OpenEngine::Math::Matrix;

SoundRenderer::SoundRenderer() {
    
} 

SoundRenderer::~SoundRenderer() {
    //    for (list<iterator)
}
    
void SoundRenderer::Handle(RenderingEventArg arg) {
    for (list<SoundNode*>::iterator itr = soundlist.begin(); 
         itr != soundlist.end();
         itr++) {
        SoundNode* sn = *itr;
        ISound* s = sn->GetSound();
        TransformationNode* tn = soundmap[sn];
        
        float dist = s->GetMaxDistance();
        tn->SetScale(Matrix<4,4,float>(dist,0.0,0.0,0.0,
                                       0.0,dist,0.0,0.0,
                                       0.0,0.0,dist,0.0,
                                       0.0,0.0,0.0,1.0));
    }
}
    
void SoundRenderer::AddSoundNode(SoundNode* node) {
    TransformationNode* tn = new TransformationNode();
    SphereNode* sn = new SphereNode();
    tn->AddNode(sn);
    node->AddNode(tn);

    soundlist.push_back(node);
    soundmap[node] = tn;
}

void SoundRenderer::RemoveSoundNode(SoundNode* node) {
    TransformationNode* tn = soundmap[node];
    if (tn == NULL) return;
    
    for (list<SoundNode*>::iterator itr = soundlist.begin();
         itr != soundlist.end();
         itr++) {
        if (node == *itr) {
            soundlist.erase(itr);
            break;
        }
    }
    soundmap.erase(node);
    tn->DeleteAllNodes();
    tn->GetParent()->DeleteNode(tn);
}


} // NS Sound
} // NS OpenEngine

