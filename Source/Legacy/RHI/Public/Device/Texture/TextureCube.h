#pragma once

#include "Graphics.h"

#include "Device/Texture/Texture.h"

#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantTextureCube);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(TextureCube);
class FTextureCube : public FTexture {
public:
    enum class EFace {
        PositiveX = 0,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,
    };

    FTextureCube(
        size_t width, size_t height, size_t levelCount,
        const FSurfaceFormat *format,
        EBufferMode mode, EBufferUsage usage,
        bool sharable );
    virtual ~FTextureCube();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    float4 DuDvDimensions() const;

    const PDeviceAPIDependantTextureCube& DeviceAPIDependantTextureCube() const { return _deviceAPIDependantTextureCube; }

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData);
    virtual void Destroy(IDeviceAPIEncapsulator *device) override;

    virtual Graphics::FDeviceAPIDependantTexture *TextureEntity() const override;

    virtual size_t SizeInBytes() const override;

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FTexture *psource) override;
    void CopyFrom(IDeviceAPIEncapsulator *device, const FTextureCube *psourceCube);

    void CopySubPart(   IDeviceAPIEncapsulator *device,
                        EFace dstFace, size_t dstLevel, const uint2& dstPos,
                        const FTextureCube *psourceCube, EFace srcFace, size_t srcLevel, const FAabb2u& srcBox );


protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const override;

private:
    void Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData);

    u32 _width;
    u32 _height;
    u32 _levelCount;

    PDeviceAPIDependantTextureCube _deviceAPIDependantTextureCube;
};
//----------------------------------------------------------------------------
template <typename T>
void FTextureCube::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData) {
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantTextureCube : public Graphics::FDeviceAPIDependantTexture {
public:
    FDeviceAPIDependantTextureCube(IDeviceAPIEncapsulator *device, const FTextureCube *resource, const TMemoryView<const u8>& optionalData);
    virtual ~FDeviceAPIDependantTextureCube();

    /*const FTextureCube *TypedResource() const {
        return checked_cast<const FTextureCube *>(Resource());
    }*/

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }
    size_t LevelCount() const { return _levelCount; }

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource) = 0;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
                                const FDeviceAPIDependantTextureCube *psource, FTextureCube::EFace srcFace, size_t srcLevel, const FAabb2u& srcBox ) = 0;

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
} //!namespace PPE
