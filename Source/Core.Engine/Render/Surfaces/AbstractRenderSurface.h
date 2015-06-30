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
enum class ShaderProgramType;
class SurfaceFormat;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractRenderSurface;
FWD_REFPTR(RenderSurfaceLock);
//----------------------------------------------------------------------------
// Same lifetime than created resources
class RenderSurfaceLock : public RefCountable {
private:
    friend class AbstractRenderSurface;
    RenderSurfaceLock(const AbstractRenderSurface *owner);

public:
    ~RenderSurfaceLock();

    RenderSurfaceLock(RenderSurfaceLock&& ) = delete;
    RenderSurfaceLock& operator =(RenderSurfaceLock&& ) = delete;

    RenderSurfaceLock(const RenderSurfaceLock& ) = delete;
    RenderSurfaceLock& operator =(const RenderSurfaceLock& ) = delete;

    const AbstractRenderSurface *Owner() const;

    void Acquire(   const Graphics::RenderTarget **pRenderTarget,
                    const Graphics::DepthStencil **pDepthStencil ) const;
    void Release(   Graphics::IDeviceAPIEncapsulator *device,
                    PRenderSurfaceLock& plockSelf);

    void Bind(Graphics::ShaderProgramType stage, size_t slot);
    void Unbind(Graphics::IDeviceAPIContextEncapsulator *context);

private:
    typedef Meta::Bit<size_t>::First<8>::type stage_field;
    typedef Meta::Bit<size_t>::After<stage_field>::Remain::type slots_field;

    size_t _state;
};
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderSurface);
class AbstractRenderSurface : public RefCountable {
protected:
    friend class RenderSurfaceLock;
    explicit AbstractRenderSurface(String&& name);

public:
    virtual ~AbstractRenderSurface();

    const String& Name() const { return _name; }

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
    void AcquireFromLock_(  const RenderSurfaceLock *plock,
                            const Graphics::RenderTarget **pRenderTarget,
                            const Graphics::DepthStencil **pDepthStencil ) const;

    String _name;

    RenderSurfaceLock _lock;

    Graphics::PCRenderTarget _renderTarget;
    Graphics::PCDepthStencil _depthStencil;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
