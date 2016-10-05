#include "stdafx.h"

#include "DelayedProcessingSystem.h"

#include "Entity/Entity.h"

#include "Core/Time/Timeline.h"
#include "Core/Time/Timepoint.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDelayedProcessingSystem::FDelayedProcessingSystem() {}
//----------------------------------------------------------------------------
FDelayedProcessingSystem::~FDelayedProcessingSystem() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_entities.empty());
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::Initialize(FEntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::Destroy(FEntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(_entities.empty());
    _entities.clear();
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::Update(const FTimeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const FTimepoint& now = timeline.Now();

    for (VECTOR(System, FDelayedProcess)::const_iterator it = _entities.begin(); it != _entities.end(); )
        if (it->Date <= now) {
            ProcessEntity(timeline, it->ID);
            Erase_DontPreserveOrder(_entities, it);
        }
        else
            ++it;
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::Queue(const FDelayedProcess& data)
{
    _entities.push_back(data);
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::Queue(const FTimepoint& date, EntityID id)
{
    _entities.emplace_back(date, id);
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::OnEntityDeleted(const FEntity& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (VECTOR(System, FDelayedProcess)::const_iterator it = _entities.begin(); it != _entities.end(); ) 
        if (it->ID == entity.ID())
            Erase_DontPreserveOrder(_entities, it);
        else
            ++it;
}
//----------------------------------------------------------------------------
void FDelayedProcessingSystem::OnEntityRefresh(const FEntity& /* entity */, ComponentFlag /* previousComponents */) {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
