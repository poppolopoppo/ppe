#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Entity/Entity_fwd.h"

#include "Core.Logic/System/ISystem.h"
#include "Core.Logic/System/SystemAspect.h"

#include "Container/Vector.h"
#include "Meta/ThreadResource.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(EntityProcessingSystem);
//----------------------------------------------------------------------------
class FEntityProcessingSystem : public ISystem, Meta::FThreadResource {
public:
    explicit FEntityProcessingSystem(const FSystemAspect& aspect);
    virtual ~FEntityProcessingSystem();

    const FSystemAspect& Aspect() const { return _aspect; }
    const VECTOR(System, EntityID)& Entities() const { return _entities; }

    virtual bool Enabled() const override { return true; }
    
    virtual void Initialize(FEntityManager& manager) override;
    virtual void Destroy(FEntityManager& manager) override;

    virtual void Update(const FTimeline& timeline) override;
    
    virtual void OnEntityDeleted(const FEntity& entity) override;
    virtual void OnEntityRefresh(const FEntity& entity, ComponentFlag previousComponents) override;

protected:
    virtual void ProcessEntity(const FTimeline& timeline, EntityID entityID) = 0;

private:
    FSystemAspect _aspect;
    VECTOR(System, EntityID) _entities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
