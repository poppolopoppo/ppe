#pragma once

#include "Graphics.h"

#include "Device/DeviceAPIDependantEntity.h"
#include "Device/DeviceResource.h"

#include "Maths/ScalarVector.h"

namespace PPE {
namespace Graphics {
class FDeviceEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantBlendState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBlend {
    Zero = 0,
    One,
    SourceColor,
    InverseSourceColor,
    SourceAlpha,
    InverseSourceAlpha,
    DestinationAlpha,
    InverseDestinationAlpha,
    DestinationColor,
    InverseDestinationColor,
    SourceAlphaSaturation,
    BlendFactor,
    InverseBlendFactor,
};
//----------------------------------------------------------------------------
enum class EBlendFunction {
    Add = 0,
    Max,
    Min,
    ReverseSubtract,
    Subtract
};
//----------------------------------------------------------------------------
enum class EColorChannels {
    None = 0,
    Red = 1,
    Green = 2,
    Blue = 4,
    Alpha = 8,
    All = Alpha|Blue|Green|Red
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(BlendState);
class FBlendState : public FDeviceResource {
public:
    FBlendState();
    virtual ~FBlendState();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantBlendState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    bool BlendEnabled() const { return _blendEnabled; }
    void SetBlendEnabled(bool v) { Assert(!Frozen()); _blendEnabled = v; }

    const float4& BlendFactor() const { return _blendFactor; }
    void SetBlendFactor(const float4& v) { Assert(!Frozen()); _blendFactor = v; }

    EBlendFunction AlphaBlendFunction() const { return _alphaBlendFunction; }
    void SetAlphaBlendFunction(EBlendFunction v) { Assert(!Frozen()); _alphaBlendFunction = v; }

    EBlend AlphaDestinationBlend() const { return _alphaDestinationBlend; }
    void SetAlphaDestinationBlend(EBlend v) { Assert(!Frozen()); _alphaDestinationBlend = v; }

    EBlend AlphaSourceBlend() const { return _alphaSourceBlend; }
    void SetAlphaSourceBlend(EBlend v) { Assert(!Frozen()); _alphaSourceBlend = v; }

    EBlendFunction ColorBlendFunction() const { return _colorBlendFunction; }
    void SetColorBlendFunction(EBlendFunction v) { Assert(!Frozen()); _colorBlendFunction = v; }

    EBlend ColorDestinationBlend() const { return _colorDestinationBlend; }
    void SetColorDestinationBlend(EBlend v) { Assert(!Frozen()); _colorDestinationBlend = v; }

    EBlend ColorSourceBlend() const { return _colorSourceBlend; }
    void SetColorSourceBlend(EBlend v) { Assert(!Frozen()); _colorSourceBlend = v; }

    EColorChannels ColorWriteChannels() const { return _colorWriteChannels; }
    void SetColorWriteChannels(EColorChannels v) { Assert(!Frozen()); _colorWriteChannels = v; }

    u32 MultiSampleMask() const { return _multiSampleMask; }
    void MultiSampleMask(u32 v) { Assert(!Frozen()); _multiSampleMask = v; }

    static const FBlendState *Additive;
    static const FBlendState *AlphaBlend;
    static const FBlendState *NonPremultiplied;
    static const FBlendState *Opaque;

    static void Start();
    static void Shutdown();

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

private:
    bool _blendEnabled;
    float4 _blendFactor;

    EBlendFunction _alphaBlendFunction = EBlendFunction::Add;
    EBlend _alphaDestinationBlend = EBlend::One;
    EBlend _alphaSourceBlend = EBlend::One;

    EBlendFunction _colorBlendFunction = EBlendFunction::Add;
    EBlend _colorDestinationBlend = EBlend::One;
    EBlend _colorSourceBlend = EBlend::One;

    EColorChannels _colorWriteChannels = EColorChannels::All;

    u32 _multiSampleMask = UINT32_MAX;

    PDeviceAPIDependantBlendState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantBlendState : public TTypedDeviceAPIDependantEntity<FBlendState> {
public:
    FDeviceAPIDependantBlendState(IDeviceAPIEncapsulator *device, const FBlendState *resource);
    virtual ~FDeviceAPIDependantBlendState();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
