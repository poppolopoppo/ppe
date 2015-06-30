#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/ComponentContainer.h"
#include "Core.Logic/Entity/EntityContainer.h"
#include "Core.Logic/Entity/EntityTagContainer.h"
#include "Core.Logic/System/SystemContainer.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Vector.h"

namespace Core {
class Timeline;
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    const ComponentContainer& Components() const { return _components; }
    const EntityContainer& Entities() const { return _entities; }
    const EntityTagContainer& Tags() const { return _tags; }
    const SystemContainer& Systems() const { return _systems; }

    void Initialize();
    void Destroy();

    EntityID CreateEntity(const EntityUID *optionalUID = nullptr);
    void DeleteEntity(EntityID id);

    void Refresh(EntityID id, ComponentFlag previousComponents);

    template <typename T>
    void AddComponent(EntityID id, T&& data);
    void RemoveComponent(ComponentTag tag, EntityID id);

    template <typename T>
    void AddComponent(ComponentID componentID, EntityID entityID, T&& data);
    void RemoveComponent(ComponentID componentID, EntityID entityID);

    void AddTag(EntityID id, const EntityTag& tag);
    void RemoveTag(EntityID id, const EntityTag& tag);

    void Update(const Timeline& timeline);

    void ShrinkToFit();
    void Clear();

private:
    ComponentContainer _components;
    EntityContainer _entities;
    EntityTagContainer _tags;
    SystemContainer _systems;

    VECTOR(Entity, EntityID) _deleting;
    ASSOCIATIVE_VECTOR(Entity, EntityID, ComponentFlag) _refreshing;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core

#include "Core.Logic/EntityManager-impl.h"
