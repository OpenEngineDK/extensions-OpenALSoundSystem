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
#include <Sound/IMonoSound.h>
#include <Geometry/Material.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Sound {

using OpenEngine::Scene::SphereNode;
using OpenEngine::Math::Matrix;
using OpenEngine::Geometry::Material;
using OpenEngine::Geometry::MaterialPtr;

SoundRenderer::SoundRenderer() {
    
} 

SoundRenderer::~SoundRenderer() {
    // TODO: cleanup
}
    
void SoundRenderer::Handle(RenderingEventArg arg) {
    for (list<SoundNode*>::iterator itr = soundlist.begin(); 
         itr != soundlist.end();
         itr++) {
        SoundNode* sn = *itr;
        IMonoSound* s = sn->GetSound();
        TransformationNode* tn = soundmap[sn];
        
        float dist = s->GetMaxDistance();
        tn->SetScale(Vector<3,float>(dist));
    }
}
    
void SoundRenderer::AddSoundNode(SoundNode* node, Vector<3,float> col) {
    TransformationNode* tn = new TransformationNode();
    MaterialPtr m = MaterialPtr(new Material());
    m->ambient = Vector<4,float>(col[0],col[1],col[2],.5);
    //m->shininess = 100;
    //m->specular = Vector<4,float>(0.0,0.0,0.0,0.5);
    m->diffuse = Vector<4,float>(.1,.1,.1,1.0);
    //m->emission = Vector<4,float>(0.0,0.2,0.0,1.0);
    
    SphereNode* sn = new SphereNode(m);
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

