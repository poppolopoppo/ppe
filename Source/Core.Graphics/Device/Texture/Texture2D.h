#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Texture/Texture.h"

#include "Core/Maths/Geometry/ScalarBoundingBox_fwd.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class DepthStencil;
class RenderTarget;
FWD_REFPTR(DeviceAPIDependantTexture2D);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Texture2D);
class Texture2D : public Texture {
public:
    Texture2D(
        size_t width, size_t height, size_t levelCount,
        const SurfaceFormat *format,
        BufferMode mode, BufferUsage usage,
        bool sharable );
    Texture2D(
        size_t width, size_t height, size_t levelCount,
        const SurfaceFormat *format,
        BufferMode mode, BufferUsage usage,
        bool sharable,
        Graphics::DeviceAPIDependantTexture2D *deviceAPIDependantTexture2D );
    virtual ~Texture2D();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    float4 DuDvDimensions() const;

    const PDeviceAPIDependantTexture2D& DeviceAPIDependantTexture2D() const { return _deviceAPIDependantTexture2D; }

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData);
    virtual void Destroy(IDeviceAPIEncapsulator *device) override;

    virtual Graphics::DeviceAPIDependantTexture *TextureEntity() const override;

    virtual size_t SizeInBytes() const override;

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const Texture *psource) override;
    void CopyFrom(IDeviceAPIEncapsulator *device, const Texture2D *psource2D);

    void CopySubPart(   IDeviceAPIEncapsulator *device, 
                        size_t dstLevel, const uint2& dstPos, 
                        const Texture2D *psource2D, size_t srcLevel, const AABB2u& srcBox );

protected:
    void Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData);

    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const override;

    PDeviceAPIDependantTexture2D _deviceAPIDependantTexture2D;

private:
    u32 _width;
    u32 _height;
    u32 _levelCount;
};
//----------------------------------------------------------------------------
template <typename T>
void Texture2D::Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantTexture2D : public DeviceAPIDependantTexture {
public:
    DeviceAPIDependantTexture2D(IDeviceAPIEncapsulator *device, const Texture2D *owner, const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantTexture2D();

    const Texture2D *TypedResource() const {
        return checked_cast<const Texture2D *>(Resource());
    }

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) = 0;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, 
                                size_t dstLevel, const uint2& dstPos, 
                                const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) = 0;

    virtual size_t VideoMemorySizeInBytes() const override;

private:
    u32 _width;
    u32 _height;
    u32 _levelCount;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
