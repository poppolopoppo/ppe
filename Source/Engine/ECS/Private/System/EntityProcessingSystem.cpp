#include "stdafx.h"

#include "EntityProcessingSystem.h"

#include "Component/ComponentContainer.h"
#include "Component/ComponentTag.h"

#include "Entity/Entity.h"
#include "Entity/EntityContainer.h"

#include "EntityManager.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEntityProcessingSystem::FEntityProcessingSystem(const FSystemAspect& aspect)
:   _aspect(aspect) {}
//----------------------------------------------------------------------------
FEntityProcessingSystem::~FEntityProcessingSystem() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FEntityProcessingSystem::Initialize(FEntityManager& manager) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const FEntityContainer& entities = manager.Entities();

    for (EntityID id : entities.UsedIDs()) {
        const FEntity& entity = entities.Get(id);
        if (_aspect.Matches(entity.ComponentFlags()))
            _entities.push_back(id);
    }
}
//----------------------------------------------------------------------------
void FEntityProcessingSystem::Destroy(FEntityManager& /* manager */) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _entities.clear();
    _entities.shrink_to_fit();
}
//----------------------------------------------------------------------------
void FEntityProcessingSystem::Update(const FTimeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (EntityID entityID : _entities)
        ProcessEntity(timeline, entityID);
}
//----------------------------------------------------------------------------
void FEntityProcessingSystem::OnEntityDeleted(const FEntity& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const bool matches = _aspect.Matches(entity.ComponentFlags());

    if (matches)
        Remove_DontPreserveOrder(_entities, entity.ID());
    else
        Assert(!Contains(_entities, entity.ID()));
}
//----------------------------------------------------------------------------
void FEntityProcessingSystem::OnEntityRefresh(const FEntity& entity, ComponentFlag previousComponents) {
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
} //!namespace PPE
