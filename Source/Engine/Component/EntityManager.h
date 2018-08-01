#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/ComponentContainer.h"
#include "Core.Logic/Entity/EntityContainer.h"
#include "Core.Logic/Entity/EntityTagContainer.h"
#include "Core.Logic/System/SystemContainer.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Vector.h"

namespace Core {
class FTimeline;
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEntityManager {
public:
    FEntityManager();
    ~FEntityManager();

    const FComponentContainer& Components() const { return _components; }
    const FEntityContainer& Entities() const { return _entities; }
    const FEntityTagContainer& Tags() const { return _tags; }
    const FSystemContainer& Systems() const { return _systems; }

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

    void AddTag(EntityID id, const FEntityTag& tag);
    void RemoveTag(EntityID id, const FEntityTag& tag);

    void Update(const FTimeline& timeline);

    void ShrinkToFit();
    void Clear();

private:
    FComponentContainer _components;
    FEntityContainer _entities;
    FEntityTagContainer _tags;
    FSystemContainer _systems;

    VECTOR(FEntity, EntityID) _deleting;
    ASSOCIATIVE_VECTOR(FEntity, EntityID, ComponentFlag) _refreshing;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core

#include "Core.Logic/EntityManager-impl.h"
