// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "EntityContainer.h"

#include "Entity.h"

#include "Misc/Guid.h"

#include <algorithm>

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEntityContainer::FEntityContainer() : _nextAvailableID(0) {
    // memory warmup to preserve over resizing in the first steps :
    STATIC_CONST_INTEGRAL(size_t, WarmupCapacity, 128);
    _entities.reserve(WarmupCapacity);
    _usedIDs.reserve(WarmupCapacity);
    _freeIDs.reserve(WarmupCapacity);
    _uidToId.reserve(WarmupCapacity);
}
//----------------------------------------------------------------------------
FEntityContainer::~FEntityContainer() {}
//----------------------------------------------------------------------------
EntityID FEntityContainer::CreateEntity(const EntityUID *optionalUID/* = nullptr */) {
    if (_freeIDs.empty()) {
        const EntityID oldAvailableID = _nextAvailableID;
        _nextAvailableID.Value = 1 + _nextAvailableID * 2;
        AssertRelease(oldAvailableID < _nextAvailableID);

        _entities.resize(_nextAvailableID);
        _freeIDs.reserve(_freeIDs.size() + _nextAvailableID - oldAvailableID);

        reverseforrange(id, u32(oldAvailableID), u32(_nextAvailableID))
            _freeIDs.push_back(EntityID(id));

        _usedIDs.reserve(_freeIDs.capacity());
        _uidToId.reserve(_usedIDs.capacity());
    }

    Assert(!_freeIDs.empty());
    const EntityID id = _freeIDs.back();
    _freeIDs.pop_back();

    const EntityUID uid(optionalUID ? *optionalUID : FGuid::Generate().ToUID() );
    Insert_AssertUnique(_uidToId, uid, id);

    _entities[id] = FEntity(id, uid);

    _usedIDs.push_back(id);

    return id;
}
//----------------------------------------------------------------------------
void FEntityContainer::DestroyEntity(EntityID id) {
    Assert(FEntity::InvalidID != id);

    Remove_AssertExists(_usedIDs, id);

    FEntity& entity = _entities[id];
    Assert(entity.ID() == id);
    Remove_AssertExistsAndSameValue(_uidToId, entity.UID(), id);

    entity = FEntity();

    _freeIDs.push_back(id);
}
//----------------------------------------------------------------------------
bool FEntityContainer::Contains(EntityID id) const {
    Assert(FEntity::InvalidID != id);

    return PPE::Contains(_usedIDs, id);
}
//----------------------------------------------------------------------------
EntityID FEntityContainer::IDFromUID(EntityUID uid) const {
    Assert(FEntity::InvalidUID != uid);

    EntityID id;
    if (!TryGetValue(_uidToId, uid, &id)) {
        AssertNotReached();
        return EntityID(FEntity::InvalidID);
    }

    Assert(FEntity::InvalidID != id);
    return id;
}
//----------------------------------------------------------------------------
void FEntityContainer::Clear() {
    clear_ReleaseMemory(_entities);
    clear_ReleaseMemory(_freeIDs);
    clear_ReleaseMemory(_usedIDs);
    _uidToId.clear();
    _nextAvailableID.Value = 0;
}
//----------------------------------------------------------------------------
void FEntityContainer::ShrinkToFit() {
    const auto max_it = std::max_element(_usedIDs.begin(), _usedIDs.end());
    Assert(_usedIDs.end() != max_it);

    const EntityID shrinkAvailableID(*max_it + 1);

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
} //!namespace PPE
