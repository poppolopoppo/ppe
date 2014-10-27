#include "stdafx.h"

#include "SamplerState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SamplerState::SamplerState() {}
//----------------------------------------------------------------------------
SamplerState::~SamplerState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void SamplerState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateSamplerState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void SamplerState::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantState);

    device->DestroySamplerState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState::DeviceAPIDependantSamplerState(IDeviceAPIEncapsulator *device, SamplerState *owner)
:   DeviceAPIDependantEntity(device)
,   _owner(owner) {
    Assert(owner);
}
//----------------------------------------------------------------------------
DeviceAPIDependantSamplerState::~DeviceAPIDependantSamplerState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const SamplerState *SamplerState::AnisotropicClamp = nullptr;
const SamplerState *SamplerState::AnisotropicWrap = nullptr;
const SamplerState *SamplerState::LinearClamp = nullptr;
const SamplerState *SamplerState::LinearWrap = nullptr;
const SamplerState *SamplerState::PointClamp = nullptr;
const SamplerState *SamplerState::PointWrap = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(SamplerState) gSamplerState_AnisotropicClamp;
    static POD_STORAGE(SamplerState) gSamplerState_AnisotropicWrap;
    static POD_STORAGE(SamplerState) gSamplerState_LinearClamp;
    static POD_STORAGE(SamplerState) gSamplerState_LinearWrap;
    static POD_STORAGE(SamplerState) gSamplerState_PointClamp;
    static POD_STORAGE(SamplerState) gSamplerState_PointWrap;
}
//----------------------------------------------------------------------------
void SamplerState::Start() {
    Assert(nullptr == AnisotropicClamp);
    {
        SamplerState *const state = new ((void *)&gSamplerState_AnisotropicClamp) SamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("AnisotropicClamp");
#endif
        state->SetAddressU(TextureAddressMode::Clamp);
        state->SetAddressV(TextureAddressMode::Clamp);
        state->SetAddressW(TextureAddressMode::Clamp);
        state->SetFilter(TextureFilter::Anisotropic);
        state->Freeze();
        AnisotropicClamp = state;
    }
    Assert(nullptr == AnisotropicWrap);
    {
        SamplerState *const state = new ((void *)&gSamplerState_AnisotropicWrap) SamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("AnisotropicWrap");
#endif
        state->SetAddressU(TextureAddressMode::Wrap);
        state->SetAddressV(TextureAddressMode::Wrap);
        state->SetAddressW(TextureAddressMode::Wrap);
        state->SetFilter(TextureFilter::Anisotropic);
        state->Freeze();
        AnisotropicWrap = state;
    }
    Assert(nullptr == LinearClamp);
    {
        SamplerState *const state = new ((void *)&gSamplerState_LinearClamp) SamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("LinearClamp");
#endif
        state->SetAddressU(TextureAddressMode::Clamp);
        state->SetAddressV(TextureAddressMode::Clamp);
        state->SetAddressW(TextureAddressMode::Clamp);
        state->SetFilter(TextureFilter::Linear);
        state->Freeze();
        LinearClamp = state;
    }
    Assert(nullptr == LinearWrap);
    {
        SamplerState *const state = new ((void *)&gSamplerState_LinearWrap) SamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("LinearWrap");
#endif
        state->SetAddressU(TextureAddressMode::Wrap);
        state->SetAddressV(TextureAddressMode::Wrap);
        state->SetAddressW(TextureAddressMode::Wrap);
        state->SetFilter(TextureFilter::Linear);
        state->Freeze();
        LinearWrap = state;
    }
    Assert(nullptr == PointClamp);
    {
        SamplerState *const state = new ((void *)&gSamplerState_PointClamp) SamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("PointClamp");
#endif
        state->SetAddressU(TextureAddressMode::Clamp);
        state->SetAddressV(TextureAddressMode::Clamp);
        state->SetAddressW(TextureAddressMode::Clamp);
        state->SetFilter(TextureFilter::Point);
        state->Freeze();
        PointClamp = state;
    }
    Assert(nullptr == PointWrap);
    {
        SamplerState *const state = new ((void *)&gSamplerState_PointWrap) SamplerState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("PointWrap");
#endif
        state->SetAddressU(TextureAddressMode::Wrap);
        state->SetAddressV(TextureAddressMode::Wrap);
        state->SetAddressW(TextureAddressMode::Wrap);
        state->SetFilter(TextureFilter::Point);
        state->Freeze();
        PointWrap = state;
    }
}
//----------------------------------------------------------------------------
void SamplerState::Shutdown() {
    Assert(nullptr != AnisotropicClamp);
    {
        Assert((void *)AnisotropicClamp == (void *)&gSamplerState_AnisotropicClamp);
        RemoveRef_AssertReachZero_NoDelete(AnisotropicClamp);
        AnisotropicClamp = nullptr;
    }
    Assert(nullptr != AnisotropicWrap);
    {
        Assert((void *)AnisotropicWrap == (void *)&gSamplerState_AnisotropicWrap);
        RemoveRef_AssertReachZero_NoDelete(AnisotropicWrap);
        AnisotropicWrap = nullptr;
    }
    Assert(nullptr != LinearClamp);
    {
        Assert((void *)LinearClamp == (void *)&gSamplerState_LinearClamp);
        RemoveRef_AssertReachZero_NoDelete(LinearClamp);
        LinearClamp = nullptr;
    }
    Assert(nullptr != LinearWrap);
    {
        Assert((void *)LinearWrap == (void *)&gSamplerState_LinearWrap);
        RemoveRef_AssertReachZero_NoDelete(LinearWrap);
        LinearWrap = nullptr;
    }
    Assert(nullptr != PointClamp);
    {
        Assert((void *)PointClamp == (void *)&gSamplerState_PointClamp);
        RemoveRef_AssertReachZero_NoDelete(PointClamp);
        PointClamp = nullptr;
    }
    Assert(nullptr != PointWrap);
    {
        Assert((void *)PointWrap == (void *)&gSamplerState_PointWrap);
        RemoveRef_AssertReachZero_NoDelete(PointWrap);
        PointWrap = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SamplerState::OnDeviceCreate(DeviceEncapsulator *device) {
#define CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(_NAME) \
    const_cast<SamplerState *>(SamplerState::_NAME)->Create(device->Device())

    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(AnisotropicClamp);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(AnisotropicWrap);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(LinearClamp);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(LinearWrap);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(PointClamp);
    CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE(PointWrap);

#undef CREATEWDEVICE_SAMPLERSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void SamplerState::OnDeviceDestroy(DeviceEncapsulator *device) {
#define DESTROYWDEVICE_SAMPLERSTATE_BUILTINTYPE(_NAME) \
    const_cast<SamplerState *>(SamplerState::_NAME)->Destroy(device->Device())

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
