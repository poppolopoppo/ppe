#include "stdafx.h"

#include "DeviceResource.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceResource::DeviceResource(DeviceResourceType resourceType)
:   _flagsAndResourceType(0) {
    bitfrozen_type::InplaceFalse(_flagsAndResourceType);
    bitresourcetype_type::InplaceSet(_flagsAndResourceType, static_cast<u32>(resourceType));
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
    bitfrozen_type::InplaceTrue(_flagsAndResourceType);
    FreezeImpl();
}
//----------------------------------------------------------------------------
void DeviceResource::Unfreeze() {
    Assert(Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    bitfrozen_type::InplaceFalse(_flagsAndResourceType);
    UnfreezeImpl();
}
//----------------------------------------------------------------------------
StringView DeviceResource::ResourceName() const {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    THIS_THREADRESOURCE_CHECKACCESS();
    return MakeStringView(_resourceName);
#else
    return StringView();
#endif
}
//----------------------------------------------------------------------------
void DeviceResource::SetResourceName(const char *name) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    _resourceName = name;
#else
    UNUSED(name);
#endif
}
//----------------------------------------------------------------------------
void DeviceResource::SetResourceName(const StringView& name) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name.empty());
    _resourceName = ToString(name);
#else
    UNUSED(name);
#endif
}
//----------------------------------------------------------------------------
void DeviceResource::SetResourceName(String&& name) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name.size());
    _resourceName = std::move(name);
#else
    UNUSED(name);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
