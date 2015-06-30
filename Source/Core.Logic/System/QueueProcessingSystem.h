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
class QueueProcessingSystem : public ISystem, Meta::ThreadResource {
public:
    QueueProcessingSystem();
    virtual ~QueueProcessingSystem();

    const VECTOR(System, EntityID)& Entities() const { return _entities; }

    virtual bool Enabled() const override { return true; }
    
    virtual void Initialize(EntityManager& manager) override;
    virtual void Destroy(EntityManager& manager) override;

    virtual void Update(const Timeline& timeline) override;

    virtual void OnEntityDeleted(const Entity& entity) override;
    virtual void OnEntityRefresh(const Entity& entity, ComponentFlag previousComponents) override;

    void Queue(EntityID id);

protected:
    virtual void ProcessEntity(const Timeline& timeline, EntityID entityID) = 0;

private:
    VECTOR(System, EntityID) _entities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
