// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DeviceSharedEntityPool.h"

#include "DeviceResourceSharable.h"

#include "Device/DeviceAPIDependantEntity.h"

#include "Allocator/PoolAllocator-impl.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDeviceSharedEntityPool::FSharedEntity, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceSharedEntityPool::FDeviceSharedEntityPool(FMemoryTracking *globalVideoMemory)
:   _mru(nullptr)
,   _lru(nullptr)
,   _map(128)
,   _usedMemory("EntityPool", globalVideoMemory) {}
//----------------------------------------------------------------------------
FDeviceSharedEntityPool::~FDeviceSharedEntityPool() {
    Assert(nullptr == _mru);
    Assert(nullptr == _lru);
    Assert(_map.empty());
}
//----------------------------------------------------------------------------
bool FDeviceSharedEntityPool::Acquire_Cooperative(PCDeviceAPIDependantEntity *pEntity, const FDeviceResourceSharable& resource) {
    Assert(resource.Frozen());
    Assert(resource.Sharable());

    const FDeviceSharedEntityKey key = resource.SharedKey();
    const map_type::iterator it = _map.find(key);
    if (_map.end() == it)
        return false;

    FSharedEntity*& shared = it->second;
    Assert(shared);

    FSharedEntity* p = shared;
    while (p && false == resource.MatchTerminalEntity(p->Entity.get()) ) {
        Assert(p->Key == key);
        p = p->Local.Next;
    }

    if (nullptr == p)
        return false;

    p->LockCount++;

    global_lru_type::PokeFront(&_mru, &_lru, p);

    Assert(key == p->Key);
    *pEntity = p->Entity;
    return true;
}
//----------------------------------------------------------------------------
void FDeviceSharedEntityPool::Release_Cooperative(const FDeviceSharedEntityKey& key, PCDeviceAPIDependantEntity& entity) {;
    Assert(entity->IsAttachedToResource() == false);

    FSharedEntity *& shared = _map[key];
    Assert(shared);

    FSharedEntity* p = shared;
    while (p && p->Entity != entity)
        p = p->Local.Next;

    if (p) {
        Assert(entity == p->Entity);
        Assert(p->LockCount);
        p->LockCount--;
    }
    else {
        AssertNotReached();
    }

    global_lru_type::PokeFront(&_mru, &_lru, p);

    entity.reset();
    Assert(entity.get() == nullptr);
}
//----------------------------------------------------------------------------
bool FDeviceSharedEntityPool::Acquire_Exclusive(PDeviceAPIDependantEntity *pEntity, const FDeviceResourceSharable& resource) {
    Assert(resource.Frozen());
    Assert(resource.Sharable());

    const FDeviceSharedEntityKey key = resource.SharedKey();
    const map_type::iterator it = _map.find(key);
    if (_map.end() == it)
        return false;

    FSharedEntity* p = it->second;
    Assert(p);

    while (p && false == (0 == p->LockCount && resource.MatchTerminalEntity(p->Entity.get())) ) {
        Assert(p->Key == key);
        p = p->Local.Next;
    }

    if (nullptr == p)
        return false;

    const TUniquePtr<FSharedEntity> shared(p);
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
void FDeviceSharedEntityPool::Release_Exclusive(const FDeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity) {;
    Assert(entity->IsAttachedToResource() == false);

    FSharedEntity *& shared = _map[key];
    FSharedEntity *const mru = new FSharedEntity;
    mru->LockCount = 0;
    mru->Key = key;
    mru->Entity = std::move(entity);
    Assert(entity.get() == nullptr);

    global_lru_type::PushFront(&_mru, &_lru, mru);
    local_lru_type::PushFront(&shared, nullptr, mru);

    const size_t sizeInBytes = shared->Entity->VideoMemorySizeInBytes();
    Assert(sizeInBytes);
    _usedMemory.Allocate(1, sizeInBytes);
}
//----------------------------------------------------------------------------
size_t FDeviceSharedEntityPool::ReleaseLRU_ReturnRealSize(size_t targetSizeInBytes) {

#ifdef WITH_PPE_ASSERT
    u64 totalSizeInBytes = 0;
#endif

    FSharedEntity* p = _lru;
    while (p && _usedMemory.TotalSizeInBytes() > targetSizeInBytes) {
        FSharedEntity *const prev = p->Global.Prev;

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
#ifdef WITH_PPE_ASSERT
        else {
            totalSizeInBytes += p->Entity->VideoMemorySizeInBytes();
        }
#endif

        p = prev;
    }

    Assert_NoAssume(_usedMemory.TotalSizeInBytes() == totalSizeInBytes);
    return checked_cast<size_t>(_usedMemory.TotalSizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
