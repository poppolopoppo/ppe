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
void FileSystemStartup::Start() {
    POOL_TAG(FileSystem)::Start();
    FileSystemToken::Start(1024);
    FileSystemPath::Create();
    FileSystemConstNames::Start();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Shutdown() {
    FileSystemConstNames::Shutdown();
    FileSystemPath::Destroy();
    FileSystemToken::Shutdown();
    POOL_TAG(FileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Clear() {
    FileSystemConstNames::Shutdown();
    FileSystemPath::Instance().Clear();
    FileSystemToken::Clear();
    FileSystemConstNames::Start();
    POOL_TAG(FileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
