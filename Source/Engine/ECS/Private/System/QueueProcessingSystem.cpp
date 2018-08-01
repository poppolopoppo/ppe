#include "stdafx.h"

#include "QueueProcessingSystem.h"

#include "Entity/Entity.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FQueueProcessingSystem::FQueueProcessingSystem() {}
//----------------------------------------------------------------------------
FQueueProcessingSystem::~FQueueProcessingSystem() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_entities.empty());
}
//----------------------------------------------------------------------------
void FQueueProcessingSystem::Initialize(FEntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _entities.reserve(32);
}
//----------------------------------------------------------------------------
void FQueueProcessingSystem::Destroy(FEntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(_entities.empty());
    _entities.shrink_to_fit();
}
//----------------------------------------------------------------------------
void FQueueProcessingSystem::Update(const FTimeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (EntityID id : _entities)
        ProcessEntity(timeline, id);

    _entities.clear();
}
//----------------------------------------------------------------------------
void FQueueProcessingSystem::OnEntityDeleted(const FEntity& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Remove_ReturnIfExists(_entities, entity.ID());
}
//----------------------------------------------------------------------------
void FQueueProcessingSystem::OnEntityRefresh(const FEntity& /* entity */, ComponentFlag /* previousComponents */) {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FQueueProcessingSystem::Queue(EntityID id) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(FEntity::InvalidID != id);

    _entities.push_back(id);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
