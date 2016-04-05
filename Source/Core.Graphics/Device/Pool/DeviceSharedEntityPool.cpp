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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DeviceSharedEntityPool::SharedEntity, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceSharedEntityPool::DeviceSharedEntityPool(MemoryTrackingData *globalVideoMemory)
:   _mru(nullptr)
,   _lru(nullptr)
,   _map(128)
,   _usedMemory("EntityPool", globalVideoMemory) {}
//----------------------------------------------------------------------------
DeviceSharedEntityPool::~DeviceSharedEntityPool() {
    Assert(nullptr == _mru);
    Assert(nullptr == _lru);
    Assert(_map.empty());
}
//----------------------------------------------------------------------------
bool DeviceSharedEntityPool::Acquire_Cooperative(PCDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource) {
    Assert(resource.Frozen());
    Assert(resource.Sharable());

    const DeviceSharedEntityKey key = resource.SharedKey();
    const map_type::iterator it = _map.find(key);
    if (_map.end() == it)
        return false;

    SharedEntity*& shared = it->second;
    Assert(shared);

    SharedEntity* p = shared;
    while (p && false == resource.MatchTerminalEntity(p->Entity.get()) ) {
        Assert(p->Key == key);
        p = p->Local.Next;
    }

    if (nullptr == p)
        return false;

    p->LockCount++;

    global_lru_type::Poke(&_mru, &_lru, p);

    Assert(key == p->Key);
    *pEntity = p->Entity;
    return true;
}
//----------------------------------------------------------------------------
void DeviceSharedEntityPool::Release_Cooperative(const DeviceSharedEntityKey& key, PCDeviceAPIDependantEntity& entity) {;
    Assert(entity->IsAttachedToResource() == false);

    SharedEntity *& shared = _map[key];
    Assert(shared);

    SharedEntity* p = shared;
    while (p && p->Entity != entity)
        p = p->Local.Next;

    AssertRelease(p);
    Assert(entity == p->Entity);
    Assert(p->LockCount);
    p->LockCount--;

    global_lru_type::Poke(&_mru, &_lru, p);

    entity.reset();
    Assert(entity.get() == nullptr);
}
//----------------------------------------------------------------------------
bool DeviceSharedEntityPool::Acquire_Exclusive(PDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource) {
    Assert(resource.Frozen());
    Assert(resource.Sharable());

    const DeviceSharedEntityKey key = resource.SharedKey();
    const map_type::iterator it = _map.find(key);
    if (_map.end() == it)
        return false;

    SharedEntity *p = it->second;
    Assert(p);

    while (p && false == (0 == p->LockCount && resource.MatchTerminalEntity(p->Entity.get())) ) {
        Assert(p->Key == key);
        p = p->Local.Next;
    }

    if (nullptr == p)
        return false;

    const UniquePtr<SharedEntity> shared(p);
    Assert(shared->Entity);
    Assert(0 == shared->LockCount);
    Assert(nullptr == shared->Local.Prev);

    global_lru_type::Erase(&_mru, &_lru, p);
    local_lru_type::Erase(&it->second, nullptr, p);

    if (nullptr == it->second)
        _map.erase(it);

    const size_t sizeInBytes = shared->Entity->VideoMemorySizeInBytes();
    Assert(sizeInBytes);
    _usedMemory.Deallocate(1, sizeInBytes);

    Assert(key == shared->Key);
    *pEntity = std::move(shared->Entity);
    return true;
}
//----------------------------------------------------------------------------
void DeviceSharedEntityPool::Release_Exclusive(const DeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity) {;
    Assert(entity->IsAttachedToResource() == false);

    SharedEntity *& shared = _map[key];
    SharedEntity *const mru = new SharedEntity;
    mru->LockCount = 0;

    global_lru_type::PushFront(&_mru, &_lru, mru);
    local_lru_type::PushFront(&shared, nullptr, mru);

    const size_t sizeInBytes = shared->Entity->VideoMemorySizeInBytes();
    Assert(sizeInBytes);
    _usedMemory.Allocate(1, sizeInBytes);

    mru->Key = key;
    mru->Entity = std::move(entity);
    Assert(entity.get() == nullptr);
}
//----------------------------------------------------------------------------
size_t DeviceSharedEntityPool::ReleaseLRU_ReturnRealSize(size_t targetSizeInBytes) {

#ifdef WITH_CORE_ASSERT
    u64 totalSizeInBytes = 0;
#endif

    SharedEntity* p = _lru;
    while (p && _usedMemory.TotalSizeInBytes() > targetSizeInBytes) {
        SharedEntity *const prev = p->Global.Prev;

        if (0 == p->LockCount) {

            const map_type::iterator it = _map.find(p->Key);
            Assert(_map.end() != it);

            global_lru_type::Erase(&_mru, &_lru, p);
            local_lru_type::Erase(&it->second, nullptr, p);

            if (nullptr == it->second)
                _map.erase(it);

            _usedMemory.Deallocate(1, p->Entity->VideoMemorySizeInBytes());

            RemoveRef_AssertReachZero(p->Entity);
            checked_delete(p);
        }
#ifdef WITH_CORE_ASSERT
        else {
            totalSizeInBytes += p->Entity->VideoMemorySizeInBytes();
        }
#endif

        p = prev;
    }

    Assert(_usedMemory.TotalSizeInBytes() == totalSizeInBytes);
    return checked_cast<size_t>(_usedMemory.TotalSizeInBytes().Value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
