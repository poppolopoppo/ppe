#include "stdafx.h"

#include "VirtualFileSystemComponent.h"

#include "IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemComponent::FVirtualFileSystemComponent(const FDirpath& alias)
:   _alias(alias) {
    Assert(!_alias.empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVirtualFileSystemComponentStartup::Start(FVirtualFileSystemComponent *component) {
    Assert(component);
    FVirtualFileSystem::Instance().Mount(component);
}
//----------------------------------------------------------------------------
void FVirtualFileSystemComponentStartup::Shutdown(FVirtualFileSystemComponent *component) {
    Assert(component);
    FVirtualFileSystem::Instance().Unmount(component);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
