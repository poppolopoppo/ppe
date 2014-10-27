#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantDepthStencilState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class CompareFunction {
    Always = 0,
    Equal,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    Never,
    NotEqual
};
//----------------------------------------------------------------------------
enum class StencilOperation {
    Decrement = 0,
    DecrementSaturation,
    Increment,
    IncrementSaturation,
    Invert,
    Keep,
    Replace,
    Zero
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DepthStencilState);
class DepthStencilState : public TypedDeviceResource<DeviceResourceType::State> {
public:
    DepthStencilState();
    virtual ~DepthStencilState();

    virtual bool Available() const override { return nullptr != _deviceAPIDependantState; }
    const PDeviceAPIDependantDepthStencilState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    bool DepthBufferEnabled() const { return _depthBufferEnabled; }
    void SetDepthBufferEnabled(bool v) { Assert(!Frozen()); _depthBufferEnabled = v; }

    bool DepthBufferWriteEnabled() const { return _depthBufferWriteEnabled; }
    void SetDepthBufferWriteEnabled(bool v) { Assert(!Frozen()); _depthBufferWriteEnabled = v; }

    CompareFunction DepthBufferFunction() const { return _depthBufferFunction; }
    void SetDepthBufferFunction(CompareFunction v) { Assert(!Frozen()); _depthBufferFunction = v; }

    u32 RendererStencil() const { return _rendererStencil; }
    void SetRendererStencil(u32 v) { Assert(!Frozen()); _rendererStencil = v; }

    bool StencilEnabled() const { return _stencilEnabled; }
    void SetStencilEnabled(bool v) { Assert(!Frozen()); _stencilEnabled = v; }

    u32 StencilReadMask() const { return _stencilReadMask; }
    void SetStencilReadMask(u32 v) { Assert(!Frozen()); _stencilReadMask = v; }

    u32 StencilWriteMask() const { return _stencilWriteMask; }
    void SetStencilWriteMask(u32 v) { Assert(!Frozen()); _stencilWriteMask = v; }

    StencilOperation FrontFaceStencilDepthBufferFail() const { return _frontFaceStencilDepthBufferFail; }
    void SetFrontFaceStencilDepthBufferFail(StencilOperation v) { Assert(!Frozen()); _frontFaceStencilDepthBufferFail = v; }

    StencilOperation FrontFaceStencilFail() const { return _frontFaceStencilFail; }
    void SetFrontFaceStencilFail(StencilOperation v) { Assert(!Frozen()); _frontFaceStencilFail = v; }

    StencilOperation FrontFaceStencilPass() const { return _frontFaceStencilPass; }
    void SetFrontFaceStencilPass(StencilOperation v) { Assert(!Frozen()); _frontFaceStencilPass = v; }

    CompareFunction FrontFaceStencilFunction() const { return _frontFaceStencilFunction; }
    void SetFrontFaceStencilFunction(CompareFunction v) { Assert(!Frozen()); _frontFaceStencilFunction = v; }

    StencilOperation BackFaceStencilDepthBufferFail() const { return _backFaceStencilDepthBufferFail; }
    void SetBackFaceStencilDepthBufferFail(StencilOperation v) { Assert(!Frozen()); _backFaceStencilDepthBufferFail = v; }

    StencilOperation BackFaceStencilFail() const { return _backFaceStencilFail; }
    void SetBackFaceStencilFail(StencilOperation v) { Assert(!Frozen()); _backFaceStencilFail = v; }

    CompareFunction BackFaceStencilFunction() const { return _backFaceStencilFunction; }
    void SetBackFaceStencilFunction(CompareFunction v) { Assert(!Frozen()); _backFaceStencilFunction = v; }

    StencilOperation BackFaceStencilPass() const { return _backFaceStencilPass; }
    void SetBackFaceStencilPass(StencilOperation v) { Assert(!Frozen()); _backFaceStencilPass = v; }

    static const DepthStencilState *Default;
    static const DepthStencilState *DepthRead;
    static const DepthStencilState *None;

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

private:
    bool _depthBufferEnabled = true;
    bool _depthBufferWriteEnabled = true;
    CompareFunction _depthBufferFunction = CompareFunction::LessEqual;

    u32 _rendererStencil = 0;
    bool _stencilEnabled = false;
    u32 _stencilReadMask = UINT8_MAX;
    u32 _stencilWriteMask = UINT8_MAX;

    StencilOperation _frontFaceStencilDepthBufferFail = StencilOperation::Keep;
    StencilOperation _frontFaceStencilFail = StencilOperation::Keep;
    StencilOperation _frontFaceStencilPass = StencilOperation::Keep;
    CompareFunction _frontFaceStencilFunction = CompareFunction::Always;

    bool _twoSidedStencilMode = false;

    StencilOperation _backFaceStencilDepthBufferFail = StencilOperation::Keep;
    StencilOperation _backFaceStencilFail = StencilOperation::Keep;
    CompareFunction _backFaceStencilFunction = CompareFunction::Always;
    StencilOperation _backFaceStencilPass = StencilOperation::Keep;

    PDeviceAPIDependantDepthStencilState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantDepthStencilState : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantDepthStencilState(IDeviceAPIEncapsulator *device, DepthStencilState *owner);
    virtual ~DeviceAPIDependantDepthStencilState();

    const DepthStencilState *Owner() const { return _owner; }

private:
    DepthStencilState *_owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
