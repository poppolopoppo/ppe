#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace FileSystem {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef wchar_t char_type;
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(Case, CaseSensitive, Case::Insensitive);
//----------------------------------------------------------------------------
enum : char_type { Separator = L'/', AltSeparator = L'\\' };
//----------------------------------------------------------------------------
class TokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(wchar_t ch) const;
};
//----------------------------------------------------------------------------
typedef BasicStringView<char_type> StringView;
//----------------------------------------------------------------------------
inline FileSystem::StringView Separators() { return MakeStringView(L"/\\"); }
//----------------------------------------------------------------------------
size_t SystemTemporaryDirectory(char_type *path, size_t capacity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace Core
