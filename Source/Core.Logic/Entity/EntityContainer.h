#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/Entity/Entity.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EntityContainer {
public:
    EntityContainer();
    ~EntityContainer();

    EntityID CreateEntity(const EntityUID *optionalUID = nullptr);
    void DestroyEntity(EntityID id);

    const VECTOR(Entity, EntityID)& UsedIDs() const { return _usedIDs; }

    Entity& Get(EntityID id) { Assert(id < _nextAvailableID); return _entities[id]; }
    const Entity& Get(EntityID id) const { Assert(id < _nextAvailableID); return _entities[id]; }

    bool Contains(EntityID id) const;

    EntityID IDFromUID(EntityUID uid) const;

    size_t EntitiesInUse() const { return _usedIDs.size(); }
    size_t EntitiesCapacity() const { return _nextAvailableID; }
    size_t EntitiesAvailable() const { return _freeIDs.size(); }

    void Clear();
    void ShrinkToFit();

private:
    VECTOR(Entity, Entity) _entities;

    VECTOR(Entity, EntityID) _usedIDs;
    VECTOR(Entity, EntityID) _freeIDs;
    EntityID _nextAvailableID;

    HASHMAP(Entity, EntityUID, EntityID) _uidToId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
