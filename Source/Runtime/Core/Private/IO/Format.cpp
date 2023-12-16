// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
struct TFormatTraits_ {
    enum EFlags : _Char {
        null =              STRING_LITERAL(_Char, '\0'),
        lbrace =            STRING_LITERAL(_Char, '{'),
        rbrace =            STRING_LITERAL(_Char, '}'),
        colon =             STRING_LITERAL(_Char, ':'),
        multiply =          STRING_LITERAL(_Char, '*'),
        zero =              STRING_LITERAL(_Char, '0'),
        fmt_alpha =         STRING_LITERAL(_Char, 'a'),
        fmt_ALPHA =         STRING_LITERAL(_Char, 'A'),
        fmt_bin =           STRING_LITERAL(_Char, 'b'),
        fmt_BIN =           STRING_LITERAL(_Char, 'B'),
        fmt_dec =           STRING_LITERAL(_Char, 'd'),
        fmt_DEC =           STRING_LITERAL(_Char, 'D'),
        fmt_hex =           STRING_LITERAL(_Char, 'x'),
        fmt_HEX =           STRING_LITERAL(_Char, 'X'),
        fmt_oct =           STRING_LITERAL(_Char, 'o'),
        fmt_OCT =           STRING_LITERAL(_Char, 'O'),
        fmt_fixed =         STRING_LITERAL(_Char, 'f'),
        fmt_FIXED =         STRING_LITERAL(_Char, 'F'),
        fmt_scient =        STRING_LITERAL(_Char, 's'),
        fmt_SCIENT =        STRING_LITERAL(_Char, 'S'),
        fmt_UPPER =         STRING_LITERAL(_Char, 'U'),
        fmt_lower =         STRING_LITERAL(_Char, 'l'),
        fmt_Capital =       STRING_LITERAL(_Char, 'C'),
        fmt_minus =         STRING_LITERAL(_Char, '-'),
        fmt_compact =       STRING_LITERAL(_Char, '_'),
        fmt_NONCOMPACT =    STRING_LITERAL(_Char, '^'),
        fmt_zeropad =       STRING_LITERAL(_Char, '#'),
        fmt_center =        STRING_LITERAL(_Char, '@'),
        fmt_truncR =        STRING_LITERAL(_Char, '<'),
        fmt_truncL =        STRING_LITERAL(_Char, '>'),
        fmt_escape =        STRING_LITERAL(_Char, '\\'),
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
    FTextFormat Format{};
    _Char FillChar{};

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
enum class EFormatParserResult_ : int {
    InvalidFormat = -1,
    Complete = 0,
    Continue,
    PrintArg,
};
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE static EFormatParserResult_ FormatParser_(TBasicStringView<_Char>& format, TBasicStringView<_Char> *outp, size_t *index, TBasicFormatProps_<_Char>& props) {
    typedef typename TFormatTraits_<_Char>::EFlags format_traits;

    if (format.empty())
        return EFormatParserResult_::Complete;

    *outp = format.CutBefore(format.begin());

    props.Repeat = 1;

    do {
        if (format_traits::lbrace != format.front()) {
            *outp = outp->GrowBack();
            format = format.ShiftFront();
        }
        else {
            const TBasicStringView<_Char> formatBeforeParse = format;

            Assert(format_traits::lbrace == format.front());
            format = format.ShiftFront(); // eat '{'

            intptr_t parsedIndex = 0;
            if (const TBasicStringView<_Char> digits = EatDigits(format); not digits.empty()) {
                Assert_NoAssume(digits.data() + digits.size() == format.data());

                if (not Atoi(&parsedIndex, digits, 10))
                    AssertNotReached();
            }
            else { // implicit position increments the index for every occurrence of '{}'
                parsedIndex = checked_cast<intptr_t>(*index + 1);
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

                    case format_traits::fmt_compact:
                    case format_traits::fmt_NONCOMPACT:
                        props.Format.SetMisc(FTextFormat::Compact, format.front() == format_traits::fmt_compact);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_zeropad:
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
                    case format_traits::fmt_escape:
                        props.Format.SetMisc(FTextFormat::Escape, true);
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
                    else {
                        if (expect_fixed || expect_repeat || expect_width)
                            return EFormatParserResult_::InvalidFormat; // invalid format : expected a number !
                        break;
                    }

                    intptr_t parsedScalar = 0;
                    {
                        const TBasicStringView<_Char> digits = EatDigits(format);
                        Assert_NoAssume(digits.data() + digits.size() == format.data());

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
                        Assert_NoAssume(false);
                        return EFormatParserResult_::InvalidFormat;
                    }
                }
                while (format.size());
            }

            if (format_traits::rbrace == format.front())
            {
                format = format.ShiftFront();
                *index = checked_cast<size_t>(parsedIndex);

                return EFormatParserResult_::PrintArg;
            }
            else
            {
                // bad string format, missing '}'
                // but handled :
                AssertMessage_NoAssume("invalid user format string", false);
#if 0
                return EFormatParserResult_::InvalidFormat; // error
#else
                *outp = formatBeforeParse;
                break; // handled: print raw format string instead of expanding it
#endif
            }
        }
    }
    while (format.size());

