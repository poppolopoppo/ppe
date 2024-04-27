// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#define EXPORT_PPE_RUNTIME_CORE_TEXTWRITER

#include "IO/TextWriter.h"

#include "IO/FormatHelpers.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Meta/Utility.h"
#include "Misc/Function.h"

#include "double-conversion-external.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static void TextWriteWFormat_(TBasicTextWriter<_Char>& w, TBasicStringView<_Char> str) {
    const FTextFormat& fmt = w.Format();
    IStreamWriter& ostream = w.Stream();

    // Handles quoting outside of every other formating:
    bool shouldQuote{ false };
    if (fmt.Misc() ^ FTextFormat::Quote) {
        shouldQuote = true;
        ostream.WritePOD(STRING_LITERAL(_Char, '"'));
    }
    DEFERRED {
        if (shouldQuote)
            ostream.WritePOD(STRING_LITERAL(_Char, '"'));
    };

    // Fast path for trivial case :
    if ((fmt.Case() == FTextFormat::Original) &&
        (fmt.Padding() == FTextFormat::Padding_None || str.size() >= fmt.Width()) &&
        (not (fmt.Misc() ^ FTextFormat::_Truncate) || str.size() <= fmt.Width()) ) {

        if (fmt.Misc() & FTextFormat::Escape)
            Escape(w, str, EEscape::Unicode);
        else
            ostream.WriteView(str);

        // Reset width padding each output to mimic std behavior
        w.Format().SetPadding(FTextFormat::Padding_None);
        return;
    }

    const int sz = checked_cast<int>(str.size());
    const int width = checked_cast<int>(w.Format().Width());
    const _Char pad_ch = w.FillChar();

    // Handles truncate :
    if ((!!width) && (fmt.Misc() ^ FTextFormat::_Truncate) && (str.size() > static_cast<size_t>(width))) {
        if (fmt.Misc() & FTextFormat::TruncateR)
            str = str.CutBefore(width);
        else if (fmt.Misc() & FTextFormat::TruncateL)
            str = str.CutStartingAt(str.size() - width);
        else
            AssertNotImplemented();
    }

    // Handles padding & case :
    int pad_left;
    int pad_right;
    switch (fmt.Padding()) {
    case PPE::FTextFormat::Padding_None:
        pad_left = pad_right = 0;
        break;
    case PPE::FTextFormat::Padding_Center:
        pad_left = Max(width - sz, 0) / 2;
        pad_right = Max(width - pad_left - sz, 0);
        break;
    case PPE::FTextFormat::Padding_Left:
        pad_left = Max(width - sz, 0);
        pad_right = 0;
        break;
    case PPE::FTextFormat::Padding_Right:
        pad_left = 0;
        pad_right = Max(width - sz, 0);
        break;
    default:
        AssertNotImplemented();
        break;
    }

    // Don't use alloca() since it will break RelocateAlloca() for alloca streams

    forrange(i, 0, pad_left)
        ostream.WritePOD(pad_ch);

    switch (fmt.Case()) {
    case PPE::FTextFormat::Original:
        if (fmt.Misc() & FTextFormat::Escape)
            Escape(w, str, EEscape::Unicode);
        else
            ostream.WriteView(str);
        break;
    case PPE::FTextFormat::Lowercase:
        foreachitem(ch, str)
            ostream.WritePOD(ToLower(*ch));
        break;
    case PPE::FTextFormat::Uppercase:
        foreachitem(ch, str)
            ostream.WritePOD(ToUpper(*ch));
        break;
    case PPE::FTextFormat::Capitalize:
        {
            bool to_lower = false;
            foreachitem(ch, str) {
                ostream.WritePOD(to_lower ? ToLower(*ch) : ToUpper(*ch));
                to_lower = IsAlnum(*ch);
            }
        }
        break;
    default:
        AssertNotImplemented();
        break;
    }

    forrange(i, 0, pad_right)
        ostream.WritePOD(pad_ch);

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
STATIC_CONST_INTEGRAL(size_t, GItoaCapacity_, 64+1/* <=> 64 bits for base 2 */);
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
    const auto fullbase = FullBaseStr_(Meta::Type<_Char>);

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
NO_INLINE static size_t ItoaBase_(u64 v, const TMemoryView<_Char>& buffer) {
    const auto fullbase = FullBaseStr_(Meta::Type<_Char>);

    const auto len = BaseDigits_<_Base>(v);
    Assert(len <= buffer.size());

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
static size_t Itoa_(u64 v, u64 base, const TMemoryView<_Char>& buffer) {
    switch (base) {
    case 2:
        return ItoaBase_<_Char, 2>(v, buffer);
    case 8:
        return ItoaBase_<_Char, 8>(v, buffer);
    case 10:
        return ItoaBase_<_Char, 10>(v, buffer);
    case 16:
        return ItoaBase_<_Char, 16>(v, buffer);
    default:
        AssertNotImplemented();
    }
}
#endif
//----------------------------------------------------------------------------
template <typename _Char>
static void TextWriteItoaUnsigned_(TBasicTextWriter<_Char>& w, u64 v) {
    _Char buffer[GItoaCapacity_];
    const u64 base = GBasesU64_[w.Format().Base()];
    const size_t len = Itoa_(v, base, MakeView(buffer));
    TextWriteWFormat_(w, TBasicStringView<_Char>(buffer, len));
}
//----------------------------------------------------------------------------
template <typename _Char>
static void TextWriteItoaSigned_(TBasicTextWriter<_Char>& w, i64 v) {
    _Char buffer[GItoaCapacity_];
    const u64 base = GBasesU64_[w.Format().Base()];
    size_t len;
    if (v < 0) {
        buffer[0] = NegChar_(Meta::Type<_Char>);
        len = (Itoa_(checked_cast<u64>(-v), base, MakeView(buffer).CutStartingAt(1)) + 1);
    }
    else {
        len = Itoa_(checked_cast<u64>(v), base, MakeView(buffer));
    }
    TextWriteWFormat_(w, TBasicStringView<_Char>(buffer, len));
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
    case PPE::FTextFormat::EFloat::DefaultFloat:
        if (not Dtoa_DefaultFloat_(double_conv, str_builder, flt))
            AssertNotReached();
        break;
    case PPE::FTextFormat::EFloat::FixedFloat:
        if (not double_conv.ToFixed(double(flt), checked_cast<int>(fmt.Precision()), &str_builder))
            AssertNotReached();
        break;
    case PPE::FTextFormat::EFloat::PrecisionFloat:
        if (not double_conv.ToPrecision(double(flt), checked_cast<int>(fmt.Precision()), &str_builder))
            AssertNotReached();
        break;
    case PPE::FTextFormat::EFloat::ScientificFloat:
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
static void TextWriteDtoa_(TBasicTextWriter<char>& w, _Flt flt) {
    char buffer[GDtoaCapacity_];
    TextWriteWFormat_(w, FStringView(buffer, Dtoa_(buffer, w.Format(), flt)));
}
//----------------------------------------------------------------------------
template <typename _Flt>
static void TextWriteDtoa_(TBasicTextWriter<wchar_t>& w, _Flt flt) {
    char ascii[GDtoaCapacity_];
    wchar_t wide[GDtoaCapacity_];
    const size_t len = Dtoa_(ascii, w.Format(), flt);
    const size_t len2 = ToWCStr(wide, GDtoaCapacity_, ascii, len);
    Assert(len2 == len + 1/* final \0 */);
    TextWriteWFormat_(w, FWStringView(wide, len2 - 1/* final \0 */));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TextWrite_(Meta::TType<char>, TBasicTextWriter<char>& w, bool v) {
    TextWriteWFormat_(w, (w.Format().Misc() ^ FTextFormat::BoolAlpha)
        ? (v ? MakeStringView("true") : MakeStringView("false"))
        : (v ? MakeStringView("1") : MakeStringView("0")) );
}
//----------------------------------------------------------------------------
static void TextWrite_(Meta::TType<wchar_t>, TBasicTextWriter<wchar_t>& w, bool v) {
    TextWriteWFormat_(w, (w.Format().Misc() ^ FTextFormat::BoolAlpha)
        ? (v ? MakeStringView(L"true") : MakeStringView(L"false"))
        : (v ? MakeStringView(L"1") : MakeStringView(L"0")));
}
//----------------------------------------------------------------------------
static void TextWrite_(Meta::TType<char>, FTextWriter& w, const void* v) {
    w << Fmt::Pointer(v);
}
//----------------------------------------------------------------------------
static void TextWrite_(Meta::TType<wchar_t>, FWTextWriter& w, const void* v) {
    w << Fmt::Pointer(v);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBaseTextWriter::FBaseTextWriter(TPtrRef<IStreamWriter> ostream) NOEXCEPT
:   _ostream(ostream) {
    Assert(_ostream);
}
//----------------------------------------------------------------------------
FBaseTextWriter::~FBaseTextWriter() NOEXCEPT {
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
    if (IBufferedStreamWriter* const buffered = _ostream->ToBufferedO())
        buffered->Flush();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBaseTextWriter::BasePut(TBasicTextWriter<char>* oss, char v) { oss->Stream().WritePOD(v); }
void FBaseTextWriter::BasePut(TBasicTextWriter<wchar_t>* oss, wchar_t v) { oss->Stream().WritePOD(v); }
//----------------------------------------------------------------------------
void FBaseTextWriter::BasePut(TBasicTextWriter<char>* oss, const TBasicStringView<char>& str) { oss->Stream().WriteView(str); }
void FBaseTextWriter::BasePut(TBasicTextWriter<wchar_t>* oss, const TBasicStringView<wchar_t>& str) { oss->Stream().WriteView(str); }
//----------------------------------------------------------------------------
void FBaseTextWriter::BasePut(TBasicTextWriter<char>* oss, const TBasicStringLiteral<char>& str) { oss->Stream().WriteView(str.MakeView()); }
void FBaseTextWriter::BasePut(TBasicTextWriter<wchar_t>* oss, const TBasicStringLiteral<wchar_t>& str) { oss->Stream().WriteView(str.MakeView()); }
//----------------------------------------------------------------------------
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, bool v) { TextWrite_(Meta::Type<char>, *oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, i8 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, i16 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, i32 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, i64 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, u8 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, u16 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, u32 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, u64 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, float v) { TextWriteDtoa_(*oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, double v) { TextWriteDtoa_(*oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, const void* v) { TextWrite_(Meta::Type<char>, *oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, const char* v) { TextWriteWFormat_(*oss, MakeCStringView(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, const TBasicStringView<char>& v) { TextWriteWFormat_(*oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<char>* oss, const TBasicStringLiteral<char>& v) { TextWriteWFormat_(*oss, v.MakeView()); }
//----------------------------------------------------------------------------
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, bool v) { TextWrite_(Meta::Type<wchar_t>, *oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, i8 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, i16 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, i32 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, i64 v) { TextWriteItoaSigned_(*oss, i64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, u8 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, u16 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, u32 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, u64 v) { TextWriteItoaUnsigned_(*oss, u64(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, float v) { TextWriteDtoa_(*oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, double v) { TextWriteDtoa_(*oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, const void* v) { TextWrite_(Meta::Type<wchar_t>, *oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, const wchar_t* v) { TextWriteWFormat_(*oss, MakeCStringView(v)); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, const TBasicStringView<wchar_t>& v) { TextWriteWFormat_(*oss, v); }
void FBaseTextWriter::BaseWrite(TBasicTextWriter<wchar_t>* oss, const TBasicStringLiteral<wchar_t>& v) { TextWriteWFormat_(*oss, v.MakeView()); }
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicTextWriter<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicTextWriter<wchar_t>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TFunction<TBasicTextWriter<char>& (TBasicTextWriter<char>&)>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TFunction<TBasicTextWriter<wchar_t>& (TBasicTextWriter<wchar_t>&)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& TextFormat_DontPad_(TBasicTextWriter<_Char>& s) {
    s.Format().SetPadding(FTextFormat::Padding_None);
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> TextFormat_Pad_(FTextFormat::EPadding padding, size_t width, _Char fill) {
    return [padding, width, fill](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char> & {
        s.Format().SetPadding(padding);

        if (INDEX_NONE != width)
            s.Format().SetWidth(width);

        if (_Char() != fill)
            s.SetFillChar(fill);

        return s;
    };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> TextFormat_SetFill_(_Char ch) {
    return [ch](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char> & {
        s.SetFillChar(ch);
        return s;
    };
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& FTextFormat::DontPad(FTextWriter& s) {
    return TextFormat_DontPad_(s);
}
//----------------------------------------------------------------------------
FTextManipulator FTextFormat::Pad(EPadding padding, size_t width, char fill) {
    return TextFormat_Pad_(padding, width, fill);
}
//----------------------------------------------------------------------------
FTextManipulator FTextFormat::PadCenter(size_t width, char fill) {
    return TextFormat_Pad_(Padding_Center, width, fill);
}
//----------------------------------------------------------------------------
FTextManipulator FTextFormat::PadLeft(size_t width, char fill) {
    return TextFormat_Pad_(Padding_Left, width, fill);
}
//----------------------------------------------------------------------------
FTextManipulator FTextFormat::PadRight(size_t width, char fill) {
    return TextFormat_Pad_(Padding_Right, width, fill);
}
//----------------------------------------------------------------------------
FTextManipulator FTextFormat::SetFill(char ch) {
    return TextFormat_SetFill_(ch);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWTextWriter& FTextFormat::DontPad(FWTextWriter& s) {
    return TextFormat_DontPad_(s);
}
//----------------------------------------------------------------------------
FWTextManipulator FTextFormat::Pad(EPadding padding, size_t width, wchar_t fill) {
    return TextFormat_Pad_(padding, width, fill);
}
//----------------------------------------------------------------------------
FWTextManipulator FTextFormat::PadCenter(size_t width, wchar_t fill) {
    return TextFormat_Pad_(Padding_Center, width, fill);
}
//----------------------------------------------------------------------------
FWTextManipulator FTextFormat::PadLeft(size_t width, wchar_t fill) {
    return TextFormat_Pad_(Padding_Left, width, fill);
}
//----------------------------------------------------------------------------
FWTextManipulator FTextFormat::PadRight(size_t width, wchar_t fill) {
    return TextFormat_Pad_(Padding_Right, width, fill);
}
//----------------------------------------------------------------------------
FWTextManipulator FTextFormat::SetFill(wchar_t ch) {
    return TextFormat_SetFill_(ch);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& writer, const FTextManipulator& manip) {
    return manip(writer);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& writer, const FWTextManipulator& manip) {
    return manip(writer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
