#include "stdafx.h"

#include "FileSystemProperties.h"

#ifdef OS_WINDOWS
#   include <windows.h>
#else
#   error "not implemented"
#endif

namespace Core {
namespace FileSystem {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TokenTraits::IsAllowedChar(wchar_t ch) const {
    return  std::isalnum(ch, Locale()) ||
            ch == L'_' || ch == L'-' || ch == L':' || ch == L'.';
}
//----------------------------------------------------------------------------
size_t SystemTemporaryDirectory(char_type *path, size_t capacity) {
    Assert(path);

#ifdef OS_WINDOWS
    Assert(capacity >= MAX_PATH);
    return checked_cast<size_t>(GetTempPathW(checked_cast<DWORD>(capacity), path));

#else
    return 0;

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace Core