#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Pool/DeviceSharedEntityKey.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/List.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/MemoryTracking.h"

namespace Core {
namespace Graphics {
class DeviceResourceSharable;
FWD_REFPTR(DeviceAPIDependantEntity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceSharedEntityPool {
public:
    struct SharedEntity;
    struct LRUNode {
        SharedEntity *Next;
        SharedEntity *Prev;
    };

    struct SharedEntity {
        DeviceSharedEntityKey Key;
        PDeviceAPIDependantEntity Entity;
        LRUNode Global;
        LRUNode Local;
        SINGLETON_POOL_ALLOCATED_DECL(SharedEntity);
    };

    explicit DeviceSharedEntityPool(MemoryTrackingData *globalVideoMemory);
    ~DeviceSharedEntityPool();

    const MemoryTrackingData& UsedMemory() const { return _usedMemory; }

    bool Acquire(PDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource);
    void Release(const DeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity);

    void ReleaseLRU(size_t targetSizeInBytes);
    void ReleaseAll();

private:
    typedef HASHMAP(Graphics, DeviceSharedEntityKey, SharedEntity *) map_type;

    SharedEntity *_mru;
    SharedEntity *_lru;

    map_type _map;
    MemoryTrackingData _usedMemory;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
