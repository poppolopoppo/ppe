#include "stdafx.h"

#include "FormatHelpers.h"

#include "Format.h"
#include "StreamProvider.h"

#include "Maths/Units.h"
#include "Memory/MemoryProvider.h"


namespace Core {
namespace Fmt {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView Format(char *buffer, size_t capacity, Fmt::FCountOfElements count) {
    FFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    if (count > 9e5f)
        oss << (count / 1e6f) << " M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << " K";
    else
        oss << count.Value;

    return oss.Written();
}
//----------------------------------------------------------------------------
FWStringView Format(wchar_t *buffer, size_t capacity, Fmt::FCountOfElements count) {
    FWFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    if (count > 9e5f)
        oss << (count / 1e6f) << L" M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << L" K";
    else
        oss << count.Value;

    return oss.Written();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView Format(char *buffer, size_t capacity, Fmt::FPointer ptr) {
    FFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));
    oss << (const void*)ptr.Value;
    return oss.Written();
}
//----------------------------------------------------------------------------
FWStringView Format(wchar_t *buffer, size_t capacity, Fmt::FPointer ptr) {
    FWFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));
    oss << (const void*)ptr.Value;
    return oss.Written();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView Format(char *buffer, size_t capacity, Fmt::FSizeInBytes size) {
    FFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    const Units::Storage::FBytes bytes(static_cast<double>(size));

    const Units::Storage::FPetabytes pb(0.8);
    const Units::Storage::FTerabytes tb(0.8);
    const Units::Storage::FGigabytes gb(0.8);
    const Units::Storage::FMegabytes mb(0.8);
    const Units::Storage::FKilobytes kb(0.8);

    if (bytes > pb)
        oss << Units::Storage::FPetabytes(bytes);
    else if (bytes > tb)
        oss << Units::Storage::FTerabytes(bytes);
    else if (bytes > gb)
        oss << Units::Storage::FGigabytes(bytes);
    else if (bytes > mb)
        oss << Units::Storage::FMegabytes(bytes);
    else if (bytes > kb)
        oss << Units::Storage::FKilobytes(bytes);
    else
        oss << bytes;

    return oss.Written();
}
//----------------------------------------------------------------------------
FWStringView Format(wchar_t *buffer, size_t capacity, Fmt::FSizeInBytes size) {
    FWFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    const Units::Storage::FBytes bytes(static_cast<double>(size));

    const Units::Storage::FPetabytes pb(0.8);
    const Units::Storage::FTerabytes tb(0.8);
    const Units::Storage::FGigabytes gb(0.8);
    const Units::Storage::FMegabytes mb(0.8);
    const Units::Storage::FKilobytes kb(0.8);

    if (bytes > pb)
        oss << Units::Storage::FPetabytes(bytes);
    else if (bytes > tb)
        oss << Units::Storage::FTerabytes(bytes);
    else if (bytes > gb)
        oss << Units::Storage::FGigabytes(bytes);
    else if (bytes > mb)
        oss << Units::Storage::FMegabytes(bytes);
    else if (bytes > kb)
        oss << Units::Storage::FKilobytes(bytes);
    else
        oss << bytes;

    return oss.Written();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Fmt
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        Core::Format(oss, "0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                Core::Format(oss, " {0:#2X}", (unsigned)hexDump.RawData[offset]);
            else
                oss << "   ";
        }
        oss << "  ";
        offset = origin;
        for (size_t row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) ? char(hexDump.RawData[offset]) : '.');
        oss << Eol;
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        Core::Format(oss, L"0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                Core::Format(oss, L" {0:#2X}", (unsigned)hexDump.RawData[offset]);
            else
                oss << L"   ";
        }
        oss << L"  ";
        offset = origin;
        for (size_t row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) ? char(hexDump.RawData[offset]) : '.');
        oss << Eol;
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FIndent& indent) {
    Assert(indent.Level >= 0);
    forrange(i, 0, indent.Level)
        oss << indent.Tab;
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FWIndent& indent) {
    Assert(indent.Level >= 0);
    forrange(i, 0, indent.Level)
        oss << indent.Tab;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::EChar ch) {
    switch (ch) {
    case Core::Fmt::LBrace:
        return oss << '{';
    case Core::Fmt::RBrace:
        return oss << '}';
    case Core::Fmt::LBracket:
        return oss << '[';
    case Core::Fmt::RBracket:
        return oss << ']';
    case Core::Fmt::LParenthese:
        return oss << '(';
    case Core::Fmt::RParenthese:
        return oss << ')';
    case Core::Fmt::Comma:
        return oss << ',';
    case Core::Fmt::Colon:
        return oss << ':';
    case Core::Fmt::SemiColon:
        return oss << ';';
    case Core::Fmt::Dot:
        return oss << '.';
    case Core::Fmt::Dollar:
        return oss << '$';
    case Core::Fmt::Question:
        return oss << '?';
    case Core::Fmt::Add:
        return oss << '+';
    case Core::Fmt::Sub:
        return oss << '-';
    case Core::Fmt::Mul:
        return oss << '*';
    case Core::Fmt::Div:
        return oss << '/';
    case Core::Fmt::Mod:
        return oss << '%';
    case Core::Fmt::Pow:
        return oss << "**";
    case Core::Fmt::Increment:
        return oss << "++";
    case Core::Fmt::Decrement:
        return oss << "--";
    case Core::Fmt::LShift:
        return oss << "<<";
    case Core::Fmt::RShift:
        return oss << ">>";
    case Core::Fmt::And:
        return oss << '&';
    case Core::Fmt::Or:
        return oss << '|';
    case Core::Fmt::Not:
        return oss << '!';
    case Core::Fmt::Xor:
        return oss << '^';
    case Core::Fmt::Complement:
        return oss << '~';
    case Core::Fmt::Assignment:
        return oss << '=';
    case Core::Fmt::Equals:
        return oss << "==";
    case Core::Fmt::NotEquals:
        return oss << "!=";
    case Core::Fmt::Less:
        return oss << '<';
    case Core::Fmt::LessOrEqual:
        return oss << "<=";
    case Core::Fmt::Greater:
        return oss << '>';
    case Core::Fmt::GreaterOrEqual:
        return oss << ">=";
    case Core::Fmt::DotDot:
        return oss << "..";
    case Core::Fmt::Sharp:
        return oss << '#';
    case Core::Fmt::Quote:
        return oss << '\'';
    case Core::Fmt::DoubleQuote:
        return oss << '"';
    case Core::Fmt::Space:
        return oss << ' ';
    case Core::Fmt::Tab:
        return oss << ' ';
    default:
        AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::EChar ch) {
    switch (ch) {
    case Core::Fmt::LBrace:
        return oss << L'{';
    case Core::Fmt::RBrace:
        return oss << L'}';
    case Core::Fmt::LBracket:
        return oss << L'[';
    case Core::Fmt::RBracket:
        return oss << L']';
    case Core::Fmt::LParenthese:
        return oss << L'(';
    case Core::Fmt::RParenthese:
        return oss << L')';
    case Core::Fmt::Comma:
        return oss << L',';
    case Core::Fmt::Colon:
        return oss << L':';
    case Core::Fmt::SemiColon:
        return oss << L';';
    case Core::Fmt::Dot:
        return oss << L'.';
    case Core::Fmt::Dollar:
        return oss << L'$';
    case Core::Fmt::Question:
        return oss << L'?';
    case Core::Fmt::Add:
        return oss << L'+';
    case Core::Fmt::Sub:
        return oss << L'-';
    case Core::Fmt::Mul:
        return oss << L'*';
    case Core::Fmt::Div:
        return oss << L'/';
    case Core::Fmt::Mod:
        return oss << L'%';
    case Core::Fmt::Pow:
        return oss << L"**";
    case Core::Fmt::Increment:
        return oss << L"++";
    case Core::Fmt::Decrement:
        return oss << L"--";
    case Core::Fmt::LShift:
        return oss << L"<<";
    case Core::Fmt::RShift:
        return oss << L">>";
    case Core::Fmt::And:
        return oss << L'&';
    case Core::Fmt::Or:
        return oss << L'|';
    case Core::Fmt::Not:
        return oss << L'!';
    case Core::Fmt::Xor:
        return oss << L'^';
    case Core::Fmt::Complement:
        return oss << L'~';
    case Core::Fmt::Assignment:
        return oss << L'=';
    case Core::Fmt::Equals:
        return oss << L"==";
    case Core::Fmt::NotEquals:
        return oss << L"!=";
    case Core::Fmt::Less:
        return oss << L'<';
    case Core::Fmt::LessOrEqual:
        return oss << L"<=";
    case Core::Fmt::Greater:
        return oss << L'>';
    case Core::Fmt::GreaterOrEqual:
        return oss << L">=";
    case Core::Fmt::DotDot:
        return oss << L"..";
    case Core::Fmt::Sharp:
        return oss << L'#';
    case Core::Fmt::Quote:
        return oss << L'\'';
    case Core::Fmt::DoubleQuote:
        return oss << L'"';
    case Core::Fmt::Space:
        return oss << L' ';
    case Core::Fmt::Tab:
        return oss << L' ';
    default:
        AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
