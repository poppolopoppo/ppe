#pragma once

#include "Core.Serialize/Serialize.h"

#include <iosfwd>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct Location {
    Location() : Location(nullptr, 0, 0) {}
    Location(const wchar_t *fileName, size_t line, size_t column)
        : FileName(fileName), Line(line), Column(column) {}

    const wchar_t *FileName;
    size_t Line;
    size_t Column;

    static Location None() { return Location(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Location& site) {
    return oss << site.FileName << '(' << site.Line << ":" << site.Column << ')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
