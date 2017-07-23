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
FWStringView FDeviceResource::ResourceName() const {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    THIS_THREADRESOURCE_CHECKACCESS();
    return MakeStringView(_resourceName);
#else
    return FStringView();
#endif
}
//----------------------------------------------------------------------------
void FDeviceResource::SetResourceName(const wchar_t* name) {
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
void FDeviceResource::SetResourceName(const FWStringView& name) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    Assert(!Frozen());
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(not name.empty());
    _resourceName = ToWString(name);
#else
    UNUSED(name);
#endif
}
//----------------------------------------------------------------------------
void FDeviceResource::SetResourceName(FWString&& name) {
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
