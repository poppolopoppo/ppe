#include "stdafx.h"

#include "FileSystemProperties.h"

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#else
#   error "not implemented"
#endif

namespace Core {
namespace FileSystem {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool TTokenTraits::IsAllowedChar(wchar_t ch) const {
    return  IsAlnum(ch) || ch == L'_' || ch == L'-' || ch == L':' || ch == L'.';
}
//----------------------------------------------------------------------------
size_t WorkingDirectory(char_type *path, size_t capacity) {
    Assert(path);

#ifdef PLATFORM_WINDOWS
    Assert(capacity >= MaxPathLength);
    return checked_cast<size_t>(::GetCurrentDirectoryW(checked_cast<DWORD>(capacity), path));

#else
    AssertNotImplemented();
    return 0;

#endif
}
//----------------------------------------------------------------------------
size_t SystemTemporaryDirectory(char_type *path, size_t capacity) {
    Assert(path);

#ifdef PLATFORM_WINDOWS
    Assert(capacity >= MaxPathLength);
    return checked_cast<size_t>(::GetTempPathW(checked_cast<DWORD>(capacity), path));

#else
    AssertNotImplemented();
    return 0;

#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace Core
