#pragma once

#include "Core.Serialize/Serialize.h"

#include <iosfwd>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLocation {
    FLocation() : FLocation(nullptr, 0, 0) {}
    FLocation(const wchar_t *fileName, size_t line, size_t column)
        : FileName(fileName), Line(line), Column(column) {}

    const wchar_t *FileName;
    size_t Line;
    size_t Column;

    static FLocation None() { return FLocation(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Lexer::FLocation& site) {
    return oss << site.FileName << '(' << site.Line << ":" << site.Column << ')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
