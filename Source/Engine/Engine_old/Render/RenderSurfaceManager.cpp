#include "stdafx.h"

#include "RenderSurfaceManager.h"

#include "Core.Graphics/Device/DeviceAPI.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/FileSystemTrie.h"

#include "Surfaces/AbstractRenderSurface.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderSurfaceManager::FRenderSurfaceManager(
    const FileSystem::char_type *virtualDir,
    const FileSystem::char_type *renderTargetName,
    const FileSystem::char_type *depthStencilName )
:   _virtualDir(virtualDir)
,   _renderTargetName(renderTargetName)
,   _depthStencilName(depthStencilName) {
    Assert(!_virtualDir.empty());
    Assert(!_renderTargetName.empty());
    Assert(!_depthStencilName.empty());
}
//----------------------------------------------------------------------------
FRenderSurfaceManager::~FRenderSurfaceManager() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_surfaces.empty());
}
//----------------------------------------------------------------------------
void FRenderSurfaceManager::Register(FAbstractRenderSurface *renderSurface) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderSurface);
    Assert(renderSurface->Name().size());

    const FDirname virtualDirname(ToWString(renderSurface->Name()));

    LOG(Info, L"[FRenderSurfaceManager] Register render surface with alias \"{0}\" ...",
        FDirpath(_virtualDir, virtualDirname) );

    PAbstractRenderSurface& it = _surfaces[virtualDirname];
    AssertRelease(!it);
    Assert(!renderSurface->InUse());

    it = renderSurface;
}
//----------------------------------------------------------------------------
void FRenderSurfaceManager::Unregister(FAbstractRenderSurface *renderSurface) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderSurface);
    Assert(renderSurface->Name().size());

    const FDirname virtualDirname(ToWString(renderSurface->Name()));

    LOG(Info, L"[FRenderSurfaceManager] Unregister render surface with alias \"{0}\" ...",
        FDirpath(_virtualDir, virtualDirname) );

    Assert(!renderSurface->InUse());
    Assert(renderSurface == _surfaces[virtualDirname]);
    _surfaces.erase(virtualDirname);
}
//----------------------------------------------------------------------------
FDirpath FRenderSurfaceManager::Alias(const FAbstractRenderSurface *renderSurface) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderSurface);
    Assert(renderSurface->Name().size());

    const FDirname virtualDirname(ToWString(renderSurface->Name()));
    return FDirpath(_virtualDir, virtualDirname);
}
//----------------------------------------------------------------------------
bool FRenderSurfaceManager::TryUnalias(const FDirpath& dirpath, PAbstractRenderSurface *renderSurface) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!dirpath.empty());
    Assert(renderSurface);

    if (dirpath.PathNode()->Parent() != _virtualDir.PathNode())
        return false;

    const FDirname& virtualDirname = dirpath.LastDirname();
    Assert(!virtualDirname.empty());

    const auto it = _surfaces.find(virtualDirname);
    if (_surfaces.end() == it)
        return false;

    Assert(it->second);
    *renderSurface = it->second;
    return true;
}
//----------------------------------------------------------------------------
FAbstractRenderSurface *FRenderSurfaceManager::Unalias(const FDirpath& dirpath) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!dirpath.empty());
    Assert(dirpath.PathNode()->Parent() == _virtualDir.PathNode());

    const FDirname virtualDirname = dirpath.LastDirname();
    Assert(!virtualDirname.empty());

    FAbstractRenderSurface *const result = _surfaces.at(virtualDirname).get();
    Assert(result);

    return result;
}
//----------------------------------------------------------------------------
void FRenderSurfaceManager::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _surfaces.clear();
}
//----------------------------------------------------------------------------
void FRenderSurfaceManager::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(_surfaces.empty());
    Assert(!_virtualDir.empty());
    Assert(!_renderTargetName.empty());
    Assert(!_depthStencilName.empty());

    LOG(Info, L"[FRenderSurfaceManager] Starting with device <{0}> ...",
        device );
}
//----------------------------------------------------------------------------
void FRenderSurfaceManager::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(!_virtualDir.empty());
    Assert(!_renderTargetName.empty());
    Assert(!_depthStencilName.empty());

    LOG(Info, L"[FRenderSurfaceManager] Shutting down with device <{0}> ...",
        device );

    for (TPair<const FDirname, PAbstractRenderSurface>& it : _surfaces) {
        Assert(!it.first.empty());
        Assert(it.second);
        Assert(!it.second->InUse());

        LOG(Info, L"[FRenderSurfaceManager] Destroy render surface with alias \"{0}\" ...",
            it.first );

        RemoveRef_AssertReachZero(it.second);
    }

    _surfaces.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
