#include "stdafx.h"

#include "VirtualFileSystemComponent.h"

#include "IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemComponent::VirtualFileSystemComponent(const Dirpath& alias)
:   _alias(alias) {
    Assert(!_alias.empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VirtualFileSystemComponentStartup::Start(VirtualFileSystemComponent *component) {
    Assert(component);
    VirtualFileSystem::Instance().Mount(component);
}
//----------------------------------------------------------------------------
void VirtualFileSystemComponentStartup::Shutdown(VirtualFileSystemComponent *component) {
    Assert(component);
    VirtualFileSystem::Instance().Unmount(component);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
