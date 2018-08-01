#include "stdafx.h"

#include "EntityManager.h"

#include "Component/ComponentTag.h"
#include "Component/IComponent.h"

#include "Entity/Entity.h"
#include "Entity/EntityTag.h"

#include "Time/Timeline.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEntityManager::FEntityManager() 
:   _systems(*this) {
}
//----------------------------------------------------------------------------
FEntityManager::~FEntityManager() {
}
//----------------------------------------------------------------------------
void FEntityManager::Initialize() {
    _systems.Initialize();
}
//----------------------------------------------------------------------------
void FEntityManager::Destroy() {
    _systems.Destroy();
}
//----------------------------------------------------------------------------
EntityID FEntityManager::CreateEntity(const EntityUID *optionalUID/* = nullptr */) {
    return _entities.CreateEntity(optionalUID);
}
//----------------------------------------------------------------------------
void FEntityManager::DeleteEntity(EntityID id) {
    FEntity& entity = _entities.Get(id);
    if (entity.Deleting())
        return;

    // deleting an entity cancel refresh status, hence systems are only summoned once
    if (entity.Refreshing()) {
        _refreshing.Remove_AssertExists(id);
        entity._refreshing = false;
    }

    entity._deleting = true;
    Add_AssertUnique(_deleting, id);
}
//----------------------------------------------------------------------------
void FEntityManager::Refresh(EntityID id, ComponentFlag previousComponents) {
    FEntity& entity = _entities.Get(id);
    if (entity.Refreshing() || entity.Deleting())
        return;

    entity._refreshing = true;
    _refreshing.Insert_AssertUnique(id, previousComponents);
}
//----------------------------------------------------------------------------
void FEntityManager::RemoveComponent(ComponentTag componentTag, EntityID id) {
    const ComponentID componentID = _components.ID(componentTag);
    RemoveComponent(componentID, id);
}
//----------------------------------------------------------------------------
void FEntityManager::RemoveComponent(ComponentID componentID, EntityID entityID) {
    _components.GetByID(componentID)->RemoveComponent(entityID);

    FEntity& entity = _entities.Get(entityID);
    Assert(0 != (entity._componentFlags & (1<<componentID)) );

    const ComponentFlag previousComponents = entity._componentFlags;
    entity._componentFlags = ComponentFlag(entity._componentFlags & (~(1<<componentID)) );

    Refresh(entityID, previousComponents);
}
//----------------------------------------------------------------------------
void FEntityManager::AddTag(EntityID id, const FEntityTag& tag) {
    Assert(_entities.Contains(id));

    _tags.AddTag(id, tag);
}
//----------------------------------------------------------------------------
void FEntityManager::RemoveTag(EntityID id, const FEntityTag& tag) {
    Assert(_entities.Contains(id));

    _tags.RemoveTag(id, tag);
}
//----------------------------------------------------------------------------
void FEntityManager::Update(const FTimeline& timeline) {
    const Timespan delta = timeline.Elapsed();

    if (_deleting.size()) {
        for (EntityID const id : _deleting) {
            FEntity& entity = _entities.Get(id);
            Assert(entity.Deleting());

            _systems.RemoveEntity(entity);
            _components.RemoveEntity(entity);
            _tags.RemoveEntity(id);

            _entities.DestroyEntity(id);
        }

        _deleting.clear();
    }
    Assert(_deleting.empty());

    if (_refreshing.size()) {
        for (const TPair<EntityID, ComponentFlag>& it : _refreshing) {
            FEntity& entity = _entities.Get(it.first);
            Assert(entity.Refreshing());

            _systems.RefreshEntity(entity, it.second);

            entity._refreshing = false;
        }

        _refreshing.clear();
    }
    Assert(_refreshing.empty());

    _systems.Process(timeline);
}
//----------------------------------------------------------------------------
void FEntityManager::ShrinkToFit() {
    _entities.ShrinkToFit();
}
//----------------------------------------------------------------------------
void FEntityManager::Clear() {
    _components.Clear();
    _entities.Clear();
    _tags.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
