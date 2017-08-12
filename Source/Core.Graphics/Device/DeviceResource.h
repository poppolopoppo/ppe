#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Memory/WeakPtr.h"

#include "Core/Meta/BitField.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Graphics/Device/DeviceResourceType.h"

#ifndef FINAL_RELEASE
#   define WITH_GRAPHICS_DEVICERESOURCE_NAME
#endif

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
#   define ONLY_IF_GRAPHICS_DEVICERESOURCE_NAME(...) __VA_ARGS__
#else
#   define ONLY_IF_GRAPHICS_DEVICERESOURCE_NAME(...) NOOP()
#endif

namespace Core {
namespace Graphics {
FWD_REFPTR(DeviceAPIDependantEntity);
class FDeviceEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResource);
FWD_WEAKPTR(DeviceResource);
class FDeviceResource : public FWeakAndRefCountable {
protected:
    explicit FDeviceResource(EDeviceResourceType resourceType);

    FDeviceResource(const FDeviceResource& ) = delete;
    FDeviceResource& operator =(const FDeviceResource& ) = delete;

public:
    virtual ~FDeviceResource();

    using Meta::FThreadResource::CheckThreadId;
    using Meta::FThreadResource::OwnedByThisThread;

    EDeviceResourceType ResourceType() const {
        return static_cast<EDeviceResourceType>(bitresourcetype_type::Get(_flagsAndResourceType));
    }

    virtual bool Available() const = 0;

    virtual FDeviceAPIDependantEntity *TerminalEntity() const = 0;

    bool Frozen() const { return bitfrozen_type::Get(_flagsAndResourceType); }
    void Freeze();
    void Unfreeze();

    void OnDeviceCreate(FDeviceEncapsulator *device);
    void OnDeviceReset(FDeviceEncapsulator *device);
    void OnDeviceLost(FDeviceEncapsulator *device);
    void OnDeviceDestroy(FDeviceEncapsulator *device);

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    FWStringView ResourceName() const;
    void SetResourceName(const wchar_t* name);
    void SetResourceName(const FWStringView& name);
    void SetResourceName(FWString&& name);
#endif

protected:
    bool Sharable_() const { return bitsharable_type::Get(_flagsAndResourceType); }
    void SetSharable_(bool value) { bitsharable_type::InplaceSet(_flagsAndResourceType, value); }

    virtual void FreezeImpl() {}
    virtual void UnfreezeImpl() {}

    virtual void VirtualOnDeviceCreate(FDeviceEncapsulator * /* device */) {}
    virtual void VirtualOnDeviceReset(FDeviceEncapsulator * /* device */) {}
    virtual void VirtualOnDeviceLost(FDeviceEncapsulator * /* device */) {}
    virtual void VirtualOnDeviceDestroy(FDeviceEncapsulator * /* device */) {}

private:
    typedef Meta::TBit<u32>::TFirst<1>::type bitfrozen_type;
    typedef Meta::TBit<u32>::TAfter<bitfrozen_type>::TField<1>::type bitsharable_type; // here to save some space
    typedef Meta::TBit<u32>::TAfter<bitsharable_type>::FRemain::type bitresourcetype_type;

    u32 _flagsAndResourceType;

#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    FWString _resourceName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
