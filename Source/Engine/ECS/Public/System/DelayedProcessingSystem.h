#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Entity/Entity_fwd.h"

#include "Core.Logic/System/ISystem.h"

#include "Container/Vector.h"
#include "Meta/ThreadResource.h"
#include "Time/Timepoint.h"

namespace PPE {
class FTimepoint;

namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DelayedProcessingSystem);
//----------------------------------------------------------------------------
class FDelayedProcessingSystem : public ISystem, Meta::FThreadResource {
public:
    struct FDelayedProcess {
        FTimepoint   Date;
        EntityID    ID;

        FDelayedProcess() {}
        FDelayedProcess(const FTimepoint& date, EntityID id)
        :   Date(date), ID(id) {}
    };

    FDelayedProcessingSystem();
    virtual ~FDelayedProcessingSystem();

    const VECTOR(System, FDelayedProcess)& Entities() const { return _entities; }

    virtual bool Enabled() const override { return true; }

    virtual void Initialize(FEntityManager& manager) override;
    virtual void Destroy(FEntityManager& manager) override;

    virtual void Update(const FTimeline& timeline) override;

    virtual void OnEntityDeleted(const FEntity& entity) override;
    virtual void OnEntityRefresh(const FEntity& entity, ComponentFlag previousComponents) override;

    void Queue(const FDelayedProcess& data);
    void Queue(const FTimepoint& date, EntityID id);

protected:
    virtual void ProcessEntity(const FTimeline& timeline, EntityID entityID) = 0;

private:
    VECTOR(System, FDelayedProcess) _entities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
