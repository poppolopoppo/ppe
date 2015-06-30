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
DelayedProcessingSystem::DelayedProcessingSystem() {}
//----------------------------------------------------------------------------
DelayedProcessingSystem::~DelayedProcessingSystem() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_entities.empty());
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::Initialize(EntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::Destroy(EntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(_entities.empty());
    _entities.clear();
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const Timepoint& now = timeline.Now();

    for (VECTOR(System, DelayedProcess)::const_iterator it = _entities.begin(); it != _entities.end(); )
        if (it->Date <= now) {
            ProcessEntity(timeline, it->ID);
            Erase_DontPreserveOrder(_entities, it);
        }
        else
            ++it;
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::Queue(const DelayedProcess& data)
{
    _entities.push_back(data);
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::Queue(const Timepoint& date, EntityID id)
{
    _entities.emplace_back(date, id);
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::OnEntityDeleted(const Entity& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (VECTOR(System, DelayedProcess)::const_iterator it = _entities.begin(); it != _entities.end(); ) 
        if (it->ID == entity.ID())
            Erase_DontPreserveOrder(_entities, it);
        else
            ++it;
}
//----------------------------------------------------------------------------
void DelayedProcessingSystem::OnEntityRefresh(const Entity& /* entity */, ComponentFlag /* previousComponents */) {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
