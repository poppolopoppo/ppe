#pragma once

#include "Core.Engine/Engine.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(DepthStencil);
FWD_REFPTR(RenderTarget);
enum class EShaderProgramType;
class FSurfaceFormat;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractRenderSurface;
FWD_REFPTR(RenderSurfaceLock);
//----------------------------------------------------------------------------
// Same lifetime than created resources
class FRenderSurfaceLock : public FRefCountable {
private:
    friend class FAbstractRenderSurface;
    FRenderSurfaceLock(const FAbstractRenderSurface *owner);

public:
    ~FRenderSurfaceLock();

    FRenderSurfaceLock(FRenderSurfaceLock&& ) = delete;
    FRenderSurfaceLock& operator =(FRenderSurfaceLock&& ) = delete;

    FRenderSurfaceLock(const FRenderSurfaceLock& ) = delete;
    FRenderSurfaceLock& operator =(const FRenderSurfaceLock& ) = delete;

    const FAbstractRenderSurface *Owner() const;

    void Acquire(   const Graphics::FRenderTarget **pRenderTarget,
                    const Graphics::FDepthStencil **pDepthStencil ) const;
    void Release(   Graphics::IDeviceAPIEncapsulator *device,
                    PRenderSurfaceLock& plockSelf);

    void Bind(Graphics::EShaderProgramType stage, size_t slot);
    void Unbind(Graphics::IDeviceAPIContext *context);

private:
    typedef Meta::TBit<size_t>::TFirst<8>::type stage_field;
    typedef Meta::TBit<size_t>::TAfter<stage_field>::FRemain::type slots_field;

    size_t _state;
};
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderSurface);
class FAbstractRenderSurface : public FRefCountable {
protected:
    friend class FRenderSurfaceLock;
    explicit FAbstractRenderSurface(FString&& name);

public:
    virtual ~FAbstractRenderSurface();

    const FString& FName() const { return _name; }

    bool InUse() const { return _lock.RefCount() > 1; }
    size_t LockRefCount() const { return _lock.RefCount() - 1; }

    void Prepare(Graphics::IDeviceAPIEncapsulator *device, PRenderSurfaceLock& plock);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device, PRenderSurfaceLock& plock);

    // TODO : handle resizing of the screen ...
    virtual void OnResize(Graphics::IDeviceAPIEncapsulator * /* device */) {}

protected:
    virtual void CreateResources_(  Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) = 0;

    virtual void DestroyResources_( Graphics::IDeviceAPIEncapsulator *device,
                                    Graphics::PCRenderTarget& pRenderTarget,
                                    Graphics::PCDepthStencil& pDepthStencil ) = 0;

private:
    void AcquireFromLock_(  const FRenderSurfaceLock *plock,
                            const Graphics::FRenderTarget **pRenderTarget,
                            const Graphics::FDepthStencil **pDepthStencil ) const;

    FString _name;

    FRenderSurfaceLock _lock;

    Graphics::PCRenderTarget _renderTarget;
    Graphics::PCDepthStencil _depthStencil;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
