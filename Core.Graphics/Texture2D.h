#pragma once

#include "Graphics.h"

#include "Texture.h"

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
    friend class DepthStencil;
    friend class RenderTarget;

    Texture2D(
        size_t width,
        size_t height,
        size_t levelCount,
        const SurfaceFormat *format,
        BufferMode mode,
        BufferUsage usage);
    Texture2D(
        size_t width,
        size_t height,
        size_t levelCount,
        const SurfaceFormat *format,
        BufferMode mode,
        BufferUsage usage,
        Graphics::DeviceAPIDependantTexture2D *deviceAPIDependantTexture2D);
    virtual ~Texture2D();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    float4 DuDvDimensions() const;

    const PDeviceAPIDependantTexture2D& DeviceAPIDependantTexture2D() const { return _deviceAPIDependantTexture2D; }

    void GetData(
        IDeviceAPIEncapsulator *device,
        size_t level,
        size_t x, size_t y,
        size_t width, size_t height,
        void *const dst, size_t stride, size_t count );

    void SetData(
        IDeviceAPIEncapsulator *device,
        size_t level,
        size_t x, size_t y,
        size_t width, size_t height,
        const void *src, size_t stride, size_t count );

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData);
    virtual void Destroy(IDeviceAPIEncapsulator *device) override;

    virtual const Graphics::DeviceAPIDependantTexture *DeviceAPIDependantTexture() const override;

    virtual size_t SizeInBytes() const override;

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

private:
    void Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData);

    u32 _width;
    u32 _height;
    u32 _levelCount;

    PDeviceAPIDependantTexture2D _deviceAPIDependantTexture2D;
};
//----------------------------------------------------------------------------
template <typename T>
void Texture2D::Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIDependantTexture2DContent {
public:
    virtual ~IDeviceAPIDependantTexture2DContent() {}
};
//----------------------------------------------------------------------------
class DeviceAPIDependantTexture2D : public Graphics::DeviceAPIDependantTexture {
public:
    DeviceAPIDependantTexture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantTexture2D();

    const Texture2D *Owner() const {
        return checked_cast<const Texture2D *>(DeviceAPIDependantTexture::Owner());
    }

    virtual const IDeviceAPIDependantTexture2DContent *Content() const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
