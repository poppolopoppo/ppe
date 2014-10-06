#pragma once

#include "Engine.h"

#include "Core/Basename.h"
#include "Core/Dirpath.h"
#include "Core/HashMap.h"
#include "Core/RefPtr.h"
#include "Core/ThreadResource.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderSurface);
//----------------------------------------------------------------------------
class RenderSurfaceManager : public Meta::ThreadResource {
public:
    RenderSurfaceManager(   const FileSystem::char_type *virtualDir,
                            const FileSystem::char_type *renderTargetName,
                            const FileSystem::char_type *depthStencilName );
    ~RenderSurfaceManager();

    RenderSurfaceManager(const RenderSurfaceManager& ) = delete;
    RenderSurfaceManager& operator =(const RenderSurfaceManager& ) = delete;

    const Dirpath& VirtualDir() const { return _virtualDir; }
    const Basename& RenderTargetName() const { return _renderTargetName; }
    const Basename& DepthStencilName() const { return _depthStencilName; }

    void Register(AbstractRenderSurface *renderSurface);
    void Unregister(AbstractRenderSurface *renderSurface);

    Dirpath Alias(const AbstractRenderSurface *renderSurface) const;
    bool TryUnalias(const Dirpath& dirpath, PAbstractRenderSurface *renderSurface) const;
    AbstractRenderSurface *Unalias(const Dirpath& dirpath) const;

    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    const Dirpath _virtualDir;
    const Basename _renderTargetName;
    const Basename _depthStencilName;

    HASHMAP(Service, Dirname, PAbstractRenderSurface) _surfaces;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
