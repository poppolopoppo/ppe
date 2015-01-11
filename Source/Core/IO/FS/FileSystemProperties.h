#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/String.h"

namespace Core {
namespace FileSystem {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef wchar_t char_type;
//----------------------------------------------------------------------------
enum : char_type { Separator = L'/', AltSeparator = L'\\' };
//----------------------------------------------------------------------------
inline const char_type *Separators() { return L"/\\"; }
//----------------------------------------------------------------------------
class TokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(wchar_t ch) const;
};
//----------------------------------------------------------------------------
size_t SystemTemporaryDirectory(char_type *path, size_t capacity);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace Core
