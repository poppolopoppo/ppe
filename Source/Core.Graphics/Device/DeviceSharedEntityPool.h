#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Container/MultiMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class DeviceResourceSharable;
struct DeviceSharedDataKey;
struct DeviceSharedEntityKey;
FWD_REFPTR(DeviceAPIDependantEntity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceEntityPool);
class DeviceSharedEntityPool {
public:
    DeviceSharedEntityPool();
    ~DeviceSharedEntityPool();

    size_t EntityCount() const;

    bool Acquire(PDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable *resource);
    void Release(const DeviceSharedEntityKey& key, DeviceAPIDependantEntity *entity);

    void Clear();

private:
    MULTIMAP(Graphics, DeviceSharedEntityKey, PDeviceAPIDependantEntity) _entities;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
