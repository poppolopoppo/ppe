#include "stdafx.h"

#include "SamplerState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSamplerState::FSamplerState()
:   FDeviceResource(EDeviceResourceType::FSamplerState) {}
//----------------------------------------------------------------------------
FSamplerState::~FSamplerState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
bool FSamplerState::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantState;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FSamplerState::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantState.get();
}
//----------------------------------------------------------------------------
void FSamplerState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateSamplerState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void FSamplerState::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantState);

    device->DestroySamplerState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantSamplerState::FDeviceAPIDependantSamplerState(IDeviceAPIEncapsulator *device, const FSamplerState *resource)
:   TTypedDeviceAPIDependantEntity<FSamplerState>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantSamplerState::~FDeviceAPIDependantSamplerState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FSamplerState *FSamplerState::AnisotropicClamp = nullptr;
const FSamplerState *FSamplerState::AnisotropicWrap = nullptr;
const FSamplerState *FSamplerState::LinearClamp = nullptr;
const FSamplerState *FSamplerState::LinearWrap = nullptr;
const FSamplerState *FSamplerState::PointClamp = nullptr;
const FSamplerState *FSamplerState::PointWrap = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(FSamplerState) GSamplerState_AnisotropicClamp;
    static POD_STORAGE(FSamplerState) GSamplerState_AnisotropicWrap;
    static POD_STORAGE(FSamplerState) GSamplerState_LinearClamp;
    static POD_STORAGE(FSamplerState) GSamplerState_LinearWrap;
    static POD_STORAGE(FSamplerState) GSamplerState_PointClamp;
    static POD_STORAGE(FSamplerState) GSamplerState_PointWrap;
}
//----------------------------------------------------------------------------
void FSamplerState::Start() {
    Assert(nullptr == AnisotropicClamp);
    {
        FSamplerState *const state = new ((void *)&GSamplerState_AnisotropicClamp) FSamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("AnisotropicClamp");
#endif
        state->SetAddressU(ETextureAddressMode::Clamp);
        state->SetAddressV(ETextureAddressMode::Clamp);
        state->SetAddressW(ETextureAddressMode::Clamp);
        state->SetFilter(ETextureFilter::Anisotropic);
        state->Freeze();
        AnisotropicClamp = state;
    }
    Assert(nullptr == AnisotropicWrap);
    {
        FSamplerState *const state = new ((void *)&GSamplerState_AnisotropicWrap) FSamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("AnisotropicWrap");
#endif
        state->SetAddressU(ETextureAddressMode::Wrap);
        state->SetAddressV(ETextureAddressMode::Wrap);
        state->SetAddressW(ETextureAddressMode::Wrap);
        state->SetFilter(ETextureFilter::Anisotropic);
        state->Freeze();
        AnisotropicWrap = state;
    }
    Assert(nullptr == LinearClamp);
    {
        FSamplerState *const state = new ((void *)&GSamplerState_LinearClamp) FSamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("LinearClamp");
#endif
        state->SetAddressU(ETextureAddressMode::Clamp);
        state->SetAddressV(ETextureAddressMode::Clamp);
        state->SetAddressW(ETextureAddressMode::Clamp);
        state->SetFilter(ETextureFilter::Linear);
        state->Freeze();
        LinearClamp = state;
    }
    Assert(nullptr == LinearWrap);
    {
        FSamplerState *const state = new ((void *)&GSamplerState_LinearWrap) FSamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("LinearWrap");
#endif
        state->SetAddressU(ETextureAddressMode::Wrap);
        state->SetAddressV(ETextureAddressMode::Wrap);
        state->SetAddressW(ETextureAddressMode::Wrap);
        state->SetFilter(ETextureFilter::Linear);
        state->Freeze();
        LinearWrap = state;
    }
    Assert(nullptr == PointClamp);
    {
        FSamplerState *const state = new ((void *)&GSamplerState_PointClamp) FSamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("PointClamp");
#endif
        state->SetAddressU(ETextureAddressMode::Clamp);
        state->SetAddressV(ETextureAddressMode::Clamp);
        state->SetAddressW(ETextureAddressMode::Clamp);
        state->SetFilter(ETextureFilter::Point);
        state->Freeze();
        PointClamp = state;
    }
    Assert(nullptr == PointWrap);
    {
        FSamplerState *const state = new ((void *)&GSamplerState_PointWrap) FSamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("PointWrap");
#endif
        state->SetAddressU(ETextureAddressMode::Wrap);
        state->SetAddressV(ETextureAddressMode::Wrap);
        state->SetAddressW(ETextureAddressMode::Wrap);
        state->SetFilter(ETextureFilter::Point);
        state->Freeze();
        PointWrap = state;
    }
}
//----------------------------------------------------------------------------
void FSamplerState::Shutdown() {
    Assert(nullptr != AnisotropicClamp);
    {
        Assert((void *)AnisotropicClamp == (void *)&GSamplerState_AnisotropicClamp);
        RemoveRef_AssertReachZero_NoDelete(AnisotropicClamp);
        AnisotropicClamp = nullptr;
    }
    Assert(nullptr != AnisotropicWrap);
    {
        Assert((void *)AnisotropicWrap == (void *)&GSamplerState_AnisotropicWrap);
        RemoveRef_AssertReachZero_NoDelete(AnisotropicWrap);
        AnisotropicWrap = nullptr;
    }
    Assert(nullptr != LinearClamp);
    {
        Assert((void *)LinearClamp == (void *)&GSamplerState_LinearClamp);
        RemoveRef_AssertReachZero_NoDelete(LinearClamp);
        LinearClamp = nullptr;
    }
    Assert(nullptr != LinearWrap);
    {
        Assert((void *)LinearWrap == (void *)&GSamplerState_LinearWrap);
        RemoveRef_AssertReachZero_NoDelete(LinearWrap);
        LinearWrap = nullptr;
    }
    Assert(nullptr != PointClamp);
    {
        Assert((void *)PointClamp == (void *)&GSamplerState_PointClamp);
        RemoveRef_AssertReachZero_NoDelete(PointClamp);
        PointClamp = nullptr;
    }
    Assert(nullptr != PointWrap);
    {
        Assert((void *)PointWrap == (void *)&GSamplerState_PointWrap);
        RemoveRef_AssertReachZero_NoDelete(PointWrap);
        PointWrap = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FSamplerState::OnDeviceCreate(FDeviceEncapsulator *device) {
#define CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(_NAME) \
    remove_const(FSamplerState::_NAME)->Create(device->Device())

    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(AnisotropicClamp);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(AnisotropicWrap);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(LinearClamp);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(LinearWrap);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(PointClamp);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(PointWrap);

#undef CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void FSamplerState::OnDeviceDestroy(FDeviceEncapsulator *device) {
#define DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(_NAME) \
    remove_const(FSamplerState::_NAME)->Destroy(device->Device())

    DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(AnisotropicClamp);
    DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(AnisotropicWrap);
    DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(LinearClamp);
    DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(LinearWrap);
    DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(PointClamp);
    DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(PointWrap);

#undef DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
