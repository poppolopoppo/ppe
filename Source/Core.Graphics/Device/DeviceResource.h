#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/BitField.h"
#include "Core/Meta/ThreadResource.h"

#define WITH_GRAPHICS_DEVICERESOURCE_NAME

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
#   include "Core/IO/String.h"
#endif

namespace Core {
namespace Graphics {
class DeviceEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class DeviceResourceType {
    Constants = 0,
    Indices,
    RenderTarget,
    ShaderEffect,
    ShaderProgram,
    State,
    Texture,
    VertexDeclaration,
    Vertices,
};
//----------------------------------------------------------------------------
const char *ResourceTypeToCStr(DeviceResourceType type);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResource);
class DeviceResource : public RefCountable, Meta::ThreadResource {
protected:
    DeviceResource(DeviceResourceType resourceType);

public:
    virtual ~DeviceResource();

    using Meta::ThreadResource::CheckThreadId;
    using Meta::ThreadResource::OwnedByThisThread;

    DeviceResourceType ResourceType() const {
        return static_cast<DeviceResourceType>(bitresourcetype_type::Get(_frozenAndResourceType));
    }

    virtual bool Available() const = 0;

    bool Frozen() const { return bitfrozen_type::Get(_frozenAndResourceType); }
    void Freeze();
    void Unfreeze();

    void OnDeviceCreate(DeviceEncapsulator *device);
    void OnDeviceReset(DeviceEncapsulator *device);
    void OnDeviceLost(DeviceEncapsulator *device);
    void OnDeviceDestroy(DeviceEncapsulator *device);

    const char *ResourceName() const;
    void SetResourceName(const char *name);
    void SetResourceName(String&& name);

protected:
    virtual void FreezeImpl() {}
    virtual void UnfreezeImpl() {}

    virtual void OnDeviceCreateImpl(DeviceEncapsulator *device) {}
    virtual void OnDeviceResetImpl(DeviceEncapsulator *device) {}
    virtual void OnDeviceLostImpl(DeviceEncapsulator *device) {}
    virtual void OnDeviceDestroyImpl(DeviceEncapsulator *device) {}

private:
    typedef Meta::Bit<u32>::First<1>::type bitfrozen_type;
    typedef Meta::Bit<u32>::After<bitfrozen_type>::Remain::type bitresourcetype_type;

    u32 _frozenAndResourceType;

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    String _resourceName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <DeviceResourceType _ResourceType>
class TypedDeviceResource : public DeviceResource {
public:
    TypedDeviceResource() : DeviceResource(_ResourceType) {}
    virtual ~TypedDeviceResource() {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
