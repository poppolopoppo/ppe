#include "stdafx.h"

#include "DeviceResource.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceResource::FDeviceResource(EDeviceResourceType resourceType)
:   _flagsAndResourceType(0) {
    bitfrozen_type::InplaceFalse(_flagsAndResourceType);
    bitresourcetype_type::InplaceSet(_flagsAndResourceType, static_cast<u32>(resourceType));
}
//----------------------------------------------------------------------------
FDeviceResource::~FDeviceResource() {}
//----------------------------------------------------------------------------
void FDeviceResource::OnDeviceCreate(FDeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void FDeviceResource::OnDeviceReset(FDeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceReset(device);
}
//----------------------------------------------------------------------------
void FDeviceResource::OnDeviceLost(FDeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceLost(device);
}
//----------------------------------------------------------------------------
void FDeviceResource::OnDeviceDestroy(FDeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    VirtualOnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
void FDeviceResource::Freeze() {
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    bitfrozen_type::InplaceTrue(_flagsAndResourceType);
    FreezeImpl();
}
//----------------------------------------------------------------------------
void FDeviceResource::Unfreeze() {
    Assert(Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    bitfrozen_type::InplaceFalse(_flagsAndResourceType);
    UnfreezeImpl();
}
//----------------------------------------------------------------------------
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
FWStringView FDeviceResource::ResourceName() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return MakeStringView(_resourceName);
}
#endif
//----------------------------------------------------------------------------
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
void FDeviceResource::SetResourceName(const wchar_t* name) {
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    _resourceName = name;
}
#endif
//----------------------------------------------------------------------------
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
void FDeviceResource::SetResourceName(const FWStringView& name) {
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not name.empty());
    _resourceName = ToWString(name);
}
#endif
//----------------------------------------------------------------------------
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
void FDeviceResource::SetResourceName(FWString&& name) {
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name.size());
    _resourceName = std::move(name);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
