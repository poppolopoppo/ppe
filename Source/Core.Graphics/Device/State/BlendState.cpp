#include "stdafx.h"

#include "BlendState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BlendState::BlendState() : _blendEnabled(false), _blendFactor(0) {}
//----------------------------------------------------------------------------
BlendState::~BlendState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void BlendState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateBlendState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void BlendState::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantState);

    device->DestroyBlendState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantBlendState::DeviceAPIDependantBlendState(IDeviceAPIEncapsulator *device, BlendState *owner)
:   DeviceAPIDependantEntity(device)
,   _owner(owner) {
    Assert(owner);
}
//----------------------------------------------------------------------------
DeviceAPIDependantBlendState::~DeviceAPIDependantBlendState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const BlendState *BlendState::Additive = nullptr;
const BlendState *BlendState::AlphaBlend = nullptr;
const BlendState *BlendState::NonPremultiplied = nullptr;
const BlendState *BlendState::Opaque = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(BlendState) gBlendState_Additive;
    static POD_STORAGE(BlendState) gBlendState_AlphaBlend;
    static POD_STORAGE(BlendState) gBlendState_NonPremultiplied;
    static POD_STORAGE(BlendState) gBlendState_Opaque;
}
//----------------------------------------------------------------------------
void BlendState::Start() {
    Assert(nullptr == Additive);
    {
        BlendState *const state = new ((void *)&gBlendState_Additive) BlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("Additive");
#endif
        state->SetBlendEnabled(true);
        state->SetColorSourceBlend(Blend::SourceAlpha);
        state->SetAlphaSourceBlend(Blend::SourceAlpha);
        state->SetColorDestinationBlend(Blend::One);
        state->SetAlphaDestinationBlend(Blend::One);
        state->Freeze();
        Additive = state;
    }
    Assert(nullptr == AlphaBlend);
    {
        BlendState *const state = new ((void *)&gBlendState_AlphaBlend) BlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("AlphaBlend");
#endif
        state->SetBlendEnabled(true);
        state->SetColorSourceBlend(Blend::One);
        state->SetAlphaSourceBlend(Blend::One);
        state->SetColorDestinationBlend(Blend::InverseSourceAlpha);
        state->SetAlphaDestinationBlend(Blend::InverseSourceAlpha);
        state->Freeze();
        AlphaBlend = state;
    }
    Assert(nullptr == NonPremultiplied);
    {
        BlendState *const state = new ((void *)&gBlendState_NonPremultiplied) BlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("NonPremultiplied");
#endif
        state->SetBlendEnabled(true);
        state->SetColorSourceBlend(Blend::SourceAlpha);
        state->SetAlphaSourceBlend(Blend::SourceAlpha);
        state->SetColorDestinationBlend(Blend::InverseSourceAlpha);
        state->SetAlphaDestinationBlend(Blend::InverseSourceAlpha);
        state->Freeze();
        NonPremultiplied = state;
    }
    Assert(nullptr == Opaque);
    {
        BlendState *const state = new ((void *)&gBlendState_Opaque) BlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("Opaque");
#endif
        state->SetColorSourceBlend(Blend::One);
        state->SetAlphaSourceBlend(Blend::One);
        state->SetColorDestinationBlend(Blend::Zero);
        state->SetAlphaDestinationBlend(Blend::Zero);
        state->Freeze();
        Opaque = state;
    }
}
//----------------------------------------------------------------------------
void BlendState::Shutdown() {
    Assert(nullptr != Additive);
    {
        Assert((void *)Additive == (void *)&gBlendState_Additive);
        RemoveRef_AssertReachZero_NoDelete(Additive);
        Additive = nullptr;
    }
    Assert(nullptr != AlphaBlend);
    {
        Assert((void *)AlphaBlend == (void *)&gBlendState_AlphaBlend);
        RemoveRef_AssertReachZero_NoDelete(AlphaBlend);
        AlphaBlend = nullptr;
    }
    Assert(nullptr != NonPremultiplied);
    {
        Assert((void *)NonPremultiplied == (void *)&gBlendState_NonPremultiplied);
        RemoveRef_AssertReachZero_NoDelete(NonPremultiplied);
        NonPremultiplied = nullptr;
    }
    Assert(nullptr != Opaque);
    {
        Assert((void *)Opaque == (void *)&gBlendState_Opaque);
        RemoveRef_AssertReachZero_NoDelete(Opaque);
        Opaque = nullptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void BlendState::OnDeviceCreate(DeviceEncapsulator *device) {
#define CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(_NAME) \
    const_cast<BlendState *>(BlendState::_NAME)->Create(device->Device())

    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(Additive);
    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(AlphaBlend);
    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(NonPremultiplied);
    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(Opaque);

#undef CREATEWDEVICE_BLENDSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void BlendState::OnDeviceDestroy(DeviceEncapsulator *device) {
#define DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE(_NAME) \
    const_cast<BlendState *>(BlendState::_NAME)->Destroy(device->Device())

    DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE(Additive);
    DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE(AlphaBlend);
    DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE(NonPremultiplied);
    DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE(Opaque);

#undef DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
