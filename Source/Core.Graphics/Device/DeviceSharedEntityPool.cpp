#include "stdafx.h"

#include "DeviceSharedEntityPool.h"

#include "DeviceAPIDependantEntity.h"
#include "DeviceSharedEntityKey.h"
#include "DeviceResourceSharable.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceSharedEntityPool::DeviceSharedEntityPool() {}
//----------------------------------------------------------------------------
DeviceSharedEntityPool::~DeviceSharedEntityPool() {
    Assert(_entities.empty());
}
//----------------------------------------------------------------------------
bool DeviceSharedEntityPool::Acquire(PDeviceAPIDependantEntity *pEntity, const DeviceResourceSharable& resource) {
    Assert(resource.Frozen());
    Assert(resource.Sharable());

    const auto range = _entities.equal_range( resource.SharedKey() );

    for (auto it = range.first; it != range.second; ++it)
        if (resource.MatchTerminalEntity(it->second.get()) ) {
            Assert(it->second->IsAttachedToResource() == false);
            
            *pEntity = std::move(it->second);
            _entities.erase(it);

            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
void DeviceSharedEntityPool::Release(const DeviceSharedEntityKey& key, PDeviceAPIDependantEntity& entity) {;
    Assert(entity->IsAttachedToResource() == false);

    _entities.emplace(key, std::move(entity));

    Assert(entity.get() == nullptr);
}
//----------------------------------------------------------------------------
void DeviceSharedEntityPool::Clear() {

#ifdef WITH_CORE_ASSERT
    for (Pair<const DeviceSharedEntityKey, PDeviceAPIDependantEntity>& it : _entities) {
        RemoveRef_AssertReachZero(it.second);
    }
#endif

    _entities.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
