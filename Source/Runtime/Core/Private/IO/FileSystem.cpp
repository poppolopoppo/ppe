#include "stdafx.h"

#include "IO/FileSystem.h"

#include "IO/ConstNames.h"
#include "IO/FileSystemToken.h"
#include "IO/FileSystemTrie.h"

#include "Allocator/PoolAllocatorTag-impl.h"

namespace PPE {
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
    FFileSystemPath::Get().Clear();
    POOL_TAG(FileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
