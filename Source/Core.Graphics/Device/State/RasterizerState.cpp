#include "stdafx.h"

#include "RasterizerState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RasterizerState::RasterizerState()
:   DeviceResource(DeviceResourceType::RasterizerState) {}
//----------------------------------------------------------------------------
RasterizerState::~RasterizerState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
bool RasterizerState::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantState;
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity *RasterizerState::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantState.get();
}
//----------------------------------------------------------------------------
void RasterizerState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateRasterizerState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void RasterizerState::Destroy(IDeviceAPIEncapsulator *device) {
    Assert(_deviceAPIDependantState);

    device->DestroyRasterizerState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState::DeviceAPIDependantRasterizerState(IDeviceAPIEncapsulator *device, const RasterizerState *resource)
:   TypedDeviceAPIDependantEntity<RasterizerState>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
DeviceAPIDependantRasterizerState::~DeviceAPIDependantRasterizerState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const RasterizerState *RasterizerState::CullClockwise = nullptr;
const RasterizerState *RasterizerState::CullCounterClockwise = nullptr;
const RasterizerState *RasterizerState::CullNone = nullptr;
const RasterizerState *RasterizerState::Wireframe = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(RasterizerState) gRasterizerState_CullClockwise;
    static POD_STORAGE(RasterizerState) gRasterizerState_CullCounterClockwise;
    static POD_STORAGE(RasterizerState) gRasterizerState_CullNone;
    static POD_STORAGE(RasterizerState) gRasterizerState_Wireframe;
}
//----------------------------------------------------------------------------
void RasterizerState::Start() {
    Assert(nullptr == CullClockwise);
    {
        RasterizerState *const state = new ((void *)&gRasterizerState_CullClockwise) RasterizerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("CullClockwise");
#endif
        state->SetCullMode(Graphics::CullMode::CullClockwiseFace);
        state->Freeze();
        CullClockwise = state;
    }
    Assert(nullptr == CullCounterClockwise);
    {
        RasterizerState *const state = new ((void *)&gRasterizerState_CullCounterClockwise) RasterizerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("CullCounterClockwise");
#endif
        state->SetCullMode(Graphics::CullMode::CullCounterClockwiseFace);
        state->Freeze();
        CullCounterClockwise = state;
    }
    Assert(nullptr == CullNone);
    {
        RasterizerState *const state = new ((void *)&gRasterizerState_CullNone) RasterizerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("CullNone");
#endif
        state->SetCullMode(Graphics::CullMode::None);
        state->Freeze();
        CullNone = state;
    }
    Assert(nullptr == Wireframe);
    {
        RasterizerState *const state = new ((void *)&gRasterizerState_Wireframe) RasterizerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("Wireframe");
#endif
        state->SetFillMode(Graphics::FillMode::WireFrame);
        state->SetCullMode(Graphics::CullMode::None);
        state->Freeze();
        Wireframe = state;
    }
}
//----------------------------------------------------------------------------
void RasterizerState::Shutdown() {
    Assert(nullptr != CullClockwise);
    {
        Assert((void *)CullClockwise == (void *)&gRasterizerState_CullClockwise);
        RemoveRef_AssertReachZero_NoDelete(CullClockwise);
        CullClockwise = nullptr;
    }
    Assert(nullptr != CullCounterClockwise);
    {
        Assert((void *)CullCounterClockwise == (void *)&gRasterizerState_CullCounterClockwise);
        RemoveRef_AssertReachZero_NoDelete(CullCounterClockwise);
        CullCounterClockwise = nullptr;
    }
    Assert(nullptr != CullNone);
    {
        Assert((void *)CullNone == (void *)&gRasterizerState_CullNone);
        RemoveRef_AssertReachZero_NoDelete(CullNone);
        CullNone = nullptr;
    }
    Assert(nullptr != Wireframe);
    {
        Assert((void *)Wireframe == (void *)&gRasterizerState_Wireframe);
        RemoveRef_AssertReachZero_NoDelete(Wireframe);
        Wireframe = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RasterizerState::OnDeviceCreate(DeviceEncapsulator *device) {
#define CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(_NAME) \
    remove_const(RasterizerState::_NAME)->Create(device->Device())

    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullClockwise);
    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullCounterClockwise);
    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(CullNone);
    CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE(Wireframe);

#undef CREATEWDEVICE_RASTERIZERSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void RasterizerState::OnDeviceDestroy(DeviceEncapsulator *device) {
#define DESTROYWDEVICE_RASTERIZERSTATE_BUILTINTYPE(_NAME) \
    remove_const(RasterizerState::_NAME)->Destroy(device->Device())

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
} //!namespace Core
