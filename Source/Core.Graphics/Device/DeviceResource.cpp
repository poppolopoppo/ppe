#include "stdafx.h"

#include "DeviceResource.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceResource::DeviceResource(DeviceResourceType resourceType)
:   _frozenAndResourceType(0) {
    bitfrozen_type::InplaceFalse(_frozenAndResourceType);
    bitresourcetype_type::InplaceSet(_frozenAndResourceType, static_cast<u32>(resourceType));
}
//----------------------------------------------------------------------------
DeviceResource::~DeviceResource() {}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceCreate(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceReset(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceReset(device);
}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceLost(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceLost(device);
}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceDestroy(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
void DeviceResource::Freeze() {
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    bitfrozen_type::InplaceTrue(_frozenAndResourceType);
    FreezeImpl();
}
//----------------------------------------------------------------------------
void DeviceResource::Unfreeze() {
    Assert(Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    bitfrozen_type::InplaceFalse(_frozenAndResourceType);
    UnfreezeImpl();
}
//----------------------------------------------------------------------------
const char *DeviceResource::ResourceName() const {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    THIS_THREADRESOURCE_CHECKACCESS();
    return _resourceName.c_str();
#else
    return nullptr;
#endif
}
//----------------------------------------------------------------------------
void DeviceResource::SetResourceName(const char *name) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    _resourceName = name;
#endif
}
//----------------------------------------------------------------------------
void DeviceResource::SetResourceName(String&& name) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name.size());
    _resourceName = std::move(name);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
