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
    OnDeviceCreateImpl(device);
}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceReset(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    OnDeviceResetImpl(device);
}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceLost(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    OnDeviceLostImpl(device);
}
//----------------------------------------------------------------------------
void DeviceResource::OnDeviceDestroy(DeviceEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    OnDeviceDestroyImpl(device);
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
const char *ResourceTypeToCStr(DeviceResourceType type) {
    switch (type)
    {
    case Core::Graphics::DeviceResourceType::Constants:
        return "Constants";
    case Core::Graphics::DeviceResourceType::Indices:
        return "Indices";
    case Core::Graphics::DeviceResourceType::RenderTarget:
        return "RenderTarget";
    case Core::Graphics::DeviceResourceType::ShaderEffect:
        return "ShaderEffect";
    case Core::Graphics::DeviceResourceType::ShaderProgram:
        return "ShaderProgram";
    case Core::Graphics::DeviceResourceType::State:
        return "State";
    case Core::Graphics::DeviceResourceType::Texture:
        return "Texture";
    case Core::Graphics::DeviceResourceType::VertexDeclaration:
        return "VertexDeclaration";
    case Core::Graphics::DeviceResourceType::Vertices:
        return "Vertices";
    }

    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
