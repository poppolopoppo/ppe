#include "stdafx.h"

#include "FileSystem.h"

#include "FileSystemConstNames.h"
#include "FS/FileSystemToken.h"
#include "FS/FileSystemTrie.h"

#include "Allocator/PoolAllocatorTag-impl.h"

namespace Core {
POOL_TAG_DEF(FileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFileSystemStartup::Start() {
    POOL_TAG(FileSystem)::Start();
    FFileSystemToken::Start(1024);
    FFileSystemPath::Create();
    FFileSystemConstNames::Start();
}
//----------------------------------------------------------------------------
void FFileSystemStartup::Shutdown() {
    FFileSystemConstNames::Shutdown();
    FFileSystemPath::Destroy();
    FFileSystemToken::Shutdown();
    POOL_TAG(FileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void FFileSystemStartup::Clear() {
    FFileSystemConstNames::Shutdown();
    FFileSystemPath::Instance().Clear();
    FFileSystemToken::Clear();
    FFileSystemConstNames::Start();
    POOL_TAG(FileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
