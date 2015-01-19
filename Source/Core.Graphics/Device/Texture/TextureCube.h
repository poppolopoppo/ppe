#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Texture/Texture.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantTextureCube);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(TextureCube);
class TextureCube : public Texture {
public:
    enum class Face {
        PositiveX = 0,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,
    };

    TextureCube(
        size_t width,
        size_t height,
        size_t levelCount,
        const SurfaceFormat *format,
        BufferMode mode,
        BufferUsage usage);
    virtual ~TextureCube();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    float4 DuDvDimensions() const;

    const PDeviceAPIDependantTextureCube& DeviceAPIDependantTextureCube() const { return _deviceAPIDependantTextureCube; }

    void GetData(
        IDeviceAPIEncapsulator *device,
        Face face,
        size_t level,
        size_t x, size_t y,
        size_t width, size_t height,
        void *const dst, size_t stride, size_t count );

    void SetData(
        IDeviceAPIEncapsulator *device,
        Face face,
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

    PDeviceAPIDependantTextureCube _deviceAPIDependantTextureCube;
};
//----------------------------------------------------------------------------
template <typename T>
void TextureCube::Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantTextureCube : public Graphics::DeviceAPIDependantTexture {
public:
    DeviceAPIDependantTextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantTextureCube();

    const TextureCube *Owner() const {
        return checked_cast<const TextureCube *>(DeviceAPIDependantTexture::Owner());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
