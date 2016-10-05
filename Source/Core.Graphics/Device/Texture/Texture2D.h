#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/Texture/Texture.h"

#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class FDepthStencil;
class FRenderTarget;
FWD_REFPTR(DeviceAPIDependantTexture2D);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Texture2D);
class FTexture2D : public FTexture {
public:
    FTexture2D(
        size_t width, size_t height, size_t levelCount,
        const FSurfaceFormat *format,
        EBufferMode mode, EBufferUsage usage,
        bool sharable );
    FTexture2D(
        size_t width, size_t height, size_t levelCount,
        const FSurfaceFormat *format,
        EBufferMode mode, EBufferUsage usage,
        bool sharable,
        Graphics::FDeviceAPIDependantTexture2D *deviceAPIDependantTexture2D );
    virtual ~FTexture2D();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    float4 DuDvDimensions() const;

    const PDeviceAPIDependantTexture2D& DeviceAPIDependantTexture2D() const { return _deviceAPIDependantTexture2D; }

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData);
    virtual void Destroy(IDeviceAPIEncapsulator *device) override;

    virtual Graphics::FDeviceAPIDependantTexture *TextureEntity() const override;

    virtual size_t SizeInBytes() const override;

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FTexture *psource) override;
    void CopyFrom(IDeviceAPIEncapsulator *device, const FTexture2D *psource2D);

    void CopySubPart(   IDeviceAPIEncapsulator *device,
                        size_t dstLevel, const uint2& dstPos,
                        const FTexture2D *psource2D, size_t srcLevel, const AABB2u& srcBox );

protected:
    void Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData);

    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const override;

    PDeviceAPIDependantTexture2D _deviceAPIDependantTexture2D;

private:
    u32 _width;
    u32 _height;
    u32 _levelCount;
};
//----------------------------------------------------------------------------
template <typename T>
void FTexture2D::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantTexture2D : public FDeviceAPIDependantTexture {
public:
    FDeviceAPIDependantTexture2D(IDeviceAPIEncapsulator *device, const FTexture2D *owner, const TMemoryView<const u8>& optionalData);
    virtual ~FDeviceAPIDependantTexture2D();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) = 0;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                size_t dstLevel, const uint2& dstPos,
                                const FDeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) = 0;

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
