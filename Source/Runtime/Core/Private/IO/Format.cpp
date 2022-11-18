#include "stdafx.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
struct TFormatTraits_ {};
//----------------------------------------------------------------------------
template <>
struct TFormatTraits_<char> {
    enum EFlags : char {
        null        = '\0',
        lbrace      = '{',
        rbrace      = '}',
        colon       = ':',
        multiply    = '*',
        zero        = '0',
        fmt_alpha   = 'a',
        fmt_ALPHA   = 'A',
        fmt_bin     = 'b',
        fmt_BIN     = 'B',
        fmt_dec     = 'd',
        fmt_DEC     = 'D',
        fmt_hex     = 'x',
        fmt_HEX     = 'X',
        fmt_oct     = 'o',
        fmt_OCT     = 'O',
        fmt_fixed   = 'f',
        fmt_FIXED   = 'F',
        fmt_scient  = 's',
        fmt_SCIENT  = 'S',
        fmt_UPPER   = 'U',
        fmt_lower   = 'l',
        fmt_Capital = 'C',
        fmt_minus   = '-',
        fmt_sharp   = '#',
        fmt_center  = '@',
        fmt_truncR  = '/',
        fmt_truncL  = '\\',
    };
};
//----------------------------------------------------------------------------
template <>
struct TFormatTraits_<wchar_t> {
    enum EFlags : wchar_t {
        null        = L'\0',
        lbrace      = L'{',
        rbrace      = L'}',
        colon       = L':',
        multiply    = L'*',
        zero        = L'0',
        fmt_alpha   = L'a',
        fmt_ALPHA   = L'A',
        fmt_bin     = L'b',
        fmt_BIN     = L'B',
        fmt_dec     = L'd',
        fmt_DEC     = L'D',
        fmt_hex     = L'x',
        fmt_HEX     = L'X',
        fmt_oct     = L'o',
        fmt_OCT     = L'O',
        fmt_fixed   = L'f',
        fmt_FIXED   = L'F',
        fmt_scient  = L's',
        fmt_SCIENT  = L'S',
        fmt_UPPER   = L'U',
        fmt_lower   = L'l',
        fmt_Capital = L'C',
        fmt_point   = L'.',
        fmt_minus   = L'-',
        fmt_sharp   = L'#',
        fmt_center  = L'@',
        fmt_truncR  = L'/',
        fmt_truncL  = L'\\',
    };
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicFormatProps_ {
    size_t Repeat{};
    _Char FillChar{};
    FTextFormat Format{};

    inline friend TBasicTextWriter<_Char>& operator >>(TBasicTextWriter<_Char>& in, TBasicFormatProps_& props) {
        props.Repeat = 1;
        props.FillChar = in.FillChar();
        props.Format = in.Format();
        return in;
    }

    inline friend TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& out, const TBasicFormatProps_& props) {
        out.SetFillChar(props.FillChar);
        out.SetFormat(props.Format);
        return out;
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(
    TBasicTextWriter<_Char>& oss,
    const details::TBasicFormatFunctor_<_Char>& functor ) {
    functor.Helper(oss, functor.Arg);
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE static bool FormatParser_(TBasicStringView<_Char>& format, TBasicStringView<_Char> *outp, size_t *index, TBasicFormatProps_<_Char>& props) {
    typedef typename TFormatTraits_<_Char>::EFlags format_traits;

    if (format.empty())
        return false;

    *outp = format.CutBefore(format.begin());
    *index = size_t(-1);

    props.Repeat = 1;

    do {
        if (format_traits::lbrace != format.front() || not IsDigit(format[1]) ) {
            *outp = outp->GrowBack();
            format = format.ShiftFront();
        }
        else {
            const TBasicStringView<_Char> formatBeforeParse = format;

            Assert(format_traits::lbrace == format.front());
            format = format.ShiftFront(); // eat '{'

            intptr_t parsedIndex = 0;
            {
                const TBasicStringView<_Char> digits = EatDigits(format);
                Assert(digits.data() + digits.size() == format.data());

                if (not Atoi(&parsedIndex, digits, 10))
                    AssertNotReached();
            }

            if (format_traits::colon == format.front()){
                format = format.ShiftFront();

                do {
                    Assert(format_traits::null != format.front());

                    switch (format.front()) {
                    case format_traits::fmt_ALPHA:
                        props.Format.SetCase(FTextFormat::Uppercase);
                    case format_traits::fmt_alpha:
                        props.Format.SetMisc(FTextFormat::BoolAlpha);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_bin:
                    case format_traits::fmt_BIN:
                        props.Format.SetBase(FTextFormat::Binary);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_DEC:
                    case format_traits::fmt_dec:
                        props.Format.SetBase(FTextFormat::Decimal);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_HEX:
                        props.Format.SetCase(FTextFormat::Uppercase);
                    case format_traits::fmt_hex:
                        props.Format.SetBase(FTextFormat::Hexadecimal);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_OCT:
                    case format_traits::fmt_oct:
                        props.Format.SetBase(FTextFormat::Octal);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_SCIENT:
                        props.Format.SetCase(FTextFormat::Uppercase);
                    case format_traits::fmt_scient:
                        props.Format.SetFloat(FTextFormat::ScientificFloat);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_UPPER:
                        props.Format.SetCase(FTextFormat::Uppercase);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_lower:
                        props.Format.SetCase(FTextFormat::Lowercase);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_Capital:
                        props.Format.SetCase(FTextFormat::Capitalize);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_sharp:
                        props.FillChar = format_traits::zero;
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_truncR:
                        props.Format.SetMisc(FTextFormat::TruncateR, true);
                        format = format.ShiftFront();
                        continue;
                    case format_traits::fmt_truncL:
                        props.Format.SetMisc(FTextFormat::TruncateL, true);
                        format = format.ShiftFront();
                        continue;
                    }

                    bool expect_fixed = false;
                    bool expect_repeat = false;
                    bool expect_width = false;

                    FTextFormat::EPadding padding = FTextFormat::Padding_Left;

                    if (format_traits::fmt_center == format.front()) {
                        format = format.ShiftFront();
                        padding = FTextFormat::Padding_Center;
                        expect_width = true;
                    }
                    else if (format_traits::fmt_minus == format.front()) {
                        format = format.ShiftFront();
                        padding = FTextFormat::Padding_Right;
                        expect_width = true;
                    }
                    else if (format_traits::multiply == format.front()) {
                        format = format.ShiftFront();
                        expect_repeat = true;
                    }
                    else if (format_traits::fmt_FIXED == format.front() ||
                            format_traits::fmt_fixed == format.front()) {
                        props.Format.SetFloat(FTextFormat::FixedFloat);
                        props.Format.SetCase(format_traits::fmt_fixed == format.front() ? FTextFormat::Lowercase : FTextFormat::Uppercase);
                        format = format.ShiftFront();
                        expect_fixed = true;
                    }
                    else if (IsDigit(format.front())) {
                        expect_width = true;
                    }

                    if (not IsDigit(format.front())) {
                        AssertRelease(not (expect_fixed||expect_repeat||expect_width)); // invalid format : expected a number !
                        break;
                    }

                    intptr_t parsedScalar = 0;
                    {
                        const TBasicStringView<_Char> digits = EatDigits(format);
                        Assert(digits.data() + digits.size() == format.data());

                        if (not Atoi(&parsedScalar, digits, 10))
                            AssertNotReached();
                    }

                    if (expect_fixed) {
                        AssertRelease(padding != FTextFormat::Padding_Right); // invalid format : negative precision is not supported
                        props.Format.SetPrecision(checked_cast<size_t>(parsedScalar));
                    }
                    else if (expect_repeat) {
                        props.Repeat = checked_cast<size_t>(parsedScalar);
                    }
                    else if (expect_width) {
                        props.Format.SetWidth(checked_cast<size_t>(parsedScalar));
                        props.Format.SetPadding(padding);
                    }
                    else {
                        AssertNotReached();
                    }
                }
                while (format.size());
            }

            if (format_traits::rbrace == format.front())
            {
                format = format.ShiftFront();
                *index = checked_cast<size_t>(parsedIndex);

                return true;
            }
            else
            {
                // bad string format, missing '}'
                // but handled :

                *outp = formatBeforeParse;
                return true;
            }
        }
    }
    while (format.size());

    Assert(not outp->empty());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE static void FormatArgsImpl_(
    TBasicTextWriter<_Char>& oss,
    const TBasicStringView<_Char>& format,
    const TMemoryView<const details::TBasicFormatFunctor_<_Char>>& args ) {
    Assert(format.data());

#if USE_PPE_ASSERT
    const EValidateFormat formatValidation = ValidateFormatString(format.data(), format.size(), args.size());
    AssertMessage_NoAssume(L"invalid format string", EValidateFormat::InvalidFormatString != formatValidation);
    AssertMessage_NoAssume(L"out-of-bounds argument index", EValidateFormat::ArgumentOutOfBounds != formatValidation);
    AssertMessage_NoAssume(L"invalid format manipulator", EValidateFormat::InvalidFormatManip != formatValidation);
    AssertMessage_NoAssume(L"unused arguments", EValidateFormat::UnusedArguments != formatValidation);
    AssertMessage_NoAssume(L"too many arguments (>10)", EValidateFormat::TooManyArguments != formatValidation);
#endif

    TBasicFormatProps_<_Char> props;
    TBasicStringView<_Char> formatIt = format;
    TBasicStringView<_Char> outp;

    oss >> props;

    const TBasicFormatProps_<_Char> original = props;

    size_t index = size_t(-1);
    while (FormatParser_(formatIt, &outp, &index, props)) {
        if (outp.size())
            oss.Put(outp);

        if (INDEX_NONE != index) {
            Assert(props.Repeat);
            AssertRelease(index < args.size()); // detects invalid user input

            for (size_t n = 0; n < props.Repeat; ++n)
                oss << props << args[index];
        }

        props = original;
    }

    oss << original; // restores original state
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE static void FormatRecordImpl_(
    TBasicTextWriter<_Char>& oss,
    const TMemoryView<const details::TBasicFormatFunctor_<_Char>>& record ) {
    Assert(not record.empty());

    oss << record.front();

    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));

    oss << Fmt::LParenthese << Fmt::Space;
    for (const details::TBasicFormatFunctor_<_Char>& fmt : record.ShiftFront()) {
        oss << sep;
        fmt.Helper(oss, fmt.Arg);
    }
    oss << Fmt::Space << Fmt::RParenthese;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
void FormatArgs_(FTextWriter& oss, const FStringView& format, const TMemoryView<const FFormatFunctor_>& args) {
    FormatArgsImpl_(oss, format, args);
}
//----------------------------------------------------------------------------
void FormatArgs_(FWTextWriter& oss, const FWStringView& format, const TMemoryView<const FWFormatFunctor_>& args) {
    FormatArgsImpl_(oss, format, args);
}
//----------------------------------------------------------------------------
void FormatRecord_(FTextWriter& oss, const TMemoryView<const FFormatFunctor_>& record) {
    FormatRecordImpl_(oss, record);
}
//----------------------------------------------------------------------------
void FormatRecord_(FWTextWriter& oss, const TMemoryView<const FWFormatFunctor_>& record) {
    FormatRecordImpl_(oss, record);
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Simple helpers used when Format() is called without arguments:
//----------------------------------------------------------------------------
FTextWriter& Format(FTextWriter& oss, const FStringView& str) {
    oss.Write(str);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& Format(FWTextWriter& oss, const FWStringView& wstr) {
    oss.Write(wstr);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
