#include "stdafx.h"

#include "EntityManager.h"

#include "Component/ComponentTag.h"
#include "Component/IComponent.h"

#include "Entity/Entity.h"
#include "Entity/EntityTag.h"

#include "Core/Time/Timeline.h"
#include "Core/Time/Timepoint.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EntityManager::EntityManager() 
:   _systems(*this) {
}
//----------------------------------------------------------------------------
EntityManager::~EntityManager() {
}
//----------------------------------------------------------------------------
void EntityManager::Initialize() {
    _systems.Initialize();
}
//----------------------------------------------------------------------------
void EntityManager::Destroy() {
    _systems.Destroy();
}
//----------------------------------------------------------------------------
EntityID EntityManager::CreateEntity(const EntityUID *optionalUID/* = nullptr */) {
    return _entities.CreateEntity(optionalUID);
}
//----------------------------------------------------------------------------
void EntityManager::DeleteEntity(EntityID id) {
    Entity& entity = _entities.Get(id);
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
void EntityManager::Refresh(EntityID id, ComponentFlag previousComponents) {
    Entity& entity = _entities.Get(id);
    if (entity.Refreshing() || entity.Deleting())
        return;

    entity._refreshing = true;
    _refreshing.Insert_AssertUnique(id, previousComponents);
}
//----------------------------------------------------------------------------
void EntityManager::RemoveComponent(ComponentTag componentTag, EntityID id) {
    const ComponentID componentID = _components.ID(componentTag);
    RemoveComponent(componentID, id);
}
//----------------------------------------------------------------------------
void EntityManager::RemoveComponent(ComponentID componentID, EntityID entityID) {
    _components.GetByID(componentID)->RemoveComponent(entityID);

    Entity& entity = _entities.Get(entityID);
    Assert(0 != (entity._componentFlags & (1<<componentID)) );

    const ComponentFlag previousComponents = entity._componentFlags;
    entity._componentFlags = entity._componentFlags & (~(1<<componentID));

    Refresh(entityID, previousComponents);
}
//----------------------------------------------------------------------------
void EntityManager::AddTag(EntityID id, const EntityTag& tag) {
    Assert(_entities.Contains(id));

    _tags.AddTag(id, tag);
}
//----------------------------------------------------------------------------
void EntityManager::RemoveTag(EntityID id, const EntityTag& tag) {
    Assert(_entities.Contains(id));

    _tags.RemoveTag(id, tag);
}
//----------------------------------------------------------------------------
void EntityManager::Update(const Timeline& timeline) {
    const Timespan delta = timeline.Elapsed();

    if (_deleting.size()) {
        for (EntityID const id : _deleting) {
            Entity& entity = _entities.Get(id);
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
        for (const Pair<EntityID, ComponentFlag>& it : _refreshing) {
            Entity& entity = _entities.Get(it.first);
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
void EntityManager::ShrinkToFit() {
    _entities.ShrinkToFit();
}
//----------------------------------------------------------------------------
void EntityManager::Clear() {
    _components.Clear();
    _entities.Clear();
    _tags.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
