#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantRasterizerState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECullMode {
    CullClockwiseFace = 0,
    CullCounterClockwiseFace,
    None,
};
//----------------------------------------------------------------------------
enum class EFillMode {
    Solid = 0,
    WireFrame,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RasterizerState);
class FRasterizerState : public FDeviceResource {
public:
    FRasterizerState();
    virtual ~FRasterizerState();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantRasterizerState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    Graphics::ECullMode CullMode() const { return _cullMode; }
    void SetCullMode(Graphics::ECullMode v) { Assert(!Frozen()); _cullMode = v; }

    Graphics::EFillMode FillMode() const { return _fillMode; }
    void SetFillMode(Graphics::EFillMode v) { Assert(!Frozen()); _fillMode = v; }

    bool MultiSampleAntiAlias() const { return _multiSampleAntiAlias; }
    void SetMultiSampleAntiAlias(bool v) { Assert(!Frozen()); _multiSampleAntiAlias = v; }

    bool ScissorTestEnabled() const { return _scissorTestEnabled; }
    void SetScissorTestEnabled(bool v) { Assert(!Frozen()); _scissorTestEnabled = v; }

    i32 DepthBias() const { return _depthBias; }
    void SetDepthBias(i32 v) { Assert(!Frozen()); _depthBias = v; }

    float SlopeScaledDepthBias() const { return _slopeScaledDepthBias; }
    void SetSlopeScaledDepthBias(float v) { Assert(!Frozen()); _slopeScaledDepthBias = v; }

    static const FRasterizerState *CullClockwise;
    static const FRasterizerState *CullCounterClockwise;
    static const FRasterizerState *CullNone;
    static const FRasterizerState *Wireframe;

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

private:
    Graphics::ECullMode _cullMode = ECullMode::CullClockwiseFace;
    Graphics::EFillMode _fillMode = EFillMode::Solid;

    bool _multiSampleAntiAlias = true;
    bool _scissorTestEnabled = false;

    i32 _depthBias = 0;
    float _slopeScaledDepthBias = 0;

    PDeviceAPIDependantRasterizerState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantRasterizerState : public TTypedDeviceAPIDependantEntity<FRasterizerState> {
public:
    FDeviceAPIDependantRasterizerState(IDeviceAPIEncapsulator *device, const FRasterizerState *resource);
    virtual ~FDeviceAPIDependantRasterizerState();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
