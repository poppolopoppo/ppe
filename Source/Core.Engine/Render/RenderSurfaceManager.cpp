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
RenderSurfaceManager::RenderSurfaceManager(
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
RenderSurfaceManager::~RenderSurfaceManager() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_surfaces.empty());
}
//----------------------------------------------------------------------------
void RenderSurfaceManager::Register(AbstractRenderSurface *renderSurface) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderSurface);
    Assert(renderSurface->Name().size());

    const Dirname virtualDirname(ToWString(renderSurface->Name()));

    LOG(Information, L"[RenderSurfaceManager] Register render surface with alias \"{0}\" ...",
        Dirpath(_virtualDir, virtualDirname) );

    PAbstractRenderSurface& it = _surfaces[virtualDirname];
    AssertRelease(!it);
    Assert(!renderSurface->InUse());

    it = renderSurface;
}
//----------------------------------------------------------------------------
void RenderSurfaceManager::Unregister(AbstractRenderSurface *renderSurface) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderSurface);
    Assert(renderSurface->Name().size());

    const Dirname virtualDirname(ToWString(renderSurface->Name()));

    LOG(Information, L"[RenderSurfaceManager] Unregister render surface with alias \"{0}\" ...",
        Dirpath(_virtualDir, virtualDirname) );

    Assert(!renderSurface->InUse());
    Assert(renderSurface == _surfaces[virtualDirname]);
    _surfaces.erase(virtualDirname);
}
//----------------------------------------------------------------------------
Dirpath RenderSurfaceManager::Alias(const AbstractRenderSurface *renderSurface) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(renderSurface);
    Assert(renderSurface->Name().size());

    const Dirname virtualDirname(ToWString(renderSurface->Name()));
    return Dirpath(_virtualDir, virtualDirname);
}
//----------------------------------------------------------------------------
bool RenderSurfaceManager::TryUnalias(const Dirpath& dirpath, PAbstractRenderSurface *renderSurface) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!dirpath.empty());
    Assert(renderSurface);

    if (dirpath.PathNode()->Parent() != _virtualDir.PathNode())
        return false;

    const Dirname& virtualDirname = dirpath.LastDirname();
    Assert(!virtualDirname.empty());

    const auto it = _surfaces.find(virtualDirname);
    if (_surfaces.end() == it)
        return false;

    Assert(it->second);
    *renderSurface = it->second;
    return true;
}
//----------------------------------------------------------------------------
AbstractRenderSurface *RenderSurfaceManager::Unalias(const Dirpath& dirpath) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!dirpath.empty());
    Assert(dirpath.PathNode()->Parent() == _virtualDir.PathNode());

    const Dirname virtualDirname = dirpath.LastDirname();
    Assert(!virtualDirname.empty());

    AbstractRenderSurface *const result = _surfaces.at(virtualDirname).get();
    Assert(result);

    return result;
}
//----------------------------------------------------------------------------
void RenderSurfaceManager::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _surfaces.clear();
}
//----------------------------------------------------------------------------
void RenderSurfaceManager::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(_surfaces.empty());
    Assert(!_virtualDir.empty());
    Assert(!_renderTargetName.empty());
    Assert(!_depthStencilName.empty());

    LOG(Information, L"[RenderSurfaceManager] Starting with device <{0}> ...",
        device );
}
//----------------------------------------------------------------------------
void RenderSurfaceManager::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(!_virtualDir.empty());
    Assert(!_renderTargetName.empty());
    Assert(!_depthStencilName.empty());

    LOG(Information, L"[RenderSurfaceManager] Shutting down with device <{0}> ...",
        device );

    for (Pair<const Dirname, PAbstractRenderSurface>& it : _surfaces) {
        Assert(!it.first.empty());
        Assert(it.second);
        Assert(!it.second->InUse());

        LOG(Information, L"[RenderSurfaceManager] Destroy render surface with alias \"{0}\" ...",
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
