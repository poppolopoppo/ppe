// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DeviceAPIDependantEntity.h"

#include "AbstractDeviceAPIEncapsulator.h"
#include "DeviceAPI.h"
#include "DeviceResource.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity::FDeviceAPIDependantEntity(const FAbstractDeviceAPIEncapsulator *encapsulator, const FDeviceResource *resource)
:   _apiAndResourceType(0)
,   _resource(resource)
,   _createdAt(InvalidDeviceRevision())
,   _lastUsed(InvalidDeviceRevision()) {
    Assert(_resource);
    bitdevicapi_type::InplaceSet(_apiAndResourceType, (u32)encapsulator->API() );
    bitresourcetype_type::InplaceSet(_apiAndResourceType, (u32)_resource->ResourceType() );
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity::FDeviceAPIDependantEntity(const FAbstractDeviceAPIEncapsulator *encapsulator, EDeviceResourceType resourceType)
:   _apiAndResourceType(0)
,   _resource(nullptr)
,   _createdAt(InvalidDeviceRevision())
,   _lastUsed(InvalidDeviceRevision()) {
    bitdevicapi_type::InplaceSet(_apiAndResourceType, (u32)encapsulator->API() );
    bitresourcetype_type::InplaceSet(_apiAndResourceType, (u32)resourceType );
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity::~FDeviceAPIDependantEntity() {}
//----------------------------------------------------------------------------
bool FDeviceAPIDependantEntity::MatchDevice(const IDeviceAPIEncapsulator *device) const {
    Assert(device);
    return device->APIEncapsulator()->API() == API();
}
//----------------------------------------------------------------------------
void FDeviceAPIDependantEntity::AttachResource(const FDeviceResource *resource) {
    Assert(nullptr != resource);
    Assert(resource->Frozen());
    Assert(!resource->Available());
    Assert( nullptr == _resource ||
            resource == _resource );
    Assert(!_resource || !_resource->Available());
    Assert(ResourceType() == resource->ResourceType());

    _resource.reset(resource);
}
//----------------------------------------------------------------------------
void FDeviceAPIDependantEntity::DetachResource(const FDeviceResource *resource) {
    UNUSED(resource);
    Assert(nullptr != resource);
    Assert(resource == _resource);
    Assert(_resource->Frozen());
    Assert(_resource->Available());

    _resource.reset(nullptr);
}
//----------------------------------------------------------------------------
void FDeviceAPIDependantEntity::SetCreatedAt(FDeviceRevision revision) {
    Assert(_resource);
    Assert(revision != InvalidDeviceRevision());

    _createdAt = revision;
}
//----------------------------------------------------------------------------
void FDeviceAPIDependantEntity::SetLastUsed(FDeviceRevision revision) {
    Assert(_resource);
    Assert(revision != InvalidDeviceRevision());

    _lastUsed = revision;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
