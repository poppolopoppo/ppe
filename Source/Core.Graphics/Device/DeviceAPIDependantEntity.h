#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceRevision.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/BitField.h"

#include <type_traits>

namespace Core {
namespace Graphics {
class FAbstractDeviceAPIEncapsulator;
enum class EDeviceAPI;
FWD_REFPTR(DeviceResource);
enum class EDeviceResourceType;
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceAPIDependantEntity);
class FDeviceAPIDependantEntity : public FRefCountable {
protected:
    FDeviceAPIDependantEntity(const FAbstractDeviceAPIEncapsulator *encapsulator, const FDeviceResource *resource);
    FDeviceAPIDependantEntity(const FAbstractDeviceAPIEncapsulator *encapsulator, EDeviceResourceType resourceType);

public:
    virtual ~FDeviceAPIDependantEntity();

    FDeviceAPIDependantEntity(const FDeviceAPIDependantEntity& ) = delete;
    FDeviceAPIDependantEntity& operator =(const FDeviceAPIDependantEntity& ) = delete;

    EDeviceAPI API() const { return static_cast<EDeviceAPI>(bitdevicapi_type::Get(_apiAndResourceType)); }
    EDeviceResourceType ResourceType() const { return static_cast<EDeviceResourceType>(bitresourcetype_type::Get(_apiAndResourceType)); }

    bool MatchDevice(const IDeviceAPIEncapsulator *device) const;

    //const FDeviceResource *Resource() const { return _resource.get(); }
    bool IsAttachedToResource() const { return nullptr != _resource; }

    void AttachResource(const FDeviceResource *resource);
    void DetachResource(const FDeviceResource *resource);

    FDeviceRevision CreatedAt() const { return _createdAt; }
    FDeviceRevision LastUsed() const { return _lastUsed; }

    void SetCreatedAt(FDeviceRevision revision);
    void SetLastUsed(FDeviceRevision revision);

    virtual size_t VideoMemorySizeInBytes() const = 0;

private:
    typedef Meta::TBit<u32>::TFirst<3>::type bitdevicapi_type;
    typedef Meta::TBit<u32>::TAfter<bitdevicapi_type>::FRemain::type bitresourcetype_type;

    u32 _apiAndResourceType;
    SCDeviceResource _resource;

    FDeviceRevision _createdAt;
    FDeviceRevision _lastUsed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TTypedDeviceAPIDependantEntity : public FDeviceAPIDependantEntity {
protected:
    STATIC_ASSERT(std::is_base_of<FDeviceResource, T>::value);
    TTypedDeviceAPIDependantEntity(const FAbstractDeviceAPIEncapsulator *encapsulator, const T *resource)
        : FDeviceAPIDependantEntity(encapsulator, resource) {}

public:
    const T *TypedResource() const { return checked_cast<const T *>(FDeviceAPIDependantEntity::Resource()); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
