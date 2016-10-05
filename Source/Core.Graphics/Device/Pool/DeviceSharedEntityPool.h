#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Pool/DeviceSharedEntityKey.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/IntrusiveList.h"
#include "Core/Memory/MemoryTracking.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class FDeviceResourceSharable;
FWD_REFPTR(DeviceAPIDependantEntity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceSharedEntityPool {
public:
    struct FSharedEntity {
        FDeviceSharedEntityKey Key;
        PDeviceAPIDependantEntity Entity;
        IntrusiveListNode<FSharedEntity> Global;
        IntrusiveListNode<FSharedEntity> Local;
        u32 LockCount;
        SINGLETON_POOL_ALLOCATED_DECL();
    };

    typedef INTRUSIVELIST_ACCESSOR(&FSharedEntity::Global) global_lru_type;
    typedef INTRUSIVELIST_ACCESSOR(&FSharedEntity::Local)  local_lru_type;

    explicit FDeviceSharedEntityPool(FMemoryTrackingData *globalVideoMemory);
    ~FDeviceSharedEntityPool();

    const FMemoryTrackingData& UsedMemory() const { return _usedMemory; }

    bool Acquire_Cooperative(PCDeviceAPIDependantEntity *pEntity, const FDeviceResourceSharable& resource);
    void Release_Cooperative(const FDeviceSharedEntityKey& key, PCDeviceAPIDependantEntity& entity);

    bool Acquire_Exclusive(PDeviceAPIDependantEntity *pEntity, const FDeviceResourceSharable& resource);
    void Release_Exclusive(const FDeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity);

    size_t ReleaseLRU_ReturnRealSize(size_t targetSizeInBytes);
    size_t ReleaseAll_ReturnRealSize() { return ReleaseLRU_ReturnRealSize(0); }

private:
    typedef HASHMAP(Graphics, FDeviceSharedEntityKey, FSharedEntity*) map_type;

    FSharedEntity *_mru;
    FSharedEntity *_lru;

    map_type _map;
    FMemoryTrackingData _usedMemory;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
