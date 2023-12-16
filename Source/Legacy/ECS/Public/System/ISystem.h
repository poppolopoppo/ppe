#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/System/System_fwd.h"

#include "Memory/RefPtr.h"

namespace PPE {
class FTimeline;

namespace Logic {
class FEntityManager;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISystem : public FRefCountable {
public:
    virtual ~ISystem() {}

    virtual bool Enabled() const = 0;
    
    virtual void Initialize(FEntityManager& manager) = 0;
    virtual void Destroy(FEntityManager& manager) = 0;

    virtual void Update(const FTimeline& timeline) = 0;

    virtual void OnEntityDeleted(const FEntity& entity) = 0;
    virtual void OnEntityRefresh(const FEntity& entity, ComponentFlag previousComponents) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
