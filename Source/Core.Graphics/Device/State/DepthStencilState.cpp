#include "stdafx.h"

#include "DepthStencilState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDepthStencilState::FDepthStencilState()
:   FDeviceResource(EDeviceResourceType::FDepthStencilState) {}
//----------------------------------------------------------------------------
FDepthStencilState::~FDepthStencilState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
bool FDepthStencilState::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantState;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FDepthStencilState::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantState.get();
}
//----------------------------------------------------------------------------
void FDepthStencilState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateDepthStencilState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void FDepthStencilState::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantState);

    device->DestroyDepthStencilState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencilState::FDeviceAPIDependantDepthStencilState(IDeviceAPIEncapsulator *device, const FDepthStencilState *resource)
:   TTypedDeviceAPIDependantEntity<FDepthStencilState>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantDepthStencilState::~FDeviceAPIDependantDepthStencilState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FDepthStencilState *FDepthStencilState::Default = nullptr;
const FDepthStencilState *FDepthStencilState::DepthRead = nullptr;
const FDepthStencilState *FDepthStencilState::None = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(FDepthStencilState) gDepthStencilState_Default;
    static POD_STORAGE(FDepthStencilState) gDepthStencilState_DepthRead;
    static POD_STORAGE(FDepthStencilState) gDepthStencilState_None;
}
//----------------------------------------------------------------------------
void FDepthStencilState::Start() {
    Assert(nullptr == Default);
    {
        FDepthStencilState *const state = new ((void *)&gDepthStencilState_Default) FDepthStencilState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("Default");
#endif
        state->SetDepthBufferEnabled(true);
        state->SetDepthBufferWriteEnabled(true);
        state->Freeze();
        Default = state;
    }
    Assert(nullptr == DepthRead);
    {
        FDepthStencilState *const state = new ((void *)&gDepthStencilState_DepthRead) FDepthStencilState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("DepthRead");
#endif
        state->SetDepthBufferEnabled(true);
        state->SetDepthBufferWriteEnabled(false);
        state->Freeze();
        DepthRead = state;
    }
    Assert(nullptr == None);
    {
        FDepthStencilState *const state = new ((void *)&gDepthStencilState_None) FDepthStencilState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("None");
#endif
        state->SetDepthBufferEnabled(false);
        state->SetDepthBufferWriteEnabled(false);
        state->Freeze();
        None = state;
    }
}
//----------------------------------------------------------------------------
void FDepthStencilState::Shutdown() {
    Assert(nullptr != Default);
    {
        Assert((void *)Default == (void *)&gDepthStencilState_Default);
        RemoveRef_AssertReachZero_NoDelete(Default);
        Default = nullptr;
    }
    Assert(nullptr != DepthRead);
    {
        Assert((void *)DepthRead == (void *)&gDepthStencilState_DepthRead);
        RemoveRef_AssertReachZero_NoDelete(DepthRead);
        DepthRead = nullptr;
    }
    Assert(nullptr != None);
    {
        Assert((void *)None == (void *)&gDepthStencilState_None);
        RemoveRef_AssertReachZero_NoDelete(None);
        None = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FDepthStencilState::OnDeviceCreate(FDeviceEncapsulator *device) {
#define CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(_NAME) \
    remove_const(FDepthStencilState::_NAME)->Create(device->Device())

    CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(Default);
    CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(DepthRead);
    CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(None);

#undef CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void FDepthStencilState::OnDeviceDestroy(FDeviceEncapsulator *device) {
#define DESTROYWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(_NAME) \
    remove_const(FDepthStencilState::_NAME)->Destroy(device->Device())

    DESTROYWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(Default);
    DESTROYWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(DepthRead);
    DESTROYWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(None);

#undef DESTROYWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
