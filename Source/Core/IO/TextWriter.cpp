#include "stdafx.h"

#include "TextWriter.h"

#include "StreamProvider.h"
#include "String.h"
#include "StringView.h"

#include "Core.External/double-conversion-external.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static void WriteWFormat_(TBasicTextWriter<_Char>& w, const TBasicStringView<_Char>& str) {
    const FTextFormat& fmt = w.Format();
    IBufferedStreamWriter* ostream = w.Stream();

    // Fast path for trivial case :
    if (fmt.Case() == FTextFormat::Original &&
        (fmt.Padding() == FTextFormat::Padding_None || str.size() >= w.Format().Width()) ) {
        ostream->WriteView(str);

        // Reset width padding each output to mimic std behavior
        w.Format().SetPadding(FTextFormat::Padding_None);
        return;
    }

    // Handles padding & case :
    const int sz = checked_cast<int>(str.size());
    const int width = checked_cast<int>(w.Format().Width());
    const _Char pad_ch = w.FillChar();

    int pad_left;
    int pad_right;
    switch (fmt.Padding()) {
    case Core::FTextFormat::Padding_None:
        pad_left = pad_right = 0;
        break;
    case Core::FTextFormat::Padding_Center:
        pad_left = Max(width - sz, 0) / 2;
        pad_right = Max(width - pad_left - sz, 0);
        break;
    case Core::FTextFormat::Padding_Left:
        pad_left = Max(width - sz, 0);
        pad_right = 0;
        break;
    case Core::FTextFormat::Padding_Right:
        pad_left = 0;
        pad_right = Max(width - sz, 0);;
        break;
    default:
        AssertNotImplemented();
        break;
    }

    // Don't use alloca() since it will break RelocateAlloca() for alloca streams

    forrange(i, 0, pad_left)
        ostream->WritePOD(pad_ch);

    switch (fmt.Case()) {
    case Core::FTextFormat::Original:
        ostream->WriteView(str);
        break;
    case Core::FTextFormat::Lowercase:
        foreachitem(ch, str)
            ostream->WritePOD(ToLower(*ch));
        break;
    case Core::FTextFormat::Uppercase:
        foreachitem(ch, str)
            ostream->WritePOD(ToUpper(*ch));
        break;
    case Core::FTextFormat::Capitalize:
        {
            bool to_lower = false;
            foreachitem(ch, str) {
                ostream->WritePOD(to_lower ? ToLower(*ch) : ToUpper(*ch));
                to_lower = IsAlnum(*ch);
            }
        }
        break;
    default:
        AssertNotImplemented();
        break;
    }

    forrange(i, 0, pad_right)
        ostream->WritePOD(pad_ch);

    // Reset width padding each output to mimic std behavior
    w.Format().SetPadding(FTextFormat::Padding_None);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GItoaCapacity_, 24);
