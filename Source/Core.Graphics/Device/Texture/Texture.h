#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/IDeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/Pool/DeviceResourceSharable.h"

namespace Core {
namespace Graphics {
enum class EBufferMode : u32;
enum class EBufferUsage : u32;
class FSurfaceFormat;
FWD_REFPTR(DeviceAPIDependantTexture);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Texture);
FWD_WEAKPTR(Texture);
class FTexture : public FDeviceResourceSharable {
public:
    FTexture(EDeviceResourceType textureType, const FSurfaceFormat *format, EBufferMode mode, EBufferUsage usage, bool sharable);
    virtual ~FTexture();

    const FSurfaceFormat *Format() const { return _format; }

    EBufferMode Mode() const { return static_cast<EBufferMode>(bitmode_type::Get(_usageAndMode)); }
    EBufferUsage Usage() const { return static_cast<EBufferUsage>(bitusage_type::Get(_usageAndMode)); }

    virtual Graphics::FDeviceAPIDependantTexture *TextureEntity() const = 0;

    virtual size_t SizeInBytes() const = 0;

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual void Destroy(IDeviceAPIEncapsulator *device) = 0;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FTexture *pother) = 0;

protected:
    size_t HashValue_() const;
    bool Match_(const FDeviceAPIDependantTexture& texture) const;

private:
    typedef Meta::TBit<u32>::TFirst<2>::type bitusage_type;
    typedef Meta::TBit<u32>::TAfter<bitusage_type>::TField<2>::type bitmode_type;

    u32 _usageAndMode;
    const FSurfaceFormat *_format;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIDependantAbstractTextureContent {
public:
    virtual ~IDeviceAPIDependantAbstractTextureContent() {}
};
//----------------------------------------------------------------------------
class FDeviceAPIDependantTexture : public TTypedDeviceAPIDependantEntity<FTexture> {
public:
    FDeviceAPIDependantTexture(IDeviceAPIEncapsulator *device, const FTexture *resource);
    virtual ~FDeviceAPIDependantTexture();

    const FSurfaceFormat *Format() const { return _format; }
    EBufferMode Mode() const { return _mode; }
    EBufferUsage Usage() const { return _usage; }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const = 0;

private:
    const FSurfaceFormat *_format;
    EBufferMode _mode;
    EBufferUsage _usage;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
