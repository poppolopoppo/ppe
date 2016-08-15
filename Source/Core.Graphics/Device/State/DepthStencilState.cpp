#include "stdafx.h"

#include "DepthStencilState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DepthStencilState::DepthStencilState()
:   DeviceResource(DeviceResourceType::DepthStencilState) {}
//----------------------------------------------------------------------------
DepthStencilState::~DepthStencilState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
bool DepthStencilState::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantState;
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity *DepthStencilState::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantState.get();
}
//----------------------------------------------------------------------------
void DepthStencilState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateDepthStencilState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void DepthStencilState::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantState);

    device->DestroyDepthStencilState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState::DeviceAPIDependantDepthStencilState(IDeviceAPIEncapsulator *device, const DepthStencilState *resource)
:   TypedDeviceAPIDependantEntity<DepthStencilState>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
DeviceAPIDependantDepthStencilState::~DeviceAPIDependantDepthStencilState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const DepthStencilState *DepthStencilState::Default = nullptr;
const DepthStencilState *DepthStencilState::DepthRead = nullptr;
const DepthStencilState *DepthStencilState::None = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(DepthStencilState) gDepthStencilState_Default;
    static POD_STORAGE(DepthStencilState) gDepthStencilState_DepthRead;
    static POD_STORAGE(DepthStencilState) gDepthStencilState_None;
}
//----------------------------------------------------------------------------
void DepthStencilState::Start() {
    Assert(nullptr == Default);
    {
        DepthStencilState *const state = new ((void *)&gDepthStencilState_Default) DepthStencilState();
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
        DepthStencilState *const state = new ((void *)&gDepthStencilState_DepthRead) DepthStencilState();
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
        DepthStencilState *const state = new ((void *)&gDepthStencilState_None) DepthStencilState();
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
void DepthStencilState::Shutdown() {
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
void DepthStencilState::OnDeviceCreate(DeviceEncapsulator *device) {
#define CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(_NAME) \
    remove_const(DepthStencilState::_NAME)->Create(device->Device())

    CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(Default);
    CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(DepthRead);
    CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(None);

#undef CREATEWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void DepthStencilState::OnDeviceDestroy(DeviceEncapsulator *device) {
#define DESTROYWDEVICE_DEPTHSTENCILSTATE_BUILTINTYPE(_NAME) \
    remove_const(DepthStencilState::_NAME)->Destroy(device->Device())

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
