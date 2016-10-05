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
class FEntityContainer {
public:
    FEntityContainer();
    ~FEntityContainer();

    EntityID CreateEntity(const EntityUID *optionalUID = nullptr);
    void DestroyEntity(EntityID id);

    const VECTOR(FEntity, EntityID)& UsedIDs() const { return _usedIDs; }

    FEntity& Get(EntityID id) { Assert(id < _nextAvailableID); return _entities[id]; }
    const FEntity& Get(EntityID id) const { Assert(id < _nextAvailableID); return _entities[id]; }

    bool Contains(EntityID id) const;

    EntityID IDFromUID(EntityUID uid) const;

    size_t EntitiesInUse() const { return _usedIDs.size(); }
    size_t EntitiesCapacity() const { return _nextAvailableID; }
    size_t EntitiesAvailable() const { return _freeIDs.size(); }

    void Clear();
    void ShrinkToFit();

private:
    VECTOR(FEntity, FEntity) _entities;

    VECTOR(FEntity, EntityID) _usedIDs;
    VECTOR(FEntity, EntityID) _freeIDs;
    EntityID _nextAvailableID;

    HASHMAP(FEntity, EntityUID, EntityID) _uidToId;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
