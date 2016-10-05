#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"

namespace Core {
namespace Graphics {
class FDeviceEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantDepthStencilState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECompareFunction {
    Always = 0,
    Equal,
    TGreater,
    TGreaterEqual,
    TLess,
    TLessEqual,
    Never,
    NotEqual
};
//----------------------------------------------------------------------------
enum class EStencilOperation {
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
class FDepthStencilState : public FDeviceResource {
public:
    FDepthStencilState();
    virtual ~FDepthStencilState();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantDepthStencilState& DeviceAPIDependantState() const {
        Assert(Frozen()); return _deviceAPIDependantState;
    }

    bool DepthBufferEnabled() const { return _depthBufferEnabled; }
    void SetDepthBufferEnabled(bool v) { Assert(!Frozen()); _depthBufferEnabled = v; }

    bool DepthBufferWriteEnabled() const { return _depthBufferWriteEnabled; }
    void SetDepthBufferWriteEnabled(bool v) { Assert(!Frozen()); _depthBufferWriteEnabled = v; }

    ECompareFunction DepthBufferFunction() const { return _depthBufferFunction; }
    void SetDepthBufferFunction(ECompareFunction v) { Assert(!Frozen()); _depthBufferFunction = v; }

    u32 RendererStencil() const { return _rendererStencil; }
    void SetRendererStencil(u32 v) { Assert(!Frozen()); _rendererStencil = v; }

    bool StencilEnabled() const { return _stencilEnabled; }
    void SetStencilEnabled(bool v) { Assert(!Frozen()); _stencilEnabled = v; }

    u32 StencilReadMask() const { return _stencilReadMask; }
    void SetStencilReadMask(u32 v) { Assert(!Frozen()); _stencilReadMask = v; }

    u32 StencilWriteMask() const { return _stencilWriteMask; }
    void SetStencilWriteMask(u32 v) { Assert(!Frozen()); _stencilWriteMask = v; }

    EStencilOperation FrontFaceStencilDepthBufferFail() const { return _frontFaceStencilDepthBufferFail; }
    void SetFrontFaceStencilDepthBufferFail(EStencilOperation v) { Assert(!Frozen()); _frontFaceStencilDepthBufferFail = v; }

    EStencilOperation FrontFaceStencilFail() const { return _frontFaceStencilFail; }
    void SetFrontFaceStencilFail(EStencilOperation v) { Assert(!Frozen()); _frontFaceStencilFail = v; }

    EStencilOperation FrontFaceStencilPass() const { return _frontFaceStencilPass; }
    void SetFrontFaceStencilPass(EStencilOperation v) { Assert(!Frozen()); _frontFaceStencilPass = v; }

    ECompareFunction FrontFaceStencilFunction() const { return _frontFaceStencilFunction; }
    void SetFrontFaceStencilFunction(ECompareFunction v) { Assert(!Frozen()); _frontFaceStencilFunction = v; }

    bool TwoSidedStencilMode() const { return _twoSidedStencilMode; }
    void SetTwoSidedStencilMode(bool v) { Assert(!Frozen()); _twoSidedStencilMode = v; }

    EStencilOperation BackFaceStencilDepthBufferFail() const { return _backFaceStencilDepthBufferFail; }
    void SetBackFaceStencilDepthBufferFail(EStencilOperation v) { Assert(!Frozen()); _backFaceStencilDepthBufferFail = v; }

    EStencilOperation BackFaceStencilFail() const { return _backFaceStencilFail; }
    void SetBackFaceStencilFail(EStencilOperation v) { Assert(!Frozen()); _backFaceStencilFail = v; }

    ECompareFunction BackFaceStencilFunction() const { return _backFaceStencilFunction; }
    void SetBackFaceStencilFunction(ECompareFunction v) { Assert(!Frozen()); _backFaceStencilFunction = v; }

    EStencilOperation BackFaceStencilPass() const { return _backFaceStencilPass; }
    void SetBackFaceStencilPass(EStencilOperation v) { Assert(!Frozen()); _backFaceStencilPass = v; }

    static const FDepthStencilState *Default;
    static const FDepthStencilState *DepthRead;
    static const FDepthStencilState *None;

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

private:
    bool _depthBufferEnabled = true;
    bool _depthBufferWriteEnabled = true;
    ECompareFunction _depthBufferFunction = ECompareFunction::TLessEqual;

    u32 _rendererStencil = 0;
    bool _stencilEnabled = false;
    u32 _stencilReadMask = UINT8_MAX;
    u32 _stencilWriteMask = UINT8_MAX;

    EStencilOperation _frontFaceStencilDepthBufferFail = EStencilOperation::Keep;
    EStencilOperation _frontFaceStencilFail = EStencilOperation::Keep;
    EStencilOperation _frontFaceStencilPass = EStencilOperation::Keep;
    ECompareFunction _frontFaceStencilFunction = ECompareFunction::Always;

    bool _twoSidedStencilMode = false;

    EStencilOperation _backFaceStencilDepthBufferFail = EStencilOperation::Keep;
    EStencilOperation _backFaceStencilFail = EStencilOperation::Keep;
    ECompareFunction _backFaceStencilFunction = ECompareFunction::Always;
    EStencilOperation _backFaceStencilPass = EStencilOperation::Keep;

    PDeviceAPIDependantDepthStencilState _deviceAPIDependantState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantDepthStencilState : public TTypedDeviceAPIDependantEntity<FDepthStencilState> {
public:
    FDeviceAPIDependantDepthStencilState(IDeviceAPIEncapsulator *device, const FDepthStencilState *resource);
    virtual ~FDeviceAPIDependantDepthStencilState();

    virtual size_t VideoMemorySizeInBytes() const override { return 0; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
