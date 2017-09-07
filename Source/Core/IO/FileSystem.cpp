#include "stdafx.h"

#include "FileSystem.h"

#include "FS/ConstNames.h"
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
    FFileSystemToken::Start();
    FFileSystemPath::Create();
    FFSConstNames::Start();
}
//----------------------------------------------------------------------------
void FFileSystemStartup::Shutdown() {
    FFSConstNames::Shutdown();
    FFileSystemPath::Destroy();
    FFileSystemToken::Shutdown();
    POOL_TAG(FileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void FFileSystemStartup::Clear() {
    FFileSystemPath::Instance().Clear();
    POOL_TAG(FileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
