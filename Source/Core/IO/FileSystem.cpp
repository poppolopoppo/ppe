#include "stdafx.h"

#include "FileSystem.h"

#include "FileSystemConstNames.h"
#include "FS/FileSystemToken.h"
#include "FS/FileSystemTrie.h"

#include "Allocator/PoolAllocatorTag-impl.h"

namespace Core {
POOLTAG_DEF(FileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemStartup::Start() {
    POOLTAG(FileSystem)::Start();
    FileSystemToken::Start(1024);
    FileSystemPath::Create();
    FileSystemConstNames::Start();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Shutdown() {
    FileSystemConstNames::Shutdown();
    FileSystemPath::Destroy();
    FileSystemToken::Shutdown();
    POOLTAG(FileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Clear() {
    FileSystemConstNames::Shutdown();
    FileSystemPath::Instance().Clear();
    FileSystemToken::Clear();
    FileSystemConstNames::Start();
    POOLTAG(FileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
