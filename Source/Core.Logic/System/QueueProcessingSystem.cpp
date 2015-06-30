#include "stdafx.h"

#include "QueueProcessingSystem.h"

#include "Entity/Entity.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
QueueProcessingSystem::QueueProcessingSystem() {}
//----------------------------------------------------------------------------
QueueProcessingSystem::~QueueProcessingSystem() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_entities.empty());
}
//----------------------------------------------------------------------------
void QueueProcessingSystem::Initialize(EntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _entities.reserve(32);
}
//----------------------------------------------------------------------------
void QueueProcessingSystem::Destroy(EntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(_entities.empty());
    _entities.shrink_to_fit();
}
//----------------------------------------------------------------------------
void QueueProcessingSystem::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (EntityID id : _entities)
        ProcessEntity(timeline, id);

    _entities.clear();
}
//----------------------------------------------------------------------------
void QueueProcessingSystem::OnEntityDeleted(const Entity& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Remove_ReturnIfExists(_entities, entity.ID());
}
//----------------------------------------------------------------------------
void QueueProcessingSystem::OnEntityRefresh(const Entity& /* entity */, ComponentFlag /* previousComponents */) {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void QueueProcessingSystem::Queue(EntityID id) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Entity::InvalidID != id);

    _entities.push_back(id);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
