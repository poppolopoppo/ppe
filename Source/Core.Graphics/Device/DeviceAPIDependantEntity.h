#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/BitField.h"

#include <type_traits>

namespace Core {
namespace Graphics {
class AbstractDeviceAPIEncapsulator;
enum class DeviceAPI;
FWD_REFPTR(DeviceResource);
enum class DeviceResourceType;
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceAPIDependantEntity);
class DeviceAPIDependantEntity : public RefCountable {
protected:
    DeviceAPIDependantEntity(const AbstractDeviceAPIEncapsulator *encapsulator, const DeviceResource *resource);

public:
    virtual ~DeviceAPIDependantEntity();

    DeviceAPIDependantEntity(const DeviceAPIDependantEntity& ) = delete;
    DeviceAPIDependantEntity& operator =(const DeviceAPIDependantEntity& ) = delete;

    DeviceAPI API() const { return static_cast<DeviceAPI>(bitdevicapi_type::Get(_apiAndResourceType)); }
    DeviceResourceType ResourceType() const { return static_cast<DeviceResourceType>(bitresourcetype_type::Get(_apiAndResourceType)); }

    bool MatchDevice(const IDeviceAPIEncapsulator *device) const;

    const DeviceResource *Resource() const { return _resource.get(); }
    bool IsAttachedToResource() const { return nullptr != _resource; }

    void AttachResource(const DeviceResource *resource);
    void DetachResource(const DeviceResource *resource);

    virtual size_t VideoMemorySizeInBytes() const = 0;

private:
    typedef Meta::Bit<u32>::First<3>::type bitdevicapi_type;
    typedef Meta::Bit<u32>::After<bitdevicapi_type>::Remain::type bitresourcetype_type;

    u32 _apiAndResourceType;
    SCDeviceResource _resource;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TypedDeviceAPIDependantEntity : public DeviceAPIDependantEntity {
protected:
    STATIC_ASSERT(std::is_base_of<DeviceResource, T>::value);
    TypedDeviceAPIDependantEntity(const AbstractDeviceAPIEncapsulator *encapsulator, const T *resource)
        : DeviceAPIDependantEntity(encapsulator, resource) {}

public:
    const T *TypedResource() const { return checked_cast<const T *>(DeviceAPIDependantEntity::Resource()); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