//----------------------------------------------------------------------------
constexpr u64 GBasesU64_[] = { 10, 2, 16, 8 };
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
using TFullBaseStr_ = const _Char(&)[_Dim]; // Force to interpret as static char array
static constexpr char NegChar_(Meta::TType<char>) { return '-'; }
static constexpr wchar_t NegChar_(Meta::TType<wchar_t>) { return L'-'; }
static constexpr TFullBaseStr_<char, 17> FullBaseStr_(Meta::TType<char>) { return "0123456789abcdef"; }
static constexpr TFullBaseStr_<wchar_t, 17> FullBaseStr_(Meta::TType<wchar_t>) { return L"0123456789abcdef"; }
//----------------------------------------------------------------------------
#if 0
template <typename _Char>
static size_t Itoa_(TBasicTextWriter<_Char>& w, u64 v, u64 base, const TMemoryView<_Char>& buffer) {
    const auto fullbase = FullBaseStr_(Meta::TType<_Char>{});

    size_t len = 0;
    do {
        const u64 d = (v % base);
        buffer[len++] = fullbase[d];
        v /= base;
    } while (v);

    std::reverse(buffer.begin(), buffer.begin() + len);
    return len;
}
#else
// From Facebook folly library, implemented by Andrei Alexandrescu
// https://github.com/facebook/folly/blob/master/folly/Conv.h
template <u64 _Base>
FORCE_INLINE static u32 BaseDigits_(u64 v) {
    static constexpr u64 GBase1 = (_Base);
    static constexpr u64 GBase2 = (_Base * _Base);
    static constexpr u64 GBase3 = (_Base * _Base * _Base);
    static constexpr u64 GBase4 = (_Base * _Base * _Base * _Base);

    u32 result = 1;
    while (true) {
        if (Likely(v < GBase1))
            return result;
        if (Likely(v < GBase2))
            return result + 1;
        if (Likely(v < GBase3))
            return result + 2;
        if (Likely(v < GBase4))
            return result + 3;

        // Skip ahead by 4 orders of magnitude
        v /= GBase4;
        result += 4;
    }
}
template <typename _Char, u64 _Base>
NO_INLINE static size_t ItoaBase_(TBasicTextWriter<_Char>& w, u64 v, const TMemoryView<_Char>& buffer) {
    const auto fullbase = FullBaseStr_(Meta::TType<_Char>{});

    const auto len = BaseDigits_<_Base>(v);

    // WARNING: using size_t or pointer arithmetic for pos slows down
    // the loop below 20x. This is because several 32-bit ops can be
    // done in parallel, but only fewer 64-bit ones.
    u32 pos = len - 1;
    while (v >= _Base) {
        auto const q = v / _Base;
        auto const r = v % _Base;
        buffer[pos--] = fullbase[r];
        v = q;
    }

    // Last digit is trivial to handle
    buffer[pos] = fullbase[v];
    return len;
}
template <typename _Char>
static size_t Itoa_(TBasicTextWriter<_Char>& w, u64 v, u64 base, const TMemoryView<_Char>& buffer) {
    switch (base) {
    case 2:
        return ItoaBase_<_Char, 2>(w, v, buffer);
    case 8:
        return ItoaBase_<_Char, 8>(w, v, buffer);
    case 10:
        return ItoaBase_<_Char, 10>(w, v, buffer);
    case 16:
        return ItoaBase_<_Char, 16>(w, v, buffer);
    default:
        AssertNotImplemented();
        return INDEX_NONE;
    }
}
#endif
//----------------------------------------------------------------------------
template <typename _Char>
static void WriteItoa_(TBasicTextWriter<_Char>& w, u64 v) {
    _Char buffer[GItoaCapacity_];
    const u64 base = GBasesU64_[w.Format().Base()];
    const size_t len = Itoa_(w, v, base, MakeView(buffer));
    WriteWFormat_(w, TBasicStringView<_Char>(buffer, len));
}
//----------------------------------------------------------------------------
template <typename _Char>
static void WriteItoa_(TBasicTextWriter<_Char>& w, i64 v) {
    _Char buffer[GItoaCapacity_];
    const u64 base = GBasesU64_[w.Format().Base()];
    size_t len;
    if (v < 0) {
        buffer[0] = NegChar_(Meta::TType<_Char>{});
        len = (Itoa_(w, checked_cast<u64>(-v), base, MakeView(buffer).CutStartingAt(1)) + 1);
    }
    else {
        len = Itoa_(w, checked_cast<u64>(v), base, MakeView(buffer));
    }
    WriteWFormat_(w, TBasicStringView<_Char>(buffer, len));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GDtoaCapacity_, 32);
