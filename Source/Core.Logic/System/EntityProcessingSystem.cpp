#include "stdafx.h"

#include "EntityProcessingSystem.h"

#include "Component/ComponentContainer.h"
#include "Component/ComponentTag.h"

#include "Entity/Entity.h"
#include "Entity/EntityContainer.h"

#include "EntityManager.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EntityProcessingSystem::EntityProcessingSystem(const SystemAspect& aspect)
:   _aspect(aspect) {}
//----------------------------------------------------------------------------
EntityProcessingSystem::~EntityProcessingSystem() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void EntityProcessingSystem::Initialize(EntityManager& manager) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const EntityContainer& entities = manager.Entities();

    for (EntityID id : entities.UsedIDs()) {
        const Entity& entity = entities.Get(id);
        if (_aspect.Matches(entity.ComponentFlags()))
            _entities.push_back(id);
    }
}
//----------------------------------------------------------------------------
void EntityProcessingSystem::Destroy(EntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _entities.clear();
    _entities.shrink_to_fit();
}
//----------------------------------------------------------------------------
void EntityProcessingSystem::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (EntityID entityID : _entities)
        ProcessEntity(timeline, entityID);
}
//----------------------------------------------------------------------------
void EntityProcessingSystem::OnEntityDeleted(const Entity& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const bool matches = _aspect.Matches(entity.ComponentFlags());

    if (matches)
        Remove_DontPreserveOrder(_entities, entity.ID());
    else
        Assert(!Contains(_entities, entity.ID()));
}
//----------------------------------------------------------------------------
void EntityProcessingSystem::OnEntityRefresh(const Entity& entity, ComponentFlag previousComponents) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const bool matches = _aspect.Matches(entity.ComponentFlags());
    const bool matched = _aspect.Matches(previousComponents);
    
    if (matches && !matched) {
        Add_AssertUnique(_entities, entity.ID());
    }
    else if (!matches && matched) {
        Remove_DontPreserveOrder(_entities, entity.ID());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
