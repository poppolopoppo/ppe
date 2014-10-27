#include "stdafx.h"

#include "FileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemStartup::Start() {
    Extname::Start(32);
    MountingPoint::Start(32);
    BasenameNoExt::Start(128);
    Dirname::Start(128);
}
//----------------------------------------------------------------------------
void FileSystemStartup::Shutdown() {
    Dirname::Shutdown();
    BasenameNoExt::Shutdown();
    MountingPoint::Shutdown();
    Extname::Shutdown();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Clear() {
    Dirname::Clear();
    BasenameNoExt::Clear();
    MountingPoint::Clear();
    Extname::Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
