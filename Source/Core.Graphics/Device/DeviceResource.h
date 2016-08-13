#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/IO/String.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Memory/WeakPtr.h"

#include "Core/Meta/BitField.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Graphics/Device/DeviceResourceType.h"

#ifndef FINAL_RELEASE
#   define WITH_GRAPHICS_DEVICERESOURCE_NAME
#endif

namespace Core {
namespace Graphics {
FWD_REFPTR(DeviceAPIDependantEntity);
class DeviceEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResource);
FWD_WEAKPTR(DeviceResource);
class DeviceResource : public WeakAndRefCountable {
protected:
    explicit DeviceResource(DeviceResourceType resourceType);

    DeviceResource(const DeviceResource& ) = delete;
    DeviceResource& operator =(const DeviceResource& ) = delete;

public:
    virtual ~DeviceResource();

    using Meta::ThreadResource::CheckThreadId;
    using Meta::ThreadResource::OwnedByThisThread;

    DeviceResourceType ResourceType() const {
        return static_cast<DeviceResourceType>(bitresourcetype_type::Get(_flagsAndResourceType));
    }

    virtual bool Available() const = 0;

    virtual DeviceAPIDependantEntity *TerminalEntity() const = 0;

    bool Frozen() const { return bitfrozen_type::Get(_flagsAndResourceType); }
    void Freeze();
    void Unfreeze();

    void OnDeviceCreate(DeviceEncapsulator *device);
    void OnDeviceReset(DeviceEncapsulator *device);
    void OnDeviceLost(DeviceEncapsulator *device);
    void OnDeviceDestroy(DeviceEncapsulator *device);

    StringSlice ResourceName() const;
    void SetResourceName(const char *name);
    void SetResourceName(const StringSlice& name);
    void SetResourceName(String&& name);

protected:
    bool Sharable_() const { return bitsharable_type::Get(_flagsAndResourceType); }
    void SetSharable_(bool value) { bitsharable_type::InplaceSet(_flagsAndResourceType, value); }

    virtual void FreezeImpl() {}
    virtual void UnfreezeImpl() {}

    virtual void VirtualOnDeviceCreate(DeviceEncapsulator * /* device */) {}
    virtual void VirtualOnDeviceReset(DeviceEncapsulator * /* device */) {}
    virtual void VirtualOnDeviceLost(DeviceEncapsulator * /* device */) {}
    virtual void VirtualOnDeviceDestroy(DeviceEncapsulator * /* device */) {}

private:
    typedef Meta::Bit<u32>::First<1>::type bitfrozen_type;
    typedef Meta::Bit<u32>::After<bitfrozen_type>::Field<1>::type bitsharable_type; // here to save some space
    typedef Meta::Bit<u32>::After<bitsharable_type>::Remain::type bitresourcetype_type;

    u32 _flagsAndResourceType;

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    String _resourceName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
