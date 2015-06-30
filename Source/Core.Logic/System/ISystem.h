#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/System/System_fwd.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;

namespace Logic {
class EntityManager;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISystem : public RefCountable {
public:
    virtual ~ISystem() {}

    virtual bool Enabled() const = 0;
    
    virtual void Initialize(EntityManager& manager) = 0;
    virtual void Destroy(EntityManager& manager) = 0;

    virtual void Update(const Timeline& timeline) = 0;

    virtual void OnEntityDeleted(const Entity& entity) = 0;
    virtual void OnEntityRefresh(const Entity& entity, ComponentFlag previousComponents) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
