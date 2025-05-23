﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/FormatHelpers.h"

#include "IO/Format.h"
#include "IO/StreamProvider.h"

#include "Maths/Units.h"
#include "Memory/MemoryProvider.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FOffset off) {
    return (*oss.FormatScope())
        << "@0x"
        << FTextFormat::Hexadecimal
        << FTextFormat::PadLeft(sizeof(off.Value) << 1, '0')
        << off.Value;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FOffset off);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::FPointer ptr) {
    return (*oss.FormatScope())
        << "[@0x"
        << FTextFormat::Hexadecimal
        << FTextFormat::PadLeft(sizeof(ptr) << 1, '0')
        << uintptr_t(ptr.Value)
        << ']';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPointer ptr) {
    return (*oss.FormatScope())
        << L"[@0x"
        << FTextFormat::Hexadecimal
        << FTextFormat::PadLeft(sizeof(ptr) << 1, L'0')
        << size_t(ptr.Value)
        << L']';
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

    const float fCount = static_cast<float>(count.Value);

    if (fCount > 9e5f)
        oss << (fCount / 1e6f) << " M";
    else if (fCount > 9e2f)
        oss << (fCount / 1e3f) << " K";
    else
        oss << count.Value << "  "/* for alignment */;

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FCountOfElements count) {
    if (oss.Format().Width() > 1) // fixes alignment for units
        oss.Format().SetWidth(oss.Format().Width() - 2);

    const float fCount = static_cast<float>(count.Value);

    if (fCount > 9e5f)
        oss << (fCount / 1e6f) << L" M";
    else if (fCount > 9e2f)
        oss << (fCount / 1e3f) << L" K";
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
template <typename _Char>
static TBasicTextWriter<_Char>& HexDump_(TBasicTextWriter<_Char>& oss, const Fmt::FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    Assert(!!hexDump.RawData.data() || totalBytes == 0);

    for (uintptr_t offset = 0; offset < totalBytes; ) {
        PPE::Format(oss, STRING_LITERAL(_Char, "0x{0:#8X} "),
            hexDump.Absolute ? reinterpret_cast<uintptr_t>(hexDump.RawData.data() + offset) : offset);

        const size_t origin = offset;
        for (u32 row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                PPE::Format(oss, STRING_LITERAL(_Char, " {0:#2X}"), (unsigned)hexDump.RawData[offset]);
            else
                oss << STRING_LITERAL(_Char, "   ");
        }

        oss << STRING_LITERAL(_Char, "  ");
        offset = origin;

        for (u32 row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) && (char(hexDump.RawData[offset]) != '\t')
                ? _Char(hexDump.RawData[offset]) : STRING_LITERAL(_Char, '.'));

        oss << Eol;
    }

    return oss;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FHexDump& hexDump) {
    return HexDump_(oss, hexDump);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FHexDump& hexDump) {
    return HexDump_(oss, hexDump);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {;
//----------------------------------------------------------------------------
template <typename _Char>
constexpr _Char Base64_padding = STRING_LITERAL(_Char, '=');
//----------------------------------------------------------------------------
template <typename _Char>
static void Base64Encode_(const FRawMemoryConst& src, const TAppendable<_Char>& dst) NOEXCEPT {
    constexpr TBasicStringLiteral<_Char> lookup{
        STRING_LITERAL(_Char, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/") };

    u32 tmp;
    auto it = src.begin();
    forrange(i, 0, src.size() / 3u) {
        tmp  = (*it++) << 16u; // convert to big endian
        tmp += (*it++) << 8u;
        tmp += (*it++);

        dst.push_back(lookup[(tmp & 0x00FC0000u) >> 18u]);
        dst.push_back(lookup[(tmp & 0x0003F000u) >> 12u]);
        dst.push_back(lookup[(tmp & 0x00000FC0u) >> 6u ]);
        dst.push_back(lookup[(tmp & 0x0000003Fu)       ]);
    }

    switch (src.size() % 3u) {
    case 1:
        tmp  = (*it++) << 16u; // convert to big endian
        dst.push_back(lookup[(tmp & 0x00FC0000u) >> 18u]);
        dst.push_back(lookup[(tmp & 0x0003F000u) >> 12u]);
        dst.push_back(Base64_padding<_Char>);
        dst.push_back(Base64_padding<_Char>);
        break;
    case 2:
        tmp  = (*it++) << 16u; // convert to big endian
        tmp += (*it++) << 8u;
        dst.push_back(lookup[(tmp & 0x00FC0000u) >> 18u]);
        dst.push_back(lookup[(tmp & 0x0003F000u) >> 12u]);
        dst.push_back(lookup[(tmp & 0x00000FC0u) >> 6u ]);
        dst.push_back(Base64_padding<_Char>);
        break;
    default:
        break;
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool Base64Decode_(const TBasicStringView<_Char>& src, const TAppendable<u8>& dst) NOEXCEPT {
    if (src.size() % 4)
        return false;

    u32 tmp = 0; // holds decoded quanta
    for (auto it = src.begin(); it != src.end(); ) {
        forrange(quantum, 0, 4) {
            tmp <<= 6u;

            if       (*it >= STRING_LITERAL(_Char, 'A') && *it <= STRING_LITERAL(_Char, 'Z')) // This area will need tweaking if
                tmp |= *it - STRING_LITERAL(_Char, 'A');                 // you are using an alternate alphabet
            else if  (*it >= STRING_LITERAL(_Char, 'a') && *it <= STRING_LITERAL(_Char, 'z'))
                tmp |= *it - STRING_LITERAL(_Char, 'G');
            else if  (*it >= STRING_LITERAL(_Char, '0') && *it <= STRING_LITERAL(_Char, '9'))
                tmp |= *it + 0x04u;
            else if  (*it == STRING_LITERAL(_Char, '+'))
                tmp |= 0x3Eu; // change to 0x2D for URL alphabet
            else if  (*it == STRING_LITERAL(_Char, '/'))
                tmp |= 0x3Fu; // change to 0x5F for URL alphabet
            else if  (*it == Base64_padding<_Char>) {//pad
                switch (src.end() - it) {
                case 1: // one pad character
                    dst.push_back(static_cast<u8>((tmp >> 16u) & 0x000000FFu));
                    dst.push_back(static_cast<u8>((tmp >> 8u ) & 0x000000FFu));
                    return true;
                case 2: //Two pad characters
                    dst.push_back(static_cast<u8>((tmp >> 10u) & 0x000000FFu));
                    return true;
                default:
                    return false;
                }
            }  else
                return false;

            ++it;
        }

        dst.push_back(static_cast<u8>((tmp >> 16u) & 0x000000FFu));
        dst.push_back(static_cast<u8>((tmp >> 8u ) & 0x000000FFu));
        dst.push_back(static_cast<u8>((tmp       ) & 0x000000FFu));
    }
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
static size_t Base64DecodeSize_(const TBasicStringView<_Char>& src) {
    if (src.size() % 4u)
        return 0;

    size_t padding = 0u;
    if (not src.empty()) {
        if (src[src.size() - 1] == Base64_padding<_Char>)
            padding++;
        if (src[src.size() - 2] == Base64_padding<_Char>)
            padding++;
    }

    return ((src.size() / 4u) * 3u - padding);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
size_t Base64EncodeSize(const FRawMemoryConst& src) NOEXCEPT {
    return (((src.size() / 3) + (src.size() % 3 > 0)) * 4);
}
//----------------------------------------------------------------------------
void Base64Encode(const FRawMemoryConst& src, const TAppendable<char>& dst) NOEXCEPT {
    Base64Encode_(src, dst);
}
//----------------------------------------------------------------------------
void Base64Encode(const FRawMemoryConst& src, const TAppendable<wchar_t>& dst) NOEXCEPT {
    Base64Encode_(src, dst);
}
//----------------------------------------------------------------------------
void Base64Encode(const FRawMemoryConst& src, const TMemoryView<char>& dst) NOEXCEPT {
    Assert(Base64EncodeSize(src) == dst.size());

    char* it = dst.data();
    Base64Encode_(src, TAppendable<char>{ &it, [](void* user, char&& ch) {
        *(*static_cast<char**>(user))++ = ch;
    } });
    Assert_NoAssume(it == dst.data() + dst.size());
}
//----------------------------------------------------------------------------
void Base64Encode(const FRawMemoryConst& src, const TMemoryView<wchar_t>& dst) NOEXCEPT {
    Assert(Base64EncodeSize(src) == dst.size());

    wchar_t* it = dst.data();
    Base64Encode_(src, TAppendable<wchar_t>{ &it, [](void* user, wchar_t&& ch) {
        *(*static_cast<wchar_t**>(user))++ = ch;
    } });
    Assert_NoAssume(it == dst.data() + dst.size());
}
//----------------------------------------------------------------------------
size_t Base64DecodeSize(const FStringView& src) NOEXCEPT {
    return Base64DecodeSize_(src);
}
//----------------------------------------------------------------------------
size_t Base64DecodeSize(const FWStringView& src) NOEXCEPT {
    return Base64DecodeSize_(src);
}
//----------------------------------------------------------------------------
bool Base64Decode(const FStringView& src, const TAppendable<u8>& dst) NOEXCEPT {
    return Base64Decode_(src, dst);
}
//----------------------------------------------------------------------------
bool Base64Decode(const FWStringView& src, const TAppendable<u8>& dst) NOEXCEPT {
    return Base64Decode_(src, dst);
}
//----------------------------------------------------------------------------
bool Base64Decode(const FStringView& src, const TMemoryView<u8>& dst) NOEXCEPT {
    Assert(Base64DecodeSize(src) == dst.size());

    u8* it = dst.data();
    const bool result = Base64Decode_(src, TAppendable<u8>{ &it, [](void* user, u8&& raw) {
        *(*static_cast<u8**>(user))++ = raw;
    } });
    Assert_NoAssume(it == dst.data() + dst.size());
    return result;
}
//----------------------------------------------------------------------------
bool Base64Decode(const FWStringView& src, const TMemoryView<u8>& dst) NOEXCEPT {
    Assert(Base64DecodeSize(src) == dst.size());

    u8* it = dst.data();
    const bool result = Base64Decode_(src, TAppendable<u8>{ &it, [](void* user, u8&& raw) {
        *(*static_cast<u8**>(user))++ = raw;
    } });
    Assert_NoAssume(it == dst.data() + dst.size());
    return result;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FBase64& b64) {
    Base64Encode(b64.RawData, { &oss, [](void* user, char&& ch) {
        static_cast<FTextWriter*>(user)->Put(ch);
    }});
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FBase64& b64) {
    Base64Encode(b64.RawData, { &oss, [](void* user, wchar_t&& wch) {
        static_cast<FWTextWriter*>(user)->Put(wch);
    }});
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
template <typename _Char>
static TBasicTextWriter<_Char>& FormatChar_(TBasicTextWriter<_Char>& oss, Fmt::EChar ch) {
    switch (ch) {
    case PPE::Fmt::LBrace:
        return oss << STRING_LITERAL(_Char, '{');
    case PPE::Fmt::RBrace:
        return oss << STRING_LITERAL(_Char, '}');
    case PPE::Fmt::LBracket:
        return oss << STRING_LITERAL(_Char, '[');
    case PPE::Fmt::RBracket:
        return oss << STRING_LITERAL(_Char, ']');
    case PPE::Fmt::LParenthese:
        return oss << STRING_LITERAL(_Char, '(');
    case PPE::Fmt::RParenthese:
        return oss << STRING_LITERAL(_Char, ')');
    case PPE::Fmt::Comma:
        return oss << STRING_LITERAL(_Char, ',');
    case PPE::Fmt::Colon:
        return oss << STRING_LITERAL(_Char, ':');
    case PPE::Fmt::SemiColon:
        return oss << STRING_LITERAL(_Char, ';');
    case PPE::Fmt::Dot:
        return oss << STRING_LITERAL(_Char, '.');
    case PPE::Fmt::Dollar:
        return oss << STRING_LITERAL(_Char, '$');
    case PPE::Fmt::Question:
        return oss << STRING_LITERAL(_Char, '?');
    case PPE::Fmt::Add:
        return oss << STRING_LITERAL(_Char, '+');
    case PPE::Fmt::Sub:
        return oss << STRING_LITERAL(_Char, '-');
    case PPE::Fmt::Mul:
        return oss << STRING_LITERAL(_Char, '*');
    case PPE::Fmt::Div:
        return oss << STRING_LITERAL(_Char, '/');
    case PPE::Fmt::Mod:
        return oss << STRING_LITERAL(_Char, '%');
    case PPE::Fmt::Pow:
        return oss << STRING_LITERAL(_Char, "**");
    case PPE::Fmt::Increment:
        return oss << STRING_LITERAL(_Char, "++");
    case PPE::Fmt::Decrement:
        return oss << STRING_LITERAL(_Char, "--");
    case PPE::Fmt::LShift:
        return oss << STRING_LITERAL(_Char, "<<");
    case PPE::Fmt::RShift:
        return oss << STRING_LITERAL(_Char, ">>");
    case PPE::Fmt::And:
        return oss << STRING_LITERAL(_Char, '&');
    case PPE::Fmt::Or:
        return oss << STRING_LITERAL(_Char, '|');
    case PPE::Fmt::Not:
        return oss << STRING_LITERAL(_Char, '!');
    case PPE::Fmt::Xor:
        return oss << STRING_LITERAL(_Char, '^');
    case PPE::Fmt::Complement:
        return oss << STRING_LITERAL(_Char, '~');
    case PPE::Fmt::Assignment:
        return oss << STRING_LITERAL(_Char, '=');
    case PPE::Fmt::Equals:
        return oss << STRING_LITERAL(_Char, "==");
    case PPE::Fmt::NotEquals:
        return oss << STRING_LITERAL(_Char, "!=");
    case PPE::Fmt::Less:
        return oss << STRING_LITERAL(_Char, '<');
    case PPE::Fmt::LessOrEqual:
        return oss << STRING_LITERAL(_Char, "<=");
    case PPE::Fmt::Greater:
        return oss << STRING_LITERAL(_Char, '>');
    case PPE::Fmt::GreaterOrEqual:
        return oss << STRING_LITERAL(_Char, ">=");
    case PPE::Fmt::DotDot:
        return oss << STRING_LITERAL(_Char, "..");
    case PPE::Fmt::Sharp:
        return oss << STRING_LITERAL(_Char, '#');
    case PPE::Fmt::Quote:
        return oss << STRING_LITERAL(_Char, '\'');
    case PPE::Fmt::DoubleQuote:
        return oss << STRING_LITERAL(_Char, '"');
    case PPE::Fmt::Space:
        return oss << STRING_LITERAL(_Char, ' ');
    case PPE::Fmt::Tab:
        return oss << STRING_LITERAL(_Char, '\t');
    case PPE::Fmt::Zero:
        return oss << STRING_LITERAL(_Char, '0');
    case PPE::Fmt::A:
        return oss << STRING_LITERAL(_Char, 'A');
    case PPE::Fmt::a:
        return oss << STRING_LITERAL(_Char, 'a');
    case PPE::Fmt::Eol:
        return oss << Eol;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Fmt::EChar ch) {
    return FormatChar_(oss, ch);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Fmt::EChar ch) {
    return FormatChar_(oss, ch);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
