// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/FileSystem.h"

#include "IO/ConstNames.h"
#include "IO/FileSystemToken.h"
#include "IO/FileSystemTrie.h"
#include "IO/String.h"

namespace PPE {
BASICTOKEN_CLASS_DEF(FFileSystemToken);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFileSystemStartup::Start() {
    FFileSystemToken::Start();
    FFileSystemTrie::Create();
    FFSConstNames::Start();
}
//----------------------------------------------------------------------------
void FFileSystemStartup::Shutdown() {
    FFSConstNames::Shutdown();
    FFileSystemTrie::Destroy();
    FFileSystemToken::Shutdown();
}
//----------------------------------------------------------------------------
void FFileSystemStartup::Clear() {
    //FFileSystemTrie::Get().Clear_ReleaseMemory(); // #TODO ref counting ?
}
//----------------------------------------------------------------------------
void FileSystem::Sanitize(const TMemoryView<char_type>& str) NOEXCEPT {
    FFileSystemToken::SanitizeToken(str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
