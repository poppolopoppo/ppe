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
    FLocation() : FLocation(FWStringView(), 0, 0, 0) {}
    FLocation(const FWStringView& filename, size_t line, size_t column, std::streamoff offset)
        : Filename(filename)
        , Offset(offset)
        , Line(checked_cast<u32>(line))
        , Column(checked_cast<u32>(column))
    {}

    FWStringView Filename;
    std::streamoff Offset;
    u32 Line;
    u32 Column;

    static FLocation None() { return FLocation(); }
};
//----------------------------------------------------------------------------
struct FSpan : FLocation {
    FSpan() : Length(0) {}
    FSpan(const FLocation& location, size_t length)
        : FLocation(location)
        , Length(checked_cast<u32>(length))
    {}

    FSpan SubRange(size_t offset, size_t count) const {
        Assert_NoAssume(offset + count < Length);
        return FSpan(FLocation(Filename, Line, Column + offset, Offset + offset), count);
    }

    static FSpan FromSite(const FLocation& start, const FLocation& stop) {
        Assert(stop.Offset >= start.Offset);
        return FSpan(start, checked_cast<size_t>(stop.Offset - start.Offset));
    }

    static FSpan FromSpan(const FSpan& start, const FSpan& stop) {
        Assert(stop.Offset >= start.Offset);
        return FSpan(start, checked_cast<size_t>((stop.Offset + stop.Length) - start.Offset));
    }

    u32 Length;
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
