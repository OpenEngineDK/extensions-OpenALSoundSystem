// preprocesser for visualizing sound.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _SOUND_RENDERER_H_
#define _SOUND_RENDERER_H_

//#include <Scene/ISceneNodeVisitor.h>
#include <Core/IListener.h>
#include <Renderers/IRenderer.h>

#include <list>
#include <map>

namespace OpenEngine {
//forward reference
    namespace Scene {
        class SoundNode;
        class TransformationNode;
    }

namespace Sound {


    //using OpenEngine::Scene::ISceneNodeVisitor;
using OpenEngine::Core::IListener;
using OpenEngine::Renderers::RenderingEventArg;
using OpenEngine::Scene::SoundNode;
using OpenEngine::Scene::TransformationNode;

using namespace std;

class SoundRenderer : public IListener<RenderingEventArg> {
private:
    list<SoundNode*> soundlist;
    map<SoundNode*, TransformationNode*> soundmap;
public:
    
    SoundRenderer(); 
    virtual ~SoundRenderer();
    
    void Handle(RenderingEventArg arg);
    
    void AddSoundNode(SoundNode* node);
    void RemoveSoundNode(SoundNode* node);

};

} // NS Sound
} // NS OpenEngine

#endif
