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

namespace OpenEngine {
    //forward declarations
    namespace Scene {
        class ISceneNode;
    }
namespace Sound {

    //forward decl
    class IMonoSound;
	class IStereoSound;

using OpenEngine::Core::IModule;
using OpenEngine::Resources::ISoundResourcePtr;
using OpenEngine::Scene::ISceneNode;

class ISoundSystem : public IModule {
public:
    virtual ~ISoundSystem() {};

    virtual IMonoSound* CreateMonoSound(ISoundResourcePtr resource) = 0;
	virtual IStereoSound* CreateStereoSound(ISoundResourcePtr resource) = 0;
    virtual void SetRoot(ISceneNode* node) = 0;
	bool IsTypeOf(const std::type_info& inf) { 
        return typeid(ISoundSystem) == inf; 
    }
};

} // NS Sound
} // NS OpenEngine

#endif
