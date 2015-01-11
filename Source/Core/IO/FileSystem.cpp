#include "stdafx.h"

#include "FileSystem.h"

#include "FileSystemConstNames.h"
#include "FS/FileSystemToken.h"
#include "FS/FileSystemTrie.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemStartup::Start() {
    FileSystemToken::Start(1024);
    FileSystemPath::Create();
    FileSystemConstNames::Start();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Shutdown() {
    FileSystemConstNames::Shutdown();
    FileSystemPath::Destroy();
    FileSystemToken::Shutdown();
}
//----------------------------------------------------------------------------
void FileSystemStartup::Clear() {
    FileSystemConstNames::Shutdown();
    FileSystemPath::Instance().Clear();
    FileSystemToken::Clear();
    FileSystemConstNames::Start();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
