#pragma once

#include "Graphics.h"

#include "DeviceResource.h"
#include "DeviceAPIDependantEntity.h"

#include "Core/ScalarVector.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantBlendState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class Blend {
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
enum class BlendFunction {
    Add = 0,
    Max,
    Min,
    ReverseSubstract,
    Substract
};
//----------------------------------------------------------------------------
enum class ColorChannels {
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
class BlendState : public TypedDeviceResource<DeviceResourceType::State> {
public:
    BlendState();
    virtual ~BlendState();

    virtual bool Available() const override { return nullptr != _deviceAPIDependantState; }
    const PDeviceAPIDependantBlendState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    bool BlendEnabled() const { return _blendEnabled; }
    void SetBlendEnabled(bool v) { Assert(!Frozen()); _blendEnabled = v; }

    const float4& BlendFactor() const { return _blendFactor; }
    void SetBlendFactor(const float4& v) { Assert(!Frozen()); _blendFactor = v; }

    BlendFunction AlphaBlendFunction() const { return _alphaBlendFunction; }
    void SetAlphaBlendFunction(BlendFunction v) { Assert(!Frozen()); _alphaBlendFunction = v; }

    Blend AlphaDestinationBlend() const { return _alphaDestinationBlend; }
    void SetAlphaDestinationBlend(Blend v) { Assert(!Frozen()); _alphaDestinationBlend = v; }

    Blend AlphaSourceBlend() const { return _alphaSourceBlend; }
    void SetAlphaSourceBlend(Blend v) { Assert(!Frozen()); _alphaSourceBlend = v; }

    BlendFunction ColorBlendFunction() const { return _colorBlendFunction; }
    void SetColorBlendFunction(BlendFunction v) { Assert(!Frozen()); _colorBlendFunction = v; }

    Blend ColorDestinationBlend() const { return _colorDestinationBlend; }
    void SetColorDestinationBlend(Blend v) { Assert(!Frozen()); _colorDestinationBlend = v; }

    Blend ColorSourceBlend() const { return _colorSourceBlend; }
    void SetColorSourceBlend(Blend v) { Assert(!Frozen()); _colorSourceBlend = v; }

    ColorChannels ColorWriteChannels() const { return _colorWriteChannels; }
    void SetColorWriteChannels(ColorChannels v) { Assert(!Frozen()); _colorWriteChannels = v; }

    u32 MultiSampleMask() const { return _multiSampleMask; }
    void MultiSampleMask(u32 v) { Assert(!Frozen()); _multiSampleMask = v; }

    static const BlendState *Additive;
    static const BlendState *AlphaBlend;
    static const BlendState *NonPremultiplied;
    static const BlendState *Opaque;

    static void Start();
    static void Shutdown();

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

private:
    bool _blendEnabled;
    float4 _blendFactor;

    BlendFunction _alphaBlendFunction = BlendFunction::Add;
    Blend _alphaDestinationBlend = Blend::One;
    Blend _alphaSourceBlend = Blend::One;

    BlendFunction _colorBlendFunction = BlendFunction::Add;
    Blend _colorDestinationBlend = Blend::One;
    Blend _colorSourceBlend = Blend::One;

    ColorChannels _colorWriteChannels = ColorChannels::All;

    u32 _multiSampleMask = UINT32_MAX;

    PDeviceAPIDependantBlendState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantBlendState : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantBlendState(IDeviceAPIEncapsulator *device, BlendState *owner);
    virtual ~DeviceAPIDependantBlendState();

    const BlendState *Owner() const { return _owner; }

private:
    BlendState *_owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
