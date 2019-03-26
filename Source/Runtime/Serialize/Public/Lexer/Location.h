#pragma once

#include "Serialize.h"

#include "IO/String.h"
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
// Need a copy of Filename string for errors
struct FErrorSpan {
    FWString Filename;
    std::streamoff Offset;
    u32 Line;
    u32 Column;
    u32 Length;

    FErrorSpan(const FSpan& span)
        : Filename(span.Filename)
        , Offset(span.Offset)
        , Line(span.Line)
        , Column(span.Column)
        , Length(span.Length)
    {}

    operator FSpan () const {
        return FSpan({ Filename, Line, Column, Offset }, Length);
    }
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
inline FTextWriter& operator <<(FTextWriter& oss, const Lexer::FLocation& site) {
    return oss << site.Filename << '(' << site.Line << ':' << site.Column << ')';
}
//----------------------------------------------------------------------------
inline FWTextWriter& operator <<(FWTextWriter& oss, const Lexer::FLocation& site) {
    return oss << site.Filename << L'(' << site.Line << L':' << site.Column << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
