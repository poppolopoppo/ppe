#include "stdafx.h"

#include "IO/FormatHelpers.h"

#include "IO/Format.h"
#include "IO/StreamProvider.h"

#include "Maths/Units.h"
#include "Memory/MemoryProvider.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FPointer ptr) {
    return oss << (const void*)ptr.Value;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPointer ptr) {
    return oss << (const void*)ptr.Value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FPercentage prc) {
    return (*oss.FormatScope())
        << FTextFormat::PadLeft(6, ' ') // 100.00
        << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << prc.Value
        << '%';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPercentage prc) {
    return (*oss.FormatScope())
        << FTextFormat::PadLeft(6, L' ') // 100.00
        << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << prc.Value
        << L'%';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FCountOfElements count) {
    if (oss.Format().Width() > 1) // fixes alignment for units
        oss.Format().SetWidth(oss.Format().Width() - 2);

    if (count > 9e5f)
        oss << (count / 1e6f) << " M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << " K";
    else
        oss << count.Value << "  "/* for alignment */;

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FCountOfElements count) {
    if (oss.Format().Width() > 1) // fixes alignment for units
        oss.Format().SetWidth(oss.Format().Width() - 2);

    if (count > 9e5f)
        oss << (count / 1e6f) << L" M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << L" K";
    else
        oss << count.Value << L"  "/* for alignment */;

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FSizeInBytes size) {
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

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FSizeInBytes size) {
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

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FDurationInMs v) {
    const Units::Time::FMicroseconds us(v.Value);

    const Units::Time::FHours hr(0.8);
    const Units::Time::FSeconds sc(0.8);
    const Units::Time::FMilliseconds ms(0.8);

    if (us > hr)
        oss << Units::Time::FHours(us);
    else if (us > sc)
        oss << Units::Time::FSeconds(us);
    else if (us > ms)
        oss << Units::Time::FMilliseconds(us);
    else
        oss << us;

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FDurationInMs v) {
    const Units::Time::FMicroseconds us(v.Value);

    const Units::Time::FHours hr(0.8);
    const Units::Time::FSeconds sc(0.8);
    const Units::Time::FMilliseconds ms(0.8);

    if (us > hr)
        oss << Units::Time::FHours(us);
    else if (us > sc)
        oss << Units::Time::FSeconds(us);
    else if (us > ms)
        oss << Units::Time::FMilliseconds(us);
    else
        oss << us;

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        PPE::Format(oss, "0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                PPE::Format(oss, " {0:#2X}", (unsigned)hexDump.RawData[offset]);
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
        PPE::Format(oss, L"0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                PPE::Format(oss, L" {0:#2X}", (unsigned)hexDump.RawData[offset]);
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
FTextWriter& operator <<(FTextWriter& oss, const Fmt::TBasicIndent<char>& indent) {
    Assert(indent.Level >= 0);
    forrange(i, 0, indent.Level)
        oss << indent.Tab;
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::TBasicIndent<wchar_t>& indent) {
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
    case PPE::Fmt::LBrace:
        return oss << '{';
    case PPE::Fmt::RBrace:
        return oss << '}';
    case PPE::Fmt::LBracket:
        return oss << '[';
    case PPE::Fmt::RBracket:
        return oss << ']';
    case PPE::Fmt::LParenthese:
        return oss << '(';
    case PPE::Fmt::RParenthese:
        return oss << ')';
    case PPE::Fmt::Comma:
        return oss << ',';
    case PPE::Fmt::Colon:
        return oss << ':';
    case PPE::Fmt::SemiColon:
        return oss << ';';
    case PPE::Fmt::Dot:
        return oss << '.';
    case PPE::Fmt::Dollar:
        return oss << '$';
    case PPE::Fmt::Question:
        return oss << '?';
    case PPE::Fmt::Add:
        return oss << '+';
    case PPE::Fmt::Sub:
        return oss << '-';
    case PPE::Fmt::Mul:
        return oss << '*';
    case PPE::Fmt::Div:
        return oss << '/';
    case PPE::Fmt::Mod:
        return oss << '%';
    case PPE::Fmt::Pow:
        return oss << "**";
    case PPE::Fmt::Increment:
        return oss << "++";
    case PPE::Fmt::Decrement:
        return oss << "--";
    case PPE::Fmt::LShift:
        return oss << "<<";
    case PPE::Fmt::RShift:
        return oss << ">>";
    case PPE::Fmt::And:
        return oss << '&';
    case PPE::Fmt::Or:
        return oss << '|';
    case PPE::Fmt::Not:
        return oss << '!';
    case PPE::Fmt::Xor:
        return oss << '^';
    case PPE::Fmt::Complement:
        return oss << '~';
    case PPE::Fmt::Assignment:
        return oss << '=';
    case PPE::Fmt::Equals:
        return oss << "==";
    case PPE::Fmt::NotEquals:
        return oss << "!=";
    case PPE::Fmt::Less:
        return oss << '<';
    case PPE::Fmt::LessOrEqual:
        return oss << "<=";
    case PPE::Fmt::Greater:
        return oss << '>';
    case PPE::Fmt::GreaterOrEqual:
        return oss << ">=";
    case PPE::Fmt::DotDot:
        return oss << "..";
    case PPE::Fmt::Sharp:
        return oss << '#';
    case PPE::Fmt::Quote:
        return oss << '\'';
    case PPE::Fmt::DoubleQuote:
        return oss << '"';
    case PPE::Fmt::Space:
        return oss << ' ';
    case PPE::Fmt::Tab:
        return oss << ' ';
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::EChar ch) {
    switch (ch) {
    case PPE::Fmt::LBrace:
        return oss << L'{';
    case PPE::Fmt::RBrace:
        return oss << L'}';
    case PPE::Fmt::LBracket:
        return oss << L'[';
    case PPE::Fmt::RBracket:
        return oss << L']';
    case PPE::Fmt::LParenthese:
        return oss << L'(';
    case PPE::Fmt::RParenthese:
        return oss << L')';
    case PPE::Fmt::Comma:
        return oss << L',';
    case PPE::Fmt::Colon:
        return oss << L':';
    case PPE::Fmt::SemiColon:
        return oss << L';';
    case PPE::Fmt::Dot:
        return oss << L'.';
    case PPE::Fmt::Dollar:
        return oss << L'$';
    case PPE::Fmt::Question:
        return oss << L'?';
    case PPE::Fmt::Add:
        return oss << L'+';
    case PPE::Fmt::Sub:
        return oss << L'-';
    case PPE::Fmt::Mul:
        return oss << L'*';
    case PPE::Fmt::Div:
        return oss << L'/';
    case PPE::Fmt::Mod:
        return oss << L'%';
    case PPE::Fmt::Pow:
        return oss << L"**";
    case PPE::Fmt::Increment:
        return oss << L"++";
    case PPE::Fmt::Decrement:
        return oss << L"--";
    case PPE::Fmt::LShift:
        return oss << L"<<";
    case PPE::Fmt::RShift:
        return oss << L">>";
    case PPE::Fmt::And:
        return oss << L'&';
    case PPE::Fmt::Or:
        return oss << L'|';
    case PPE::Fmt::Not:
        return oss << L'!';
    case PPE::Fmt::Xor:
        return oss << L'^';
    case PPE::Fmt::Complement:
        return oss << L'~';
    case PPE::Fmt::Assignment:
        return oss << L'=';
    case PPE::Fmt::Equals:
        return oss << L"==";
    case PPE::Fmt::NotEquals:
        return oss << L"!=";
    case PPE::Fmt::Less:
        return oss << L'<';
    case PPE::Fmt::LessOrEqual:
        return oss << L"<=";
    case PPE::Fmt::Greater:
        return oss << L'>';
    case PPE::Fmt::GreaterOrEqual:
        return oss << L">=";
    case PPE::Fmt::DotDot:
        return oss << L"..";
    case PPE::Fmt::Sharp:
        return oss << L'#';
    case PPE::Fmt::Quote:
        return oss << L'\'';
    case PPE::Fmt::DoubleQuote:
        return oss << L'"';
    case PPE::Fmt::Space:
        return oss << L' ';
    case PPE::Fmt::Tab:
        return oss << L' ';
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
