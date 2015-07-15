#include "stdafx.h"

#include "DeviceResourceSharable.h"

#include "DeviceAPIDependantEntity.h"
#include "DeviceSharedEntityKey.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceResourceSharable::DeviceResourceSharable(DeviceResourceType resourceType, bool sharable)
:   DeviceResource(resourceType) 
,   _sharable(sharable)
{}
//----------------------------------------------------------------------------
bool DeviceResourceSharable::MatchTerminalEntity(const DeviceAPIDependantEntity *entity) const {
    Assert(Frozen());
    Assert(!Available());
    Assert(_sharable);
    Assert(!entity->IsAttachedToResource());

    return (entity->ResourceType() == ResourceType())
        ? VirtualMatchTerminalEntity(entity)
        : false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
