#include "stdafx.h"

#include "DeviceResourceSharable.h"

#include "DeviceSharedEntityKey.h"

#include "Device/DeviceAPIDependantEntity.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceResourceSharable::FDeviceResourceSharable(EDeviceResourceType resourceType, bool sharable)
:   FDeviceResource(resourceType) {
    FDeviceResource::SetSharable_(sharable);
    _sharedKey = FDeviceSharedEntityKey::Invalid();
}
//----------------------------------------------------------------------------
const FDeviceSharedEntityKey& FDeviceResourceSharable::SharedKey() const {
    Assert(Frozen());
    Assert(Sharable());
    Assert(_sharedKey != FDeviceSharedEntityKey::Invalid());

    return _sharedKey;
}
//----------------------------------------------------------------------------
bool FDeviceResourceSharable::MatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    Assert(Frozen());
    Assert(!Available());
    Assert(Sharable());
    Assert(entity);
    Assert(!entity->IsAttachedToResource());

    return (entity->ResourceType() == ResourceType())
        ? VirtualMatchTerminalEntity(entity)
        : false;
}
//----------------------------------------------------------------------------
void FDeviceResourceSharable::FreezeImpl() {
    FDeviceResource::FreezeImpl();

    if (Sharable_())
    {
        const size_t hashValue = VirtualSharedKeyHashValue();
        _sharedKey = FDeviceSharedEntityKey::Make(ResourceType(), hashValue);
    }
}
//----------------------------------------------------------------------------
void FDeviceResourceSharable::UnfreezeImpl() {
    FDeviceResource::UnfreezeImpl();

    if (Sharable_())
    {
        _sharedKey = FDeviceSharedEntityKey::Invalid();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
