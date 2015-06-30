#include "stdafx.h"

#include "EntityContainer.h"

#include "Entity.h"

#include "Core/Meta/Guid.h"

#include <algorithm>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EntityContainer::EntityContainer() : _nextAvailableID(0) {
    // memory warmup to preserve over resizing in the first steps :
    STATIC_CONST_INTEGRAL(size_t, WarmupCapacity, 128);
    _entities.reserve(WarmupCapacity);
    _usedIDs.reserve(WarmupCapacity);
    _freeIDs.reserve(WarmupCapacity);
    _uidToId.reserve(WarmupCapacity);
}
//----------------------------------------------------------------------------
EntityContainer::~EntityContainer() {}
//----------------------------------------------------------------------------
EntityID EntityContainer::CreateEntity(const EntityUID *optionalUID/* = nullptr */) {
    if (_freeIDs.empty()) {
        const EntityID oldAvailableID = _nextAvailableID;
        _nextAvailableID = 1 + _nextAvailableID * 2;
        AssertRelease(oldAvailableID < _nextAvailableID);

        _entities.resize(_nextAvailableID);
        _freeIDs.reserve(_freeIDs.size() + _nextAvailableID - oldAvailableID);

        reverseforrange(id, u32(oldAvailableID), u32(_nextAvailableID))
            _freeIDs.push_back(id);

        _usedIDs.reserve(_freeIDs.capacity());
        _uidToId.reserve(_usedIDs.capacity());
    }

    Assert(!_freeIDs.empty());
    EntityID const id = _freeIDs.back();
    _freeIDs.pop_back();

    EntityUID const uid = optionalUID 
        ? *optionalUID 
        : Guid::Generate().ToUID();
    Insert_AssertUnique(_uidToId, uid, id);

    _entities[id] = Entity(id, uid);

    _usedIDs.push_back(id);

    return id;
}
//----------------------------------------------------------------------------
void EntityContainer::DestroyEntity(EntityID id) {
    Assert(Entity::InvalidID != id);

    Remove_AssertExists(_usedIDs, id);

    Entity& entity = _entities[id];
    Assert(entity.ID() == id);
    Remove_AssertExistsAndSameValue(_uidToId, entity.UID(), id);

    entity = Entity();

    _freeIDs.push_back(id);
}
//----------------------------------------------------------------------------
bool EntityContainer::Contains(EntityID id) const {
    Assert(Entity::InvalidID != id);

    return Core::Contains(_usedIDs, id);
}
//----------------------------------------------------------------------------
EntityID EntityContainer::IDFromUID(EntityUID uid) const {
    Assert(Entity::InvalidUID != uid);

    EntityID id;
    if (!TryGetValue(_uidToId, uid, &id)) {
        AssertNotReached();
        return Entity::InvalidID;
    }

    Assert(Entity::InvalidID != id);
    return id;
}
//----------------------------------------------------------------------------
void EntityContainer::Clear() {
    _entities.clear();
    _entities.shrink_to_fit();
    _freeIDs.clear();
    _freeIDs.shrink_to_fit();
    _usedIDs.clear();
    _usedIDs.shrink_to_fit();
    _uidToId.clear();
    _nextAvailableID = 0;
}
//----------------------------------------------------------------------------
void EntityContainer::ShrinkToFit() {
    const auto max_it = std::max_element(_usedIDs.begin(), _usedIDs.end());
    Assert(_usedIDs.end() != max_it);

    const EntityID shrinkAvailableID = *max_it + 1;

    if (shrinkAvailableID >= _nextAvailableID) {
        Assert(shrinkAvailableID == _nextAvailableID);
        return;
    }

    Assert(_nextAvailableID - shrinkAvailableID <= _freeIDs.size());
    std::remove_if(_freeIDs.begin(), _freeIDs.end(), [=](EntityID id) {
        return id >= shrinkAvailableID;
    });

    _entities.resize(shrinkAvailableID);

    _usedIDs.shrink_to_fit();
    _freeIDs.shrink_to_fit();

    _nextAvailableID = shrinkAvailableID;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
