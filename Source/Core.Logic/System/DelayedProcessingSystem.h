#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Entity/Entity_fwd.h"

#include "Core.Logic/System/ISystem.h"

#include "Core/Container/Vector.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Time/Timepoint.h"

namespace Core {
class Timepoint;

namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DelayedProcessingSystem);
//----------------------------------------------------------------------------
class DelayedProcessingSystem : public ISystem, Meta::ThreadResource {
public:
    struct DelayedProcess {
        Timepoint   Date;
        EntityID    ID;

        DelayedProcess() {}
        DelayedProcess(const Timepoint& date, EntityID id)
        :   Date(date), ID(id) {}
    };

    DelayedProcessingSystem();
    virtual ~DelayedProcessingSystem();

    const VECTOR(System, DelayedProcess)& Entities() const { return _entities; }

    virtual bool Enabled() const override { return true; }

    virtual void Initialize(EntityManager& manager) override;
    virtual void Destroy(EntityManager& manager) override;

    virtual void Update(const Timeline& timeline) override;

    virtual void OnEntityDeleted(const Entity& entity) override;
    virtual void OnEntityRefresh(const Entity& entity, ComponentFlag previousComponents) override;

    void Queue(const DelayedProcess& data);
    void Queue(const Timepoint& date, EntityID id);

protected:
    virtual void ProcessEntity(const Timeline& timeline, EntityID entityID) = 0;

private:
    VECTOR(System, DelayedProcess) _entities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
