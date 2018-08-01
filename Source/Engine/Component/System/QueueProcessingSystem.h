#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Entity/Entity_fwd.h"

#include "Core.Logic/System/ISystem.h"

#include "Core/Container/Vector.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(QueueProcessingSystem);
//----------------------------------------------------------------------------
class FQueueProcessingSystem : public ISystem, Meta::FThreadResource {
public:
    FQueueProcessingSystem();
    virtual ~FQueueProcessingSystem();

    const VECTOR(System, EntityID)& Entities() const { return _entities; }

    virtual bool Enabled() const override { return true; }
    
    virtual void Initialize(FEntityManager& manager) override;
    virtual void Destroy(FEntityManager& manager) override;

    virtual void Update(const FTimeline& timeline) override;

    virtual void OnEntityDeleted(const FEntity& entity) override;
    virtual void OnEntityRefresh(const FEntity& entity, ComponentFlag previousComponents) override;

    void Queue(EntityID id);

protected:
    virtual void ProcessEntity(const FTimeline& timeline, EntityID entityID) = 0;

private:
    VECTOR(System, EntityID) _entities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
