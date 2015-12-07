#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Pool/DeviceSharedEntityKey.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/IntrusiveList.h"
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
    struct SharedEntity {
        DeviceSharedEntityKey Key;
        PDeviceAPIDependantEntity Entity;
        IntrusiveListNode<SharedEntity> Global;
        IntrusiveListNode<SharedEntity> Local;
        u32 LockCount;
        SINGLETON_POOL_ALLOCATED_DECL();
    };

    typedef INTRUSIVELIST(&SharedEntity::Global) global_lru_type;
    typedef INTRUSIVELIST(&SharedEntity::Local)  local_lru_type;

    explicit DeviceSharedEntityPool(MemoryTrackingData *globalVideoMemory);
    ~DeviceSharedEntityPool();

    const MemoryTrackingData& UsedMemory() const { return _usedMemory; }

    bool Acquire_Cooperative(PCDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource);
    void Release_Cooperative(const DeviceSharedEntityKey& key, PCDeviceAPIDependantEntity& entity);

    bool Acquire_Exclusive(PDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource);
    void Release_Exclusive(const DeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity);

    size_t ReleaseLRU_ReturnRealSize(size_t targetSizeInBytes);
    size_t ReleaseAll_ReturnRealSize() { return ReleaseLRU_ReturnRealSize(0); }

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
