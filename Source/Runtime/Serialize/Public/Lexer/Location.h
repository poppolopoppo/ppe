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
    FLocation() = default;
    FLocation(const FWStringView& filename, size_t line, size_t column, std::streamoff offset) NOEXCEPT
        : Filename(filename)
        , Offset(offset)
        , Line(checked_cast<u32>(line))
        , Column(checked_cast<u32>(column))
    {}

    FWStringView Filename;
    std::streamoff Offset{ 0 };
    u32 Line{ 0 };
    u32 Column{ 0 };

    void Rewind() {
        Offset = 0;
        Line = Column = 0;
    }

    static FLocation None() { return FLocation(); }
};
//----------------------------------------------------------------------------
struct FSpan : FLocation {
    FSpan() = default;
    FSpan(const FLocation& location, size_t length) NOEXCEPT
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

    u32 Length{ 0 };
};
//----------------------------------------------------------------------------
// Need a copy of Filename string for errors
struct FErrorSpan {
    FWString Filename;
    std::streamoff Offset{ 0 };
    u32 Line{ 0 };
    u32 Column{ 0 };
    u32 Length{ 0 };

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
template <typename _Char>
inline TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Lexer::FLocation& site) {
    return oss << site.Filename << STRING_LITERAL(_Char, '(')
        << site.Line << STRING_LITERAL(_Char, ':')
        << site.Column << STRING_LITERAL(_Char, ')');
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
