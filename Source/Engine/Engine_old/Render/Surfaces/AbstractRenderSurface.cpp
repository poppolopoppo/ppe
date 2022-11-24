// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "AbstractRenderSurface.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"
#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include <stddef.h>
#include <stdint.h>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderSurfaceLock::FRenderSurfaceLock(const FAbstractRenderSurface *owner)
:   _state(0) {
    Assert(Owner() == owner); // deduced from this address
}
//----------------------------------------------------------------------------
FRenderSurfaceLock::~FRenderSurfaceLock() {
    Assert(!Owner()->InUse());
}
//----------------------------------------------------------------------------
const FAbstractRenderSurface *FRenderSurfaceLock::Owner() const {
    return reinterpret_cast<const FAbstractRenderSurface *>((const u8 *)this - offsetof(FAbstractRenderSurface, _lock) );
}
//----------------------------------------------------------------------------
void FRenderSurfaceLock::Acquire(
    const Graphics::FRenderTarget **pRenderTarget,
    const Graphics::FDepthStencil **pDepthStencil ) const {
    Assert(pRenderTarget);
    Assert(pDepthStencil);

    Owner()->AcquireFromLock_(this, pRenderTarget, pDepthStencil);
}
//----------------------------------------------------------------------------
void FRenderSurfaceLock::Release(
    Graphics::IDeviceAPIEncapsulator *device,
    PRenderSurfaceLock& plockSelf) {
    Assert(plockSelf == this);

    FAbstractRenderSurface *const owner = reinterpret_cast<FAbstractRenderSurface *>((u8 *)this - offsetof(FAbstractRenderSurface, _lock) );
    owner->Destroy(device, plockSelf);
}
//----------------------------------------------------------------------------
void FRenderSurfaceLock::Bind(Graphics::EShaderProgramType stage, size_t slot) {
    stage_field::InplaceBitOr(_state, 1 << size_t(stage));
    slots_field::InplaceBitOr(_state, 1 << size_t(slot));
}
//----------------------------------------------------------------------------
void FRenderSurfaceLock::Unbind(Graphics::IDeviceAPIContext *context) {
    if (0 == _state)
        return;

    const size_t stage = stage_field::Get(_state);
    const size_t slots = slots_field::Get(_state);
    Assert(stage);
    Assert(slots);

    for (size_t st = 0; st < size_t(Graphics::EShaderProgramType::__Count); ++st)
        if ((UINT64_C(1) << st) & stage)
            for (size_t sl = 0; (UINT64_C(1) << sl) <= slots; ++sl)
                if ((UINT64_C(1) << sl) & slots)
                    context->SetTexture(Graphics::EShaderProgramType(st), sl, nullptr);

    _state  = 0; // reset state
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAbstractRenderSurface::FAbstractRenderSurface(FString&& name)
:   _name(std::move(name))
,   _lock(this) {
    Assert(!_name.empty());
    AddRef(&_lock); // lifetime is manually handled to count actives references to this surface
}
//----------------------------------------------------------------------------
FAbstractRenderSurface::~FAbstractRenderSurface() {
    FRenderSurfaceLock *plock = &_lock; // remove the reference manually added in the ctor
    RemoveRef_AssertReachZero_NoDelete(plock); // if reference count is not 0 then someone is leaking a lock to this render surface
}
//----------------------------------------------------------------------------
void FAbstractRenderSurface::Prepare(Graphics::IDeviceAPIEncapsulator *device, PRenderSurfaceLock& plock) {
    Assert(!plock || &_lock == plock.get());

    if (1 == _lock.RefCount()) {
        Assert(!plock);
        Assert(!_renderTarget);
        Assert(!_depthStencil);

        CreateResources_(device, _renderTarget, _depthStencil);
    }

    plock.reset(&_lock);

    Assert(_lock.RefCount() > 1);
}
//----------------------------------------------------------------------------
void FAbstractRenderSurface::Destroy(Graphics::IDeviceAPIEncapsulator *device, PRenderSurfaceLock& plock) {
    Assert(&_lock == plock);
    Assert(_lock.RefCount() > 1);
    Assert(_renderTarget || _depthStencil);

    plock.reset(nullptr);

    if (1 == _lock.RefCount())
        DestroyResources_(device, _renderTarget, _depthStencil);
}
//----------------------------------------------------------------------------
void FAbstractRenderSurface::AcquireFromLock_(
    const FRenderSurfaceLock *plock,
    const Graphics::FRenderTarget **pRenderTarget,
    const Graphics::FDepthStencil **pDepthStencil ) const {
    Assert(plock == &_lock);
    Assert(_lock.RefCount() > 1);
    Assert(_renderTarget || _depthStencil);

    *pRenderTarget = _renderTarget.get();
    *pDepthStencil = _depthStencil.get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
