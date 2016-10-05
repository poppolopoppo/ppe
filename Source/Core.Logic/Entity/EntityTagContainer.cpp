#include "stdafx.h"

#include "EntityTagContainer.h"

#include "Entity.h"
#include "EntityTag.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEntityTagContainer::FEntityTagContainer() {
    _tagToIDs.reserve(32);
}
//----------------------------------------------------------------------------
FEntityTagContainer::~FEntityTagContainer() {}
//----------------------------------------------------------------------------
void FEntityTagContainer::AddTag(EntityID id, const FEntityTag& tag) {
    Assert(FEntity::InvalidID != id);
    Assert(!tag.empty());

    _idToTags.emplace(id, tag);

    Entities& bag = _tagToIDs[tag];
    bag.push_back(id);
}
//----------------------------------------------------------------------------
void FEntityTagContainer::RemoveTag(EntityID id, const FEntityTag& tag) {
    Assert(FEntity::InvalidID != id);
    Assert(!tag.empty());

    RemoveKeyValue_AssertExists(_idToTags, id, tag);
    
    Entities& bag = _tagToIDs.at(tag);
    Remove_DontPreserveOrder(bag, id);
}
//----------------------------------------------------------------------------
bool FEntityTagContainer::HasTag(EntityID id, const FEntityTag& tag) const {
    Assert(FEntity::InvalidID != id);
    Assert(!tag.empty());

    const TPair<EntityToTags::const_iterator, EntityToTags::const_iterator> range = _idToTags.equal_range(id);
    for (EntityToTags::const_iterator it = range.first; it != range.second; ++it)
        if (it->second == tag)
            return true;

    return false;
}
//----------------------------------------------------------------------------
void FEntityTagContainer::RemoveEntity(EntityID id) {
    Assert(FEntity::InvalidID != id);

    const TPair<EntityToTags::const_iterator, EntityToTags::const_iterator> range = _idToTags.equal_range(id);
    Assert(range.first != range.second);
    Assert(range.first != _idToTags.end());

    for (EntityToTags::const_iterator it = range.first; it != range.second; ++it) {
        Entities& bag = _tagToIDs.at(it->second);
        Remove_DontPreserveOrder(bag, id);
    }

    _idToTags.erase(range.first, range.second);
}
//----------------------------------------------------------------------------
size_t FEntityTagContainer::EntityTags(FEntityTag *pTags, size_t capacity, EntityID id) const {
    Assert(FEntity::InvalidID != id);

    return FillMatchingValues_ReturnCount(pTags, capacity, _idToTags, id);
}
//----------------------------------------------------------------------------
void FEntityTagContainer::EntityTags(VECTOR_THREAD_LOCAL(FEntity, FEntityTag)& outTags, EntityID id) const {
    Assert(FEntity::InvalidID != id);

    const TPair<EntityToTags::const_iterator, EntityToTags::const_iterator> range = _idToTags.equal_range(id);
    Assert(range.first != range.second);
    Assert(range.first != _idToTags.end());

    outTags.reserve(outTags.size() + std::distance(range.first, range.second));

    for (EntityToTags::const_iterator it = range.first; it != range.second; ++it)
        outTags.push_back(it->second);
}
//----------------------------------------------------------------------------
const VECTOR(FEntity, EntityID)& FEntityTagContainer::TagEntities(const FEntityTag& tag) const {
    Assert(!tag.empty());

    return _tagToIDs.at(tag);
}
//----------------------------------------------------------------------------
void FEntityTagContainer::Clear() {

    _idToTags.clear();
    _tagToIDs.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
