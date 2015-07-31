#include "stdafx.h"

#include "DeviceSharedEntityPool.h"

#include "DeviceResourceSharable.h"

#include "Device/DeviceAPIDependantEntity.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Graphics, DeviceSharedEntityPool::SharedEntity, );
//----------------------------------------------------------------------------
template <DeviceSharedEntityPool::LRUNode DeviceSharedEntityPool::SharedEntity::*_Member>
static void LRUList_Push_(  DeviceSharedEntityPool::SharedEntity **pHead,
                            DeviceSharedEntityPool::SharedEntity **pTail,
                            DeviceSharedEntityPool::SharedEntity *entity) {
    Assert(pHead);
    Assert(entity);

    DeviceSharedEntityPool::LRUNode& node = entity->*_Member;
    node.Prev = nullptr;
    node.Next = *pHead;

    if (nullptr != *pHead) {
        DeviceSharedEntityPool::LRUNode& head = (*pHead)->*_Member;
        Assert(nullptr == head.Prev);
        head.Prev = entity;
    }
    else if (pTail) {
        Assert(nullptr == *pTail);
        *pTail = entity;
    }

    *pHead = entity;
}
//----------------------------------------------------------------------------
template <DeviceSharedEntityPool::LRUNode DeviceSharedEntityPool::SharedEntity::*_Member>
static void LRUList_Remove_(DeviceSharedEntityPool::SharedEntity **pHead,
                            DeviceSharedEntityPool::SharedEntity **pTail,
                            DeviceSharedEntityPool::SharedEntity *entity) {
    Assert(pHead);
    Assert(entity);
    Assert(*pHead);
    Assert(nullptr == pTail || *pTail);

    DeviceSharedEntityPool::LRUNode& node = entity->*_Member;

    if (node.Prev) {
        Assert(node.Prev->*_Member.Next == entity);
        node.Prev->*_Member.Next = node.Next;
    }

    if (node.Next) {
        Assert(node.Next->*_Member.Prev == entity);
        node.Next->*_Member.Prev = node.Prev;
    }

    if (*pHead == entity) {
        Assert(nullptr == node.Prev);
        *pHead = node.Next;
        Assert(nullptr == *pHead || nullptr == (*pHead)->*_Member.Prev);
    }

    if (pTail && *pTail == entity) {
        Assert(nullptr == node.Next);
        *pTail = node.Prev;
        Assert(nullptr == *pTail || nullptr == (*pTail)->*_Member.Next);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceSharedEntityPool::DeviceSharedEntityPool(MemoryTrackingData *globalVideoMemory) 
:   _mru(nullptr)
,   _lru(nullptr)
,   _map(32)
,   _usedMemory("EntityPool", globalVideoMemory) {}
//----------------------------------------------------------------------------
DeviceSharedEntityPool::~DeviceSharedEntityPool() {
    Assert(nullptr == _mru);
    Assert(nullptr == _lru);
    Assert(_map.empty());
}
//----------------------------------------------------------------------------
bool DeviceSharedEntityPool::Acquire(PDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource) {
    Assert(resource.Frozen());
    Assert(resource.Sharable());

    const DeviceSharedEntityKey key = resource.SharedKey();
    const map_type::iterator it = _map.find(key);
    if (_map.end() == it)
        return false;

    SharedEntity *p = it->second;
    Assert(p);

    while (p && false == resource.MatchTerminalEntity(p->Entity.get()) ) {
        Assert(p->Key == key);
        p = p->Local.Next;
    }

    if (nullptr == p)
        return false;

    const UniquePtr<SharedEntity> shared(p);
    Assert(shared->Entity);
    Assert(nullptr == shared->Local.Prev);

    LRUList_Remove_<&SharedEntity::Global>(&_mru, &_lru, shared.get());
    LRUList_Remove_<&SharedEntity::Local>(&it->second, nullptr, shared.get());

    if (nullptr == it->second)
        _map.erase(it);

    _usedMemory.Deallocate(1, shared->Entity->VideoMemorySizeInBytes());

    Assert(key == shared->Key);
    *pEntity = std::move(shared->Entity);
    return true;
}
//----------------------------------------------------------------------------
void DeviceSharedEntityPool::Release(const DeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity) {;
    Assert(entity->IsAttachedToResource() == false);

    SharedEntity *& shared = _map[key];
    SharedEntity *const mru = new SharedEntity;

    LRUList_Push_<&SharedEntity::Global>(&_mru, &_lru, mru);
    LRUList_Push_<&SharedEntity::Global>(&shared, nullptr, mru);

    _usedMemory.Allocate(1, entity->VideoMemorySizeInBytes());

    mru->Key = key;
    mru->Entity = std::move(entity);
    Assert(entity.get() == nullptr);
}
//----------------------------------------------------------------------------
void DeviceSharedEntityPool::ReleaseAll() {

    SharedEntity *p = _mru;
    while (p) {
        SharedEntity *const next = p->Global.Next;
        Assert(nullptr != next || _lru == p);
        _usedMemory.Deallocate(1, p->Entity->VideoMemorySizeInBytes());
        RemoveRef_AssertReachZero(p->Entity);
        checked_delete(p);
        p = next;
    }

    _mru = _lru = nullptr;
    _map.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
