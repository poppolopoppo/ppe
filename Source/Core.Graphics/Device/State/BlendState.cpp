#include "stdafx.h"

#include "BlendState.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBlendState::FBlendState()
:   FDeviceResource(EDeviceResourceType::FBlendState)
,   _blendEnabled(false), _blendFactor(0) {}
//----------------------------------------------------------------------------
FBlendState::~FBlendState() {
    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
bool FBlendState::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantState;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FBlendState::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantState.get();
}
//----------------------------------------------------------------------------
void FBlendState::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantState);

    _deviceAPIDependantState = device->CreateBlendState(this);

    Assert(_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
void FBlendState::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantState);

    device->DestroyBlendState(this, _deviceAPIDependantState);

    Assert(!_deviceAPIDependantState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantBlendState::FDeviceAPIDependantBlendState(IDeviceAPIEncapsulator *device, const FBlendState *resource)
:   TTypedDeviceAPIDependantEntity<FBlendState>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantBlendState::~FDeviceAPIDependantBlendState() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FBlendState *FBlendState::Additive = nullptr;
const FBlendState *FBlendState::AlphaBlend = nullptr;
const FBlendState *FBlendState::NonPremultiplied = nullptr;
const FBlendState *FBlendState::Opaque = nullptr;
//----------------------------------------------------------------------------
namespace {
    static POD_STORAGE(FBlendState) gBlendState_Additive;
    static POD_STORAGE(FBlendState) gBlendState_AlphaBlend;
    static POD_STORAGE(FBlendState) gBlendState_NonPremultiplied;
    static POD_STORAGE(FBlendState) gBlendState_Opaque;
}
//----------------------------------------------------------------------------
void FBlendState::Start() {
    Assert(nullptr == Additive);
    {
        FBlendState *const state = new ((void *)&gBlendState_Additive) FBlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("Additive");
#endif
        state->SetBlendEnabled(true);
        state->SetColorSourceBlend(EBlend::SourceAlpha);
        state->SetAlphaSourceBlend(EBlend::SourceAlpha);
        state->SetColorDestinationBlend(EBlend::One);
        state->SetAlphaDestinationBlend(EBlend::One);
        state->Freeze();
        Additive = state;
    }
    Assert(nullptr == AlphaBlend);
    {
        FBlendState *const state = new ((void *)&gBlendState_AlphaBlend) FBlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("AlphaBlend");
#endif
        state->SetBlendEnabled(true);
        state->SetColorSourceBlend(EBlend::One);
        state->SetAlphaSourceBlend(EBlend::One);
        state->SetColorDestinationBlend(EBlend::InverseSourceAlpha);
        state->SetAlphaDestinationBlend(EBlend::InverseSourceAlpha);
        state->Freeze();
        AlphaBlend = state;
    }
    Assert(nullptr == NonPremultiplied);
    {
        FBlendState *const state = new ((void *)&gBlendState_NonPremultiplied) FBlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("NonPremultiplied");
#endif
        state->SetBlendEnabled(true);
        state->SetColorSourceBlend(EBlend::SourceAlpha);
        state->SetAlphaSourceBlend(EBlend::SourceAlpha);
        state->SetColorDestinationBlend(EBlend::InverseSourceAlpha);
        state->SetAlphaDestinationBlend(EBlend::InverseSourceAlpha);
        state->Freeze();
        NonPremultiplied = state;
    }
    Assert(nullptr == Opaque);
    {
        FBlendState *const state = new ((void *)&gBlendState_Opaque) FBlendState();
        AddRef(state);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        state->SetResourceName("Opaque");
#endif
        state->SetColorSourceBlend(EBlend::One);
        state->SetAlphaSourceBlend(EBlend::One);
        state->SetColorDestinationBlend(EBlend::Zero);
        state->SetAlphaDestinationBlend(EBlend::Zero);
        state->Freeze();
        Opaque = state;
    }
}
//----------------------------------------------------------------------------
void FBlendState::Shutdown() {
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
void FBlendState::OnDeviceCreate(FDeviceEncapsulator *device) {
#define CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(_NAME) \
    remove_const(FBlendState::_NAME)->Create(device->Device())

    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(Additive);
    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(AlphaBlend);
    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(NonPremultiplied);
    CREATEWDEVICE_BLENDSTATE_BUILTINTYPE(Opaque);

#undef CREATEWDEVICE_BLENDSTATE_BUILTINTYPE
}
//----------------------------------------------------------------------------
void FBlendState::OnDeviceDestroy(FDeviceEncapsulator *device) {
#define DESTROYWDEVICE_BLENDSTATE_BUILTINTYPE(_NAME) \
    remove_const(FBlendState::_NAME)->Destroy(device->Device())

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
