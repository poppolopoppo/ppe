#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantSamplerState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class TextureAddressMode {
    Clamp = 0,
    Mirror,
    Wrap,
};
//----------------------------------------------------------------------------
enum class TextureFilter {
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
class SamplerState : public DeviceResource {
public:
    SamplerState();
    virtual ~SamplerState();

    virtual bool Available() const override;
    virtual DeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantSamplerState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    TextureAddressMode AddressU() const { return _addressU; }
    void SetAddressU(TextureAddressMode v) { Assert(!Frozen()); _addressU = v; }

    TextureAddressMode AddressV() const { return _addressV; }
    void SetAddressV(TextureAddressMode v) { Assert(!Frozen()); _addressV = v; }

    TextureAddressMode AddressW() const { return _addressW; }
    void SetAddressW(TextureAddressMode v) { Assert(!Frozen()); _addressW = v; }

    TextureFilter Filter() const { return _filter; }
    void SetFilter(TextureFilter v) { Assert(!Frozen()); _filter = v; }

    u32 MaxAnisotropy() const { return _maxAnisotropy; }
    void SetMaxAnisotropy(u32 v) { Assert(!Frozen()); _maxAnisotropy = v; }

    u32 MaxMipLevel() const { return _maxMipLevel; }
    void SetMaxMipLevel(u32 v) { Assert(!Frozen()); _maxMipLevel = v; }

    float MipMapLODBias() const { return _mipMapLODBias; }
    void SetMipMapLODBias(float v) { Assert(!Frozen()); _mipMapLODBias = v; }

    static const SamplerState *AnisotropicClamp;
    static const SamplerState *AnisotropicWrap;
    static const SamplerState *LinearClamp;
    static const SamplerState *LinearWrap;
    static const SamplerState *PointClamp;
    static const SamplerState *PointWrap;

    static const SamplerState *Default() { return LinearClamp; }

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

private:
    TextureAddressMode _addressU = TextureAddressMode::Wrap;
    TextureAddressMode _addressV = TextureAddressMode::Wrap;
    TextureAddressMode _addressW = TextureAddressMode::Wrap;

    TextureFilter _filter = TextureFilter::Linear;

    u32 _maxAnisotropy = 16;
    u32 _maxMipLevel = 32;
    float _mipMapLODBias = 0;

    PDeviceAPIDependantSamplerState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantSamplerState : public TypedDeviceAPIDependantEntity<SamplerState> {
public:
    DeviceAPIDependantSamplerState(IDeviceAPIEncapsulator *device, const SamplerState *resource);
    virtual ~DeviceAPIDependantSamplerState();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
