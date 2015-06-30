#include "stdafx.h"

#include "EntityTagContainer.h"

#include "Entity.h"
#include "EntityTag.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EntityTagContainer::EntityTagContainer() {
    _idToTags.reserve(128);
    _tagToIDs.reserve(32);
}
//----------------------------------------------------------------------------
EntityTagContainer::~EntityTagContainer() {}
//----------------------------------------------------------------------------
void EntityTagContainer::AddTag(EntityID id, const EntityTag& tag) {
    Assert(Entity::InvalidID != id);
    Assert(!tag.empty());

    _idToTags.emplace(id, tag);

    Entities& bag = _tagToIDs[tag];
    bag.push_back(id);
}
//----------------------------------------------------------------------------
void EntityTagContainer::RemoveTag(EntityID id, const EntityTag& tag) {
    Assert(Entity::InvalidID != id);
    Assert(!tag.empty());

    RemoveKeyValue_AssertExists(_idToTags, id, tag);
    
    Entities& bag = _tagToIDs.at(tag);
    Remove_DontPreserveOrder(bag, id);
}
//----------------------------------------------------------------------------
bool EntityTagContainer::HasTag(EntityID id, const EntityTag& tag) const {
    Assert(Entity::InvalidID != id);
    Assert(!tag.empty());

    const Pair<EntityToTags::const_iterator, EntityToTags::const_iterator> range = _idToTags.equal_range(id);
    for (EntityToTags::const_iterator it = range.first; it != range.second; ++it)
        if (it->second == tag)
            return true;

    return false;
}
//----------------------------------------------------------------------------
void EntityTagContainer::RemoveEntity(EntityID id) {
    Assert(Entity::InvalidID != id);

    const Pair<EntityToTags::const_iterator, EntityToTags::const_iterator> range = _idToTags.equal_range(id);
    Assert(range.first != range.second);
    Assert(range.first != _idToTags.end());

    for (EntityToTags::const_iterator it = range.first; it != range.second; ++it) {
        Entities& bag = _tagToIDs.at(it->second);
        Remove_DontPreserveOrder(bag, id);
    }

    _idToTags.erase(range.first, range.second);
}
//----------------------------------------------------------------------------
size_t EntityTagContainer::EntityTags(EntityTag *pTags, size_t capacity, EntityID id) const {
    Assert(Entity::InvalidID != id);

    return FillMatchingValues_ReturnCount(pTags, capacity, _idToTags, id);
}
//----------------------------------------------------------------------------
void EntityTagContainer::EntityTags(VECTOR_THREAD_LOCAL(Entity, EntityTag)& outTags, EntityID id) const {
    Assert(Entity::InvalidID != id);

    const Pair<EntityToTags::const_iterator, EntityToTags::const_iterator> range = _idToTags.equal_range(id);
    Assert(range.first != range.second);
    Assert(range.first != _idToTags.end());

    outTags.reserve(outTags.size() + std::distance(range.first, range.second));

    for (EntityToTags::const_iterator it = range.first; it != range.second; ++it)
        outTags.push_back(it->second);
}
//----------------------------------------------------------------------------
const VECTOR(Entity, EntityID)& EntityTagContainer::TagEntities(const EntityTag& tag) const {
    Assert(!tag.empty());

    return _tagToIDs.at(tag);
}
//----------------------------------------------------------------------------
void EntityTagContainer::Clear() {

    _idToTags.clear();
    _tagToIDs.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
