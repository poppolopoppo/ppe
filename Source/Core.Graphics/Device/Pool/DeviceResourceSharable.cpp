#include "stdafx.h"

#include "DeviceResourceSharable.h"

#include "DeviceSharedEntityKey.h"

#include "Device/DeviceAPIDependantEntity.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceResourceSharable::DeviceResourceSharable(DeviceResourceType resourceType, bool sharable)
:   DeviceResource(resourceType) {
    DeviceResource::SetSharable_(sharable);
    _sharedKey = DeviceSharedEntityKey::Invalid();
}
//----------------------------------------------------------------------------
const DeviceSharedEntityKey& DeviceResourceSharable::SharedKey() const {
    Assert(Frozen());
    Assert(Sharable());
    Assert(_sharedKey != DeviceSharedEntityKey::Invalid());

    return _sharedKey;
}
//----------------------------------------------------------------------------
bool DeviceResourceSharable::MatchTerminalEntity(const DeviceAPIDependantEntity *entity) const {
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
void DeviceResourceSharable::FreezeImpl() {
    DeviceResource::FreezeImpl();

    if (Sharable_())
    {
        const size_t hashValue = VirtualSharedKeyHashValue();
        _sharedKey = DeviceSharedEntityKey::Make(ResourceType(), hashValue);
    }
}
//----------------------------------------------------------------------------
void DeviceResourceSharable::UnfreezeImpl() {
    DeviceResource::UnfreezeImpl();

    if (Sharable_())
    {
        _sharedKey = DeviceSharedEntityKey::Invalid();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
