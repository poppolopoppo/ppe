#include "stdafx.h"

#include "RenderSurfaceManager.h"

#include "Core.Graphics/DeviceAPIEncapsulator.h"

#include "Core/Logger.h"

#include "AbstractRenderSurface.h"

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

    if (dirpath.MountingPoint() != _virtualDir.MountingPoint())
        return false;

    if (dirpath.Path().size() != _virtualDir.Path().size() + 1)
        return false;

    const auto end = _virtualDir.Path().begin();
    auto j = dirpath.Path().begin();
    for (auto i = _virtualDir.Path().begin(); i != end; ++i, ++j)
        if (*i != *j)
            return false;

    const Dirname& virtualDirname = dirpath.Path().back();
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
#ifdef _DEBUG
    {
        PAbstractRenderSurface renderSurfaceForDBG;
        Assert(TryUnalias(dirpath, &renderSurfaceForDBG));
    }
#endif

    const Dirname& virtualDirname = dirpath.Path().back();
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
