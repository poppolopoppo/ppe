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
class FEntityTagContainer {
public:
    FEntityTagContainer();
    ~FEntityTagContainer();

    FEntityTagContainer(const FEntityTagContainer& ) = delete;
    FEntityTagContainer& operator =(const FEntityTagContainer& ) = delete;

    void AddTag(EntityID id, const FEntityTag& tag);
    void RemoveTag(EntityID id, const FEntityTag& tag);

    bool HasTag(EntityID id, const FEntityTag& tag) const;

    void RemoveEntity(EntityID id);

    size_t EntityTags(FEntityTag *pTags, size_t capacity, EntityID id) const;
    void EntityTags(VECTOR_THREAD_LOCAL(FEntity, FEntityTag)& outTags, EntityID id) const;

    const VECTOR(FEntity, EntityID)& TagEntities(const FEntityTag& tag) const;

    void Clear();

private:
    typedef VECTOR(FEntity, EntityID) Entities;
    typedef HASHMAP(FEntity, FEntityTag, Entities) TagToEntities;
    typedef MULTIMAP(FEntity, EntityID, FEntityTag) EntityToTags;

    TagToEntities _tagToIDs;
    EntityToTags _idToTags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
