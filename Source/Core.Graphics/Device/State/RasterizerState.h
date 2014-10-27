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
enum class CullMode {
    CullClockwiseFace = 0,
    CullCounterClockwiseFace,
    None,
};
//----------------------------------------------------------------------------
enum class FillMode {
    Solid = 0,
    WireFrame,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(RasterizerState);
class RasterizerState : public TypedDeviceResource<DeviceResourceType::State> {
public:
    RasterizerState();
    virtual ~RasterizerState();

    virtual bool Available() const override { return nullptr != _deviceAPIDependantState; }
    const PDeviceAPIDependantRasterizerState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    Graphics::CullMode CullMode() const { return _cullMode; }
    void SetCullMode(Graphics::CullMode v) { Assert(!Frozen()); _cullMode = v; }

    Graphics::FillMode FillMode() const { return _fillMode; }
    void SetFillMode(Graphics::FillMode v) { Assert(!Frozen()); _fillMode = v; }

    bool MultiSampleAntiAlias() const { return _multiSampleAntiAlias; }
    void SetMultiSampleAntiAlias(bool v) { Assert(!Frozen()); _multiSampleAntiAlias = v; }

    bool ScissorTestEnabled() const { return _scissorTestEnabled; }
    void SetScissorTestEnabled(bool v) { Assert(!Frozen()); _scissorTestEnabled = v; }

    i32 DepthBias() const { return _depthBias; }
    void SetDepthBias(i32 v) { Assert(!Frozen()); _depthBias = v; }

    float SlopeScaledDepthBias() const { return _slopeScaledDepthBias; }
    void SetSlopeScaledDepthBias(float v) { Assert(!Frozen()); _slopeScaledDepthBias = v; }

    static const RasterizerState *CullClockwise;
    static const RasterizerState *CullCounterClockwise;
    static const RasterizerState *CullNone;
    static const RasterizerState *Wireframe;

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

private:
    Graphics::CullMode _cullMode = CullMode::CullClockwiseFace;
    Graphics::FillMode _fillMode = FillMode::Solid;

    bool _multiSampleAntiAlias = true;
    bool _scissorTestEnabled = false;

    i32 _depthBias = 0;
    float _slopeScaledDepthBias = 0;

    PDeviceAPIDependantRasterizerState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantRasterizerState : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantRasterizerState(IDeviceAPIEncapsulator *device, RasterizerState *owner);
    virtual ~DeviceAPIDependantRasterizerState();

    const RasterizerState *Owner() const { return _owner; }

private:
    RasterizerState *_owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
