#pragma once

#include "Serialize.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLocation {
    FLocation() : FLocation(FWStringView(), 0, 0) {}
    FLocation(const FWStringView& filename, size_t line, size_t column)
        : Filename(filename), Line(line), Column(column) {}

    FWStringView Filename;
    size_t Line;
    size_t Column;

    static FLocation None() { return FLocation(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Lexer::FLocation& site) {
    return oss << site.Filename << '(' << site.Line << ':' << site.Column << ')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