//----------------------------------------------------------------------------
static bool Dtoa_DefaultFloat_(
    const double_conversion::DoubleToStringConverter& double_conv,
    double_conversion::StringBuilder& str_builder,
    float f ) {
    return double_conv.ToShortestSingle(f, &str_builder);
}
//----------------------------------------------------------------------------
static bool Dtoa_DefaultFloat_(
    const double_conversion::DoubleToStringConverter& double_conv,
    double_conversion::StringBuilder& str_builder,
    double d ) {
    return double_conv.ToShortest(d, &str_builder);
}
//----------------------------------------------------------------------------
template <typename _Flt>
static size_t Dtoa_(const TMemoryView<char>& buffer, const FTextFormat& fmt, _Flt flt) {
    STATIC_ASSERT(std::is_floating_point<_Flt>::value);
    double_conversion::StringBuilder str_builder(buffer.data(), checked_cast<int>(buffer.size()));
    auto& double_conv = double_conversion::DoubleToStringConverter::EcmaScriptConverter();
    switch (fmt.Float()) {
    case Core::FTextFormat::EFloat::DefaultFloat:
        if (not Dtoa_DefaultFloat_(double_conv, str_builder, flt))
            AssertNotReached();
        break;
    case Core::FTextFormat::EFloat::FixedFloat:
        if (not double_conv.ToFixed(double(flt), checked_cast<int>(fmt.Precision()), &str_builder))
            AssertNotReached();
        break;
    case Core::FTextFormat::EFloat::PrecisionFloat:
        if (not double_conv.ToPrecision(double(flt), checked_cast<int>(fmt.Precision()), &str_builder))
            AssertNotReached();
        break;
    case Core::FTextFormat::EFloat::ScientificFloat:
        if (not double_conv.ToExponential(double(flt), checked_cast<int>(fmt.Precision()), &str_builder))
            AssertNotReached();
        break;
    default:
        AssertNotImplemented();
        break;
    }
    return checked_cast<size_t>(str_builder.position());
}
//----------------------------------------------------------------------------
template <typename _Flt>
static void WriteDtoa_(TBasicTextWriter<char>& w, _Flt flt) {
    char buffer[GDtoaCapacity_];
    WriteWFormat_(w, FStringView(buffer, Dtoa_(buffer, w.Format(), flt)));
}
//----------------------------------------------------------------------------
template <typename _Flt>
static void WriteDtoa_(TBasicTextWriter<wchar_t>& w, _Flt flt) {
    char ascii[GDtoaCapacity_];
    wchar_t wide[GDtoaCapacity_];
    const size_t len = Dtoa_(ascii, w.Format(), flt);
    const size_t len2 = ToWCStr(wide, GDtoaCapacity_, ascii, len);
    Assert(len2 == len + 1/* final \0 */);
    WriteWFormat_(w, FWStringView(wide, len2 - 1/* final \0 */));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Write_(Meta::TType<char>, TBasicTextWriter<char>& w, bool v) {
    WriteWFormat_(w, (w.Format().Misc() ^ FTextFormat::BoolAlpha)
        ? (v ? MakeStringView("true") : MakeStringView("false"))
        : (v ? MakeStringView("1") : MakeStringView("0")) );
}
//----------------------------------------------------------------------------
static void Write_(Meta::TType<wchar_t>, TBasicTextWriter<wchar_t>& w, bool v) {
    WriteWFormat_(w, (w.Format().Misc() ^ FTextFormat::BoolAlpha)
        ? (v ? MakeStringView(L"true") : MakeStringView(L"false"))
        : (v ? MakeStringView(L"1") : MakeStringView(L"0")));
}
//----------------------------------------------------------------------------
static void Write_(Meta::TType<char>, TBasicTextWriter<char>& w, const void* v) {
    const FTextFormat org = w.ResetFormat();
    const char fillChar = w.FillChar();
    w.Put("[0x");
    w << FTextFormat::Hexadecimal;
    w << FTextFormat::PadLeft(sizeof(v) << 1, '0');
    WriteItoa_(w, u64(v));
    w.Put(']');
    w.Format() = org;
    w.SetFillChar(fillChar);
}
//----------------------------------------------------------------------------
static void Write_(Meta::TType<wchar_t>, TBasicTextWriter<wchar_t>& w, const void* v) {
    const FTextFormat org = w.ResetFormat();
    const wchar_t fillChar = w.FillChar();
    w.Put(L"[0x");
    w << FTextFormat::Hexadecimal;
    w << FTextFormat::PadLeft(sizeof(v) << 1, L'0');
    WriteItoa_<wchar_t>(w, u64(v));
    w.Put(L']');
    w.Format() = org;
    w.SetFillChar(fillChar);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBaseTextWriter::FBaseTextWriter(IBufferedStreamWriter* ostream)
    : _ostream(ostream) {
    Assert(_ostream);
}
//----------------------------------------------------------------------------
FBaseTextWriter::~FBaseTextWriter() {
    //_ostream->Flush(); // let the client control Flush()
}
//----------------------------------------------------------------------------
FTextFormat FBaseTextWriter::SetFormat(const FTextFormat& fmt) {
    FTextFormat cpy = _format;
    _format = fmt;
    return cpy;
}
//----------------------------------------------------------------------------
FTextFormat FBaseTextWriter::ResetFormat() {
    return SetFormat(FTextFormat());
}
//----------------------------------------------------------------------------
void FBaseTextWriter::Flush() {
    _ostream->Flush();
}
//----------------------------------------------------------------------------
void FBaseTextWriter::Reset() {
    _ostream->SeekO(0, ESeekOrigin::Begin);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <> void TBasicTextWriter<char>::Put(char ch) { _ostream->WritePOD(ch); }
template <> void TBasicTextWriter<wchar_t>::Put(wchar_t ch) { _ostream->WritePOD(ch); }
//----------------------------------------------------------------------------
template <> void TBasicTextWriter<char>::Put(const TBasicStringView<char>& str) { _ostream->WriteView(str); }
template <> void TBasicTextWriter<wchar_t>::Put(const TBasicStringView<wchar_t>& str) { _ostream->WriteView(str); }
//----------------------------------------------------------------------------
template <> void TBasicTextWriter<char>::Write(bool v) { Write_(Meta::TType<char>{}, *this, v); }
template <> void TBasicTextWriter<char>::Write(i8 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<char>::Write(i16 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<char>::Write(i32 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<char>::Write(i64 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<char>::Write(u8 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<char>::Write(u16 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<char>::Write(u32 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<char>::Write(u64 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<char>::Write(float v) { WriteDtoa_(*this, v); }
template <> void TBasicTextWriter<char>::Write(double v) { WriteDtoa_(*this, v); }
template <> void TBasicTextWriter<char>::Write(const void* v) { Write_(Meta::TType<char>{}, *this, v); }
template <> void TBasicTextWriter<char>::Write(const char* v) { WriteWFormat_(*this, MakeCStringView(v)); }
template <> void TBasicTextWriter<char>::Write(const TBasicStringView<char>& v) { WriteWFormat_(*this, v); }
//----------------------------------------------------------------------------
template <> void TBasicTextWriter<wchar_t>::Write(bool v) { Write_(Meta::TType<wchar_t>{}, *this, v); }
template <> void TBasicTextWriter<wchar_t>::Write(i8 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(i16 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(i32 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(i64 v) { WriteItoa_(*this, i64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(u8 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(u16 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(u32 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(u64 v) { WriteItoa_(*this, u64(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(float v) { WriteDtoa_(*this, v); }
template <> void TBasicTextWriter<wchar_t>::Write(double v) { WriteDtoa_(*this, v); }
template <> void TBasicTextWriter<wchar_t>::Write(const void* v) { Write_(Meta::TType<wchar_t>{}, *this, v); }
template <> void TBasicTextWriter<wchar_t>::Write(const wchar_t* v) { WriteWFormat_(*this, MakeCStringView(v)); }
template <> void TBasicTextWriter<wchar_t>::Write(const TBasicStringView<wchar_t>& v) { WriteWFormat_(*this, v); }
//----------------------------------------------------------------------------
/*extern CORE_API*/ template class TBasicTextWriter<char>;
/*extern CORE_API*/ template class TBasicTextWriter<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
