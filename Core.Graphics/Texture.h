#pragma once

#include "Graphics.h"

#include "DeviceAPIDependantEntity.h"
#include "DeviceResource.h"

namespace Core {
namespace Graphics {
enum class BufferMode : u32;
enum class BufferUsage : u32;
class SurfaceFormat;
FWD_REFPTR(DeviceAPIDependantTexture);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Texture);
class Texture : public TypedDeviceResource<DeviceResourceType::Texture> {
public:
    Texture(const SurfaceFormat *format, BufferMode mode, BufferUsage usage);
    virtual ~Texture();

    const SurfaceFormat *Format() const { return _format; }

    BufferMode Mode() const { return static_cast<BufferMode>(bitmode_type::Get(_usageAndMode)); }
    BufferUsage Usage() const { return static_cast<BufferUsage>(bitusage_type::Get(_usageAndMode)); }

    virtual const Graphics::DeviceAPIDependantTexture *DeviceAPIDependantTexture() const = 0;
    virtual size_t SizeInBytes() const = 0;

    virtual bool Available() const override { return DeviceAPIDependantTexture() != nullptr; }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual void Destroy(IDeviceAPIEncapsulator *device) = 0;

private:
    typedef Meta::Bit<u32>::First<2>::type bitusage_type;
    typedef Meta::Bit<u32>::After<bitusage_type>::Field<2>::type bitmode_type;

    u32 _usageAndMode;
    const SurfaceFormat *_format;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantTexture : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantTexture(IDeviceAPIEncapsulator *device, Texture *owner);
    virtual ~DeviceAPIDependantTexture();

    const Texture *Owner() const { return _owner; }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

private:
    Texture *_owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