    Assert(not outp->empty());
    return EFormatParserResult_::Continue;
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE static void FormatArgsImpl_(
    TBasicTextWriter<_Char>& oss,
    TBasicStringView<_Char> format,
    TMemoryView<const details::TBasicFormatFunctor_<_Char>> args ) {
    Assert(format.data());

#if USE_PPE_ASSERT
    {
        const EValidateFormat formatValidation = ValidateFormatString(format.data(), format.size(), args.size());
        AssertMessage_NoAssume("invalid format string", EValidateFormat::InvalidFormatString != formatValidation);
        AssertMessage_NoAssume("out-of-bounds argument index", EValidateFormat::ArgumentOutOfBounds != formatValidation);
        AssertMessage_NoAssume("invalid format manipulator", EValidateFormat::InvalidFormatManip != formatValidation);
        AssertMessage_NoAssume("unused arguments", EValidateFormat::UnusedArguments != formatValidation);
        AssertMessage_NoAssume("too many arguments (>10)", EValidateFormat::TooManyArguments != formatValidation);
    }
#endif

    TBasicFormatProps_<_Char> props;
    TBasicStringView<_Char> outp;

    oss >> props;

    const TBasicFormatProps_<_Char> original = props;

    size_t index = size_t(-1);
    for (;;) {
        const EFormatParserResult_ result = FormatParser_(format, &outp, &index, props);
        AssertRelease(result != EFormatParserResult_::InvalidFormat);
        if (result == EFormatParserResult_::Complete ||
            result == EFormatParserResult_::InvalidFormat)
            break;

        if (outp.size())
            oss.Put(outp);

        if (result == EFormatParserResult_::PrintArg) {
            Assert(props.Repeat);
            AssertReleaseMessage("format argument index is out-of-bounds", index < args.size()); // detects invalid user input

            for (size_t n = 0; n < props.Repeat; ++n)
                oss << props << args[index];
        }
        else {
            Assert_NoAssume(result == EFormatParserResult_::Continue);
        }

        props = original;
    }

    oss << original; // restores original state
}
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD NO_INLINE static bool UnsafeFormatArgsImpl_(
    TBasicTextWriter<_Char>& oss,
    TBasicConstChar<_Char> unsafeFormat,
    TMemoryView<const details::TBasicFormatFunctor_<_Char>> args ) {
    STATIC_CONST_INTEGRAL(size_t, MaxLengthForUnsafeFormat, 256);

    size_t safeLength = 0;
    if (not unsafeFormat.unsafe_length(MaxLengthForUnsafeFormat, &safeLength))
        return false;

    if (ValidateFormatString(unsafeFormat.c_str(), safeLength, args.size()) != EValidateFormat::Valid)
        return false;

    FormatArgsImpl_(oss, TBasicStringView<_Char>(unsafeFormat.c_str(), safeLength), std::move(args));
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE static void FormatRecordImpl_(
    TBasicTextWriter<_Char>& oss,
    TMemoryView<const details::TBasicFormatFunctor_<_Char>> record ) {
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
bool UnsafeFormatArgs_(FTextWriter& oss, FConstChar unsafeFormat, TMemoryView<const FFormatFunctor_> args) {
    return UnsafeFormatArgsImpl_(oss, unsafeFormat, std::move(args));
}
//----------------------------------------------------------------------------
bool UnsafeFormatArgs_(FWTextWriter& oss, FConstWChar unsafeFormat, TMemoryView<const FWFormatFunctor_> args) {
    return UnsafeFormatArgsImpl_(oss, unsafeFormat, std::move(args));
}
//----------------------------------------------------------------------------
void FormatArgs_(FTextWriter& oss, FStringLiteral format, TMemoryView<const FFormatFunctor_> args) {
    FormatArgsImpl_(oss, format.MakeView(), std::move(args));
}
//----------------------------------------------------------------------------
void FormatArgs_(FWTextWriter& oss, FWStringLiteral format, TMemoryView<const FWFormatFunctor_> args) {
    FormatArgsImpl_(oss, format.MakeView(), std::move(args));
}
//----------------------------------------------------------------------------
void FormatRecord_(FTextWriter& oss, TMemoryView<const FFormatFunctor_> record) {
    FormatRecordImpl_(oss, std::move(record));
}
//----------------------------------------------------------------------------
void FormatRecord_(FWTextWriter& oss, TMemoryView<const FWFormatFunctor_> record) {
    FormatRecordImpl_(oss, std::move(record));
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Simple helpers used when Format() is called without arguments:
//----------------------------------------------------------------------------
FTextWriter& Format(FTextWriter& oss, FStringLiteral str) {
    oss.Write(str);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& Format(FWTextWriter& oss, FWStringLiteral wstr) {
    oss.Write(wstr);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
