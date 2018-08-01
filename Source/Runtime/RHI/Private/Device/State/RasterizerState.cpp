#include "stdafx.h"

#include "RasterizerState.h"

#include "Device/DeviceEncapsulator.h"

#include "Meta/AlignedStorage.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRasterizerState::FRasterizerState()
:   FDeviceResource(EDeviceResourceType::FRasterizerState) {}
//----------------------------------------------------------------------------
FRasterizerState::~FRasterizerState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
bool FRasterizerState::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantState;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FRasterizerState::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantState.get();
}
//----------------------------------------------------------------------------
void FRasterizerState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateRasterizerState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void FRasterizerState::Destroy(IDeviceAPIEncapsulator *device) {
    Assert(_deviceAPIDependantState);

    device->DestroyRasterizerState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantRasterizerState::FDeviceAPIDependantRasterizerState(IDeviceAPIEncapsulator *device, const FRasterizerState *resource)
:   TTypedDeviceAPIDependantEntity<FRasterizerState>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantRasterizerState::~FDeviceAPIDependantRasterizerState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FRasterizerState *FRasterizerState::CullClockwise = nullptr;
const FRasterizerState *FRasterizerState::CullCounterClockwise = nullptr;
const FRasterizerState *FRasterizerState::CullNone = nullptr;
const FRasterizerState *FRasterizerState::Wireframe = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(FRasterizerState) GRasterizerState_CullClockwise;
    static POD_STORAGE(FRasterizerState) GRasterizerState_CullCounterClockwise;
    static POD_STORAGE(FRasterizerState) GRasterizerState_CullNone;
    static POD_STORAGE(FRasterizerState) GRasterizerState_Wireframe;
}
//----------------------------------------------------------------------------
void FRasterizerState::Start() {
    Assert(nullptr == CullClockwise);
    {
        FRasterizerState *const state = INPLACE_NEW(&GRasterizerState_CullClockwise, FRasterizerState)();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName(L"CullClockwise");
#endif
        state->SetCullMode(Graphics::ECullMode::CullClockwiseFace);
        state->Freeze();
        CullClockwise = state;
    }
    Assert(nullptr == CullCounterClockwise);
    {
        FRasterizerState *const state = INPLACE_NEW(&GRasterizerState_CullCounterClockwise, FRasterizerState)();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName(L"CullCounterClockwise");
#endif
        state->SetCullMode(Graphics::ECullMode::CullCounterClockwiseFace);
        state->Freeze();
        CullCounterClockwise = state;
    }
    Assert(nullptr == CullNone);
    {
        FRasterizerState *const state = INPLACE_NEW(&GRasterizerState_CullNone, FRasterizerState)();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName(L"CullNone");
#endif
        state->SetCullMode(Graphics::ECullMode::None);
        state->Freeze();
        CullNone = state;
    }
    Assert(nullptr == Wireframe);
    {
        FRasterizerState *const state = INPLACE_NEW(&GRasterizerState_Wireframe, FRasterizerState)();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName(L"Wireframe");
#endif
        state->SetFillMode(Graphics::EFillMode::WireFrame);
        state->SetCullMode(Graphics::ECullMode::None);
        state->Freeze();
        Wireframe = state;
    }
}
//----------------------------------------------------------------------------
void FRasterizerState::Shutdown() {
    Assert(nullptr != CullClockwise);
    {
        Assert((void *)CullClockwise == (void *)&GRasterizerState_CullClockwise);
        RemoveRef_AssertReachZero_NoDelete(CullClockwise);
        CullClockwise = nullptr;
    }
    Assert(nullptr != CullCounterClockwise);
    {
        Assert((void *)CullCounterClockwise == (void *)&GRasterizerState_CullCounterClockwise);
        RemoveRef_AssertReachZero_NoDelete(CullCounterClockwise);
        CullCounterClockwise = nullptr;
    }
    Assert(nullptr != CullNone);
    {
        Assert((void *)CullNone == (void *)&GRasterizerState_CullNone);
        RemoveRef_AssertReachZero_NoDelete(CullNone);
        CullNone = nullptr;
    }
    Assert(nullptr != Wireframe);
    {
        Assert((void *)Wireframe == (void *)&GRasterizerState_Wireframe);
        RemoveRef_AssertReachZero_NoDelete(Wireframe);
        Wireframe = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FRasterizerState::OnDeviceCreate(FDeviceEncapsulator *device) {
#define CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(_NAME) \
    remove_const(FRasterizerState::_NAME)->Create(device->Device())

    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullClockwise);
    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullCounterClockwise);
    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullNone);
    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(Wireframe);

#undef CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void FRasterizerState::OnDeviceDestroy(FDeviceEncapsulator *device) {
#define DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE(_NAME) \
    remove_const(FRasterizerState::_NAME)->Destroy(device->Device())

    DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullClockwise);
    DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullCounterClockwise);
    DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullNone);
    DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE(Wireframe);

#undef DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
