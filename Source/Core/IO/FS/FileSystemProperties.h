#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/String_fwd.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace FileSystem {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef wchar_t char_type;
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(ECase, CaseSensitive, ECase::Insensitive);
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MaxPathLength, 256);
//----------------------------------------------------------------------------
enum : char_type { Separator = L'/', AltSeparator = L'\\' };
//----------------------------------------------------------------------------
class TTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(wchar_t ch) const;
};
//----------------------------------------------------------------------------
using FString = TBasicString<char_type>;
using FStringView = TBasicStringView<char_type>;
//----------------------------------------------------------------------------
inline FileSystem::FStringView Separators() { return MakeStringView(L"/\\"); }
//----------------------------------------------------------------------------
size_t WorkingDirectory(char_type *path, size_t capacity);
//----------------------------------------------------------------------------
size_t SystemTemporaryDirectory(char_type *path, size_t capacity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace Core
