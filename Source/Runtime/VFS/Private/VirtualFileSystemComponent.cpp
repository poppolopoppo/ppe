// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VirtualFileSystemComponent.h"

#include "VirtualFileSystem.h"

namespace PPE {
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
} //!namespace PPE
