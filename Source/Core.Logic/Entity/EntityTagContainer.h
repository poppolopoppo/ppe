#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Entity/Entity_fwd.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/MultiMap.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EntityTagContainer {
public:
    EntityTagContainer();
    ~EntityTagContainer();

    EntityTagContainer(const EntityTagContainer& ) = delete;
    EntityTagContainer& operator =(const EntityTagContainer& ) = delete;

    void AddTag(EntityID id, const EntityTag& tag);
    void RemoveTag(EntityID id, const EntityTag& tag);

    bool HasTag(EntityID id, const EntityTag& tag) const;

    void RemoveEntity(EntityID id);

    size_t EntityTags(EntityTag *pTags, size_t capacity, EntityID id) const;
    void EntityTags(VECTOR_THREAD_LOCAL(Entity, EntityTag)& outTags, EntityID id) const;

    const VECTOR(Entity, EntityID)& TagEntities(const EntityTag& tag) const;

    void Clear();

private:
    typedef VECTOR(Entity, EntityID) Entities;
    typedef HASHMAP(Entity, EntityTag, Entities) TagToEntities;
    typedef MULTIMAP(Entity, EntityID, EntityTag) EntityToTags;

    TagToEntities _tagToIDs;
    EntityToTags _idToTags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
