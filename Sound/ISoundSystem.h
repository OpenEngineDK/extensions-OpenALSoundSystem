// Sound system interface
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _INTERFACE_SOUND_SYSTEM_H_
#define _INTERFACE_SOUND_SYSTEM_H_

#include <Core/IModule.h>
#include <Resources/ISoundResource.h>
#include <Resources/IStreamingSoundResource.h>

#include <Core/IListener.h>
#include <Renderers/IRenderer.h>

#include <string>

namespace OpenEngine {
    //forward declarations
    namespace Scene {
        class ISceneNode;
    }
    namespace Sound {
        class ISound;

using Core::IModule;
using Core::IListener;
using Resources::ISoundResourcePtr;
using Resources::IStreamingSoundResourcePtr;
using Scene::ISceneNode;
using Renderers::RenderingEventArg;

using std::string;

class ISoundSystem : public IModule, public IListener<RenderingEventArg> {
public:
    virtual ~ISoundSystem() {};

    virtual ISound* CreateSound(ISoundResourcePtr resource) = 0;
    virtual ISound* CreateSound(IStreamingSoundResourcePtr resource) = 0;
    //virtual void SetRoot(ISceneNode* node) = 0;
	virtual void SetMasterGain(float gain) = 0;
	virtual float GetMasterGain() = 0;
	bool IsTypeOf(const std::type_info& inf) { 
        return typeid(ISoundSystem) == inf; 
    }
    
    virtual unsigned int GetDeviceCount() = 0;
    virtual string GetDeviceName(unsigned int device) = 0;
    virtual void SetDevice(unsigned int device) = 0;
};

} // NS Sound
} // NS OpenEngine

#endif
