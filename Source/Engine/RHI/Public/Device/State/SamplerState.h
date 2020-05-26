#pragma once

#include "Graphics.h"

#include "Device/DeviceAPIDependantEntity.h"
#include "Device/DeviceResource.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantSamplerState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETextureAddressMode {
    Clamp = 0,
    Mirror,
    Wrap,
};
//----------------------------------------------------------------------------
enum class ETextureDimension {
    FTexture2D = 0,
    Texture3D,
    FTextureCube,
    Texture2DArray,
    TextureCubeArray,
};
//----------------------------------------------------------------------------
enum class ETextureFilter {
    Linear = 0,
    Point,
    Anisotropic,
    LinearMipPoint,             // Use linear filtering to shrink or expand, and point filtering between mipmap levels (mip).
    PointMipLinear,             // Use point filtering to shrink (minify) or expand (magnify), and linear filtering between mipmap levels.
    MinLinearMagPointMipLinear, // Use linear filtering to shrink, point filtering to expand, and linear filtering between mipmap levels.
    MinLinearMagPointMipPoint,  // Use linear filtering to shrink, point filtering to expand, and point filtering between mipmap levels.
    MinPointMagLinearMipLinear, // Use point filtering to shrink, linear filtering to expand, and linear filtering between mipmap levels.
    MinPointMagLinearMipPoint,  // MinPointMagLinearMipPoint
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(SamplerState);
class FSamplerState : public FDeviceResource {
public:
    FSamplerState();
    virtual ~FSamplerState();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantSamplerState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    ETextureAddressMode AddressU() const { return _addressU; }
    void SetAddressU(ETextureAddressMode v) { Assert(!Frozen()); _addressU = v; }

    ETextureAddressMode AddressV() const { return _addressV; }
    void SetAddressV(ETextureAddressMode v) { Assert(!Frozen()); _addressV = v; }

    ETextureAddressMode AddressW() const { return _addressW; }
    void SetAddressW(ETextureAddressMode v) { Assert(!Frozen()); _addressW = v; }

    ETextureFilter Filter() const { return _filter; }
    void SetFilter(ETextureFilter v) { Assert(!Frozen()); _filter = v; }

    u32 MaxAnisotropy() const { return _maxAnisotropy; }
    void SetMaxAnisotropy(u32 v) { Assert(!Frozen()); _maxAnisotropy = v; }

    u32 MaxMipLevel() const { return _maxMipLevel; }
    void SetMaxMipLevel(u32 v) { Assert(!Frozen()); _maxMipLevel = v; }

    float MipMapLODBias() const { return _mipMapLODBias; }
    void SetMipMapLODBias(float v) { Assert(!Frozen()); _mipMapLODBias = v; }

    static const FSamplerState *AnisotropicClamp;
    static const FSamplerState *AnisotropicWrap;
    static const FSamplerState *LinearClamp;
    static const FSamplerState *LinearWrap;
    static const FSamplerState *PointClamp;
    static const FSamplerState *PointWrap;

    static const FSamplerState *Default() { return LinearClamp; }

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

private:
    ETextureAddressMode _addressU = ETextureAddressMode::Wrap;
    ETextureAddressMode _addressV = ETextureAddressMode::Wrap;
    ETextureAddressMode _addressW = ETextureAddressMode::Wrap;

    ETextureFilter _filter = ETextureFilter::Linear;

    u32 _maxAnisotropy = 16;
    u32 _maxMipLevel = 32;
    float _mipMapLODBias = 0;

    PDeviceAPIDependantSamplerState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantSamplerState : public TTypedDeviceAPIDependantEntity<FSamplerState> {
public:
    FDeviceAPIDependantSamplerState(IDeviceAPIEncapsulator *device, const FSamplerState *resource);
    virtual ~FDeviceAPIDependantSamplerState();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
