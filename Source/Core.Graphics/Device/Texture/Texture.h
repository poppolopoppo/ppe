#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/IDeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/Pool/DeviceResourceSharable.h"

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
FWD_WEAKPTR(Texture);
class Texture : public DeviceResourceSharable {
public:
    Texture(DeviceResourceType textureType, const SurfaceFormat *format, BufferMode mode, BufferUsage usage, bool sharable);
    virtual ~Texture();

    const SurfaceFormat *Format() const { return _format; }

    BufferMode Mode() const { return static_cast<BufferMode>(bitmode_type::Get(_usageAndMode)); }
    BufferUsage Usage() const { return static_cast<BufferUsage>(bitusage_type::Get(_usageAndMode)); }

    virtual Graphics::DeviceAPIDependantTexture *TextureEntity() const = 0;

    virtual size_t SizeInBytes() const = 0;

    virtual bool Available() const override;
    virtual DeviceAPIDependantEntity *TerminalEntity() const override;

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual void Destroy(IDeviceAPIEncapsulator *device) = 0;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const Texture *pother) = 0;

protected:
    size_t HashValue_() const;
    bool Match_(const DeviceAPIDependantTexture& texture) const;
    
private:
    typedef Meta::Bit<u32>::First<2>::type bitusage_type;
    typedef Meta::Bit<u32>::After<bitusage_type>::Field<2>::type bitmode_type;

    u32 _usageAndMode;
    const SurfaceFormat *_format;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIDependantAbstractTextureContent {
public:
    virtual ~IDeviceAPIDependantAbstractTextureContent() {}
};
//----------------------------------------------------------------------------
class DeviceAPIDependantTexture : public TypedDeviceAPIDependantEntity<Texture> {
public:
    DeviceAPIDependantTexture(IDeviceAPIEncapsulator *device, const Texture *resource);
    virtual ~DeviceAPIDependantTexture();

    const SurfaceFormat *Format() const { return _format; }
    BufferMode Mode() const { return _mode; }
    BufferUsage Usage() const { return _usage; }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const = 0;

private:
    const SurfaceFormat *_format;
    BufferMode _mode;
    BufferUsage _usage;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
