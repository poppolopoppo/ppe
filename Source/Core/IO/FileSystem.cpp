#include "stdafx.h"

#include "FileSystem.h"

#include "FS/FileSystemToken.h"
#include "FS/FileSystemTrie.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemStartup::Start() {
    FileSystemToken::Start(1024);
    FileSystemPath::Create();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Shutdown() {
    FileSystemPath::Destroy();
    FileSystemToken::Shutdown();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Clear() {
    FileSystemPath::Instance().Clear();
    FileSystemToken::Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
