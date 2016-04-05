#include "stdafx.h"

#include "DeviceAPIDependantEntity.h"

#include "AbstractDeviceAPIEncapsulator.h"
#include "DeviceAPI.h"
#include "DeviceResource.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantEntity::DeviceAPIDependantEntity(const AbstractDeviceAPIEncapsulator *encapsulator, const DeviceResource *resource)
:   _apiAndResourceType(0)
,   _resource(resource)
,   _createdAt(InvalidDeviceRevision())
,   _lastUsed(InvalidDeviceRevision()) {
    Assert(_resource);
    bitdevicapi_type::InplaceSet(_apiAndResourceType, (u32)encapsulator->API() );
    bitresourcetype_type::InplaceSet(_apiAndResourceType, (u32)_resource->ResourceType() );
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity::DeviceAPIDependantEntity(const AbstractDeviceAPIEncapsulator *encapsulator, DeviceResourceType resourceType)
:   _apiAndResourceType(0)
,   _resource(nullptr)
,   _createdAt(InvalidDeviceRevision())
,   _lastUsed(InvalidDeviceRevision()) {
    bitdevicapi_type::InplaceSet(_apiAndResourceType, (u32)encapsulator->API() );
    bitresourcetype_type::InplaceSet(_apiAndResourceType, (u32)resourceType );
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity::~DeviceAPIDependantEntity() {}
//----------------------------------------------------------------------------
bool DeviceAPIDependantEntity::MatchDevice(const IDeviceAPIEncapsulator *device) const {
    Assert(device);
    return device->APIEncapsulator()->API() == API();
}
//----------------------------------------------------------------------------
void DeviceAPIDependantEntity::AttachResource(const DeviceResource *resource) {
    Assert(nullptr != resource);
    Assert(resource->Frozen());
    Assert(!resource->Available());
    Assert( nullptr == _resource ||
            resource == _resource );
    Assert(!_resource->Available());
    Assert(ResourceType() == resource->ResourceType());

    _resource.reset(resource);
}
//----------------------------------------------------------------------------
void DeviceAPIDependantEntity::DetachResource(const DeviceResource *resource) {
    UNUSED(resource);
    Assert(nullptr != resource);
    Assert(resource == _resource);
    Assert(_resource->Frozen());
    Assert(_resource->Available());

    _resource.reset(nullptr);
}
//----------------------------------------------------------------------------
void DeviceAPIDependantEntity::SetCreatedAt(DeviceRevision revision) {
    Assert(_resource);
    Assert(revision != InvalidDeviceRevision());

    _createdAt = revision;
}
//----------------------------------------------------------------------------
void DeviceAPIDependantEntity::SetLastUsed(DeviceRevision revision) {
    Assert(_resource);
    Assert(revision != InvalidDeviceRevision());

    _lastUsed = revision;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
