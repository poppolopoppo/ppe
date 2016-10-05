#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/HashMap.h"
#include "Core/IO/FS/Basename.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

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
class FRenderSurfaceManager : public Meta::FThreadResource {
public:
    FRenderSurfaceManager(   const FileSystem::char_type *virtualDir,
                            const FileSystem::char_type *renderTargetName,
                            const FileSystem::char_type *depthStencilName );
    ~FRenderSurfaceManager();

    FRenderSurfaceManager(const FRenderSurfaceManager& ) = delete;
    FRenderSurfaceManager& operator =(const FRenderSurfaceManager& ) = delete;

    const FDirpath& VirtualDir() const { return _virtualDir; }
    const FBasename& RenderTargetName() const { return _renderTargetName; }
    const FBasename& DepthStencilName() const { return _depthStencilName; }

    void Register(FAbstractRenderSurface *renderSurface);
    void Unregister(FAbstractRenderSurface *renderSurface);

    FDirpath Alias(const FAbstractRenderSurface *renderSurface) const;
    bool TryUnalias(const FDirpath& dirpath, PAbstractRenderSurface *renderSurface) const;
    FAbstractRenderSurface *Unalias(const FDirpath& dirpath) const;

    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    const FDirpath _virtualDir;
    const FBasename _renderTargetName;
    const FBasename _depthStencilName;

    HASHMAP(TService, FDirname, PAbstractRenderSurface) _surfaces;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
