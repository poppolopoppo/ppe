#include "stdafx.h"

#include "AbstractRenderSurface.h"

#include "Core.Graphics/DepthStencil.h"
#include "Core.Graphics/RenderTarget.h"
#include "Core.Graphics/ShaderProgram.h"

#include <stddef.h>
#include <stdint.h>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RenderSurfaceLock::RenderSurfaceLock(const AbstractRenderSurface *owner)
:   _state(0) {
    Assert(Owner() == owner); // deduced from this address
}
//----------------------------------------------------------------------------
RenderSurfaceLock::~RenderSurfaceLock() {
    Assert(!Owner()->InUse());
}
//----------------------------------------------------------------------------
const AbstractRenderSurface *RenderSurfaceLock::Owner() const {
    return reinterpret_cast<const AbstractRenderSurface *>((const u8 *)this - offsetof(AbstractRenderSurface, _lock) );
}
//----------------------------------------------------------------------------
void RenderSurfaceLock::Acquire(
    const Graphics::RenderTarget **pRenderTarget,
    const Graphics::DepthStencil **pDepthStencil ) const {
    Assert(pRenderTarget);
    Assert(pDepthStencil);

    Owner()->AcquireFromLock_(this, pRenderTarget, pDepthStencil);
}
//----------------------------------------------------------------------------
void RenderSurfaceLock::Release(
    Graphics::IDeviceAPIEncapsulator *device,
    PRenderSurfaceLock& plockSelf) {
    Assert(plockSelf == this);

    AbstractRenderSurface *const owner = reinterpret_cast<AbstractRenderSurface *>((u8 *)this - offsetof(AbstractRenderSurface, _lock) );
    owner->Destroy(device, plockSelf);
}
//----------------------------------------------------------------------------
void RenderSurfaceLock::Bind(Graphics::ShaderProgramType stage, size_t slot) {
    stage_field::InplaceBitOr(_state, 1 << size_t(stage));
    slots_field::InplaceBitOr(_state, 1 << size_t(slot));
}
//----------------------------------------------------------------------------
void RenderSurfaceLock::Unbind(Graphics::IDeviceAPIContextEncapsulator *context) {
    if (0 == _state)
        return;

    const size_t stage = stage_field::Get(_state);
    const size_t slots = slots_field::Get(_state);
    Assert(stage);
    Assert(slots);

    for (size_t st = 0; st < size_t(Graphics::ShaderProgramType::__Count); ++st)
        if ((UINT64_C(1) << st) & stage)
            for (size_t sl = 0; (UINT64_C(1) << sl) <= slots; ++sl)
                if ((UINT64_C(1) << sl) & slots)
                    context->SetTexture(Graphics::ShaderProgramType(st), sl, nullptr);

    _state  = 0; // reset state
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AbstractRenderSurface::AbstractRenderSurface(String&& name)
:   _name(std::move(name))
,   _lock(this) {
    Assert(!_name.empty());
    AddRef(&_lock); // lifetime is manually handled to count actives references to this surface
}
//----------------------------------------------------------------------------
AbstractRenderSurface::~AbstractRenderSurface() {
    RenderSurfaceLock *plock = &_lock; // remove the reference manually added in the ctor
    RemoveRef_AssertReachZero_NoDelete(plock); // if reference count is not 0 then someone is leaking a lock to this render surface
}
//----------------------------------------------------------------------------
void AbstractRenderSurface::Prepare(Graphics::IDeviceAPIEncapsulator *device, PRenderSurfaceLock& plock) {
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
void AbstractRenderSurface::Destroy(Graphics::IDeviceAPIEncapsulator *device, PRenderSurfaceLock& plock) {
    Assert(&_lock == plock);
    Assert(_lock.RefCount() > 1);
    Assert(_renderTarget || _depthStencil);

    plock.reset(nullptr);

    if (1 == _lock.RefCount())
        DestroyResources_(device, _renderTarget, _depthStencil);
}
//----------------------------------------------------------------------------
void AbstractRenderSurface::AcquireFromLock_(
    const RenderSurfaceLock *plock,
    const Graphics::RenderTarget **pRenderTarget,
    const Graphics::DepthStencil **pDepthStencil ) const {
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
