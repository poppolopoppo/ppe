#include "stdafx.h"

#include "Format.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
struct FormatTraits_ {};
//----------------------------------------------------------------------------
template <>
struct FormatTraits_<char> {
    enum Flags : char {
        null        = '\0',
        lbrace      = '{',
        rbrace      = '}',
        colon       = ':',
        multiply    = '*',
        zero        = '0',
        fmt_alpha   = 'a',
        fmt_ALPHA   = 'A',
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
        fmt_upper   = 'u',
        fmt_UPPER   = 'U',
        fmt_base    = 'b',
        fmt_BASE    = 'B',
        fmt_size    = 'z',
        fmt_SIZE    = 'Z',
        fmt_point   = '.',
        fmt_minus   = '-',
        fmt_sharp   = '#',
    };
};
//----------------------------------------------------------------------------
template <>
struct FormatTraits_<wchar_t> {
    enum Flags : wchar_t {
        null        = L'\0',
        lbrace      = L'{',
        rbrace      = L'}',
        colon       = L':',
        multiply    = L'*',
        zero        = L'0',
        fmt_alpha   = L'a',
        fmt_ALPHA   = L'A',
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
        fmt_upper   = L'u',
        fmt_UPPER   = L'U',
        fmt_base    = L'b',
        fmt_BASE    = L'B',
        fmt_size    = L'z',
        fmt_SIZE    = L'Z',
        fmt_point   = L'.',
        fmt_minus   = L'-',
        fmt_sharp   = L'#',
    };
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FormatProperties_ {
    int Fill;
    std::streamsize Width;
    std::streamsize Precision;
    std::ios_base::fmtflags Flags;
    size_t Repeat;

    template <typename _Char, typename _Traits = std::char_traits<_Char> >
    void From(const std::basic_ostream<_Char, _Traits>& io);

    template <typename _Char, typename _Traits = std::char_traits<_Char> >
    FormatProperties_ To(std::basic_ostream<_Char, _Traits>& io) const;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
void FormatProperties_::From(const std::basic_ostream<_Char, _Traits>& io) {
    Fill = checked_cast<int>(io.fill());
    Width = io.width();
    Precision = io.precision();
    Flags = io.flags();
    Repeat = 1;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
FormatProperties_ FormatProperties_::To(std::basic_ostream<_Char, _Traits>& io) const {
    return FormatProperties_ {
        io.fill(checked_cast<_Char>(Fill)),
        io.width(Width),
        io.precision(Precision),
        io.flags(Flags),
        Repeat
    };
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const details::_FormatFunctor<_Char, _Traits>& functor ) {
    functor._helper(oss, functor._pArg);
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool FormatParser_(BasicStringView<_Char>& format, BasicStringView<_Char> *outp, size_t *index, FormatProperties_& props) {
    typedef typename FormatTraits_<_Char>::Flags format_traits;

    if (format.empty())
        return false;

    *outp = format.CutBefore(format.begin());
    *index = size_t(-1);

    do {
        bool fixed = false;

        if (format_traits::lbrace != format.front() || not IsDigit(format[1]) ) {
            *outp = outp->GrowBack();
            format = format.ShiftFront();
        }
        else {
            const BasicStringView<_Char> formatBeforeParse = format;

            Assert(format_traits::lbrace == format.front());
            format = format.ShiftFront(); // eat '{'

            intptr_t parsedIndex = 0;
            {
                const BasicStringView<_Char> digits = EatDigits(format);
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
                        props.Flags |= std::ios_base::uppercase;
                    case format_traits::fmt_alpha:
                        props.Flags |= std::ios_base::boolalpha;
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_DEC:
                    case format_traits::fmt_dec:
                        props.Flags = (props.Flags & ~std::ios_base::basefield) | (std::ios_base::dec & std::ios_base::basefield);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_HEX:
                        props.Flags |= std::ios_base::uppercase;
                    case format_traits::fmt_hex:
                        props.Flags = (props.Flags & ~std::ios_base::basefield) | (std::ios_base::hex & std::ios_base::basefield);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_OCT:
                    case format_traits::fmt_oct:
                        props.Flags = (props.Flags & ~std::ios_base::basefield) | (std::ios_base::oct & std::ios_base::basefield);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_FIXED:
                    case format_traits::fmt_fixed:
                        props.Flags = (props.Flags & ~std::ios_base::floatfield) | (std::ios_base::fixed & std::ios_base::floatfield);
                        format = format.ShiftFront();
                        fixed = true;
                        continue;

                    case format_traits::fmt_SCIENT:
                        props.Flags |= std::ios_base::uppercase;
                    case format_traits::fmt_scient:
                        props.Flags = (props.Flags & ~std::ios_base::floatfield) | (std::ios_base::scientific & std::ios_base::floatfield);
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_UPPER:
                    case format_traits::fmt_upper:
                        props.Flags |= std::ios_base::uppercase;
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_BASE:
                    case format_traits::fmt_base:
                        props.Flags |= std::ios_base::showbase;
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_point:
                        props.Flags |= std::ios_base::showpoint;
                        format = format.ShiftFront();
                        continue;

                    case format_traits::fmt_sharp:
                        props.Fill = format_traits::zero;
                        format = format.ShiftFront();
                        continue;
                    }

                    int sign = 1;
                    if (format_traits::fmt_minus == format.front()) {
                        format = format.ShiftFront();
                        sign = -1;
                    }

                    if (not IsDigit(format.front())) {
                        AssertRelease(sign > 0); // invalid format : minus without digits
                        break;
                    }

                    intptr_t parsedScalar = 0;
                    {
                        const BasicStringView<_Char> digits = EatDigits(format);
                        Assert(digits.data() + digits.size() == format.data());

                        if (not Atoi(&parsedScalar, digits, 10))
                            AssertNotReached();
                    }

                    if (fixed) {
                        AssertRelease(sign > 0); // invalid format : negative precision is not supported
                        props.Precision = checked_cast<std::streamsize>(parsedScalar);
                    }
                    else {
                        props.Width = checked_cast<std::streamsize>(parsedScalar);

                        if (sign < 0) {
                            props.Flags &= ~std::ios_base::right;
                            props.Flags |= std::ios_base::left;
                        }
                        else {
                            props.Flags &= ~std::ios_base::left;
                            props.Flags |= std::ios_base::right;
                        }
                    }
                }
                while (format.size() && format_traits::rbrace != format.front());
            }

            props.Repeat = 1;
            if (format_traits::multiply == format.front())
            {
                format = format.ShiftFront();
                AssertRelease(IsDigit(format.front())); // invalid format : * must be followed by a digit

                intptr_t parsedRepeat = 0;
                {
                    const BasicStringView<_Char> digits = EatDigits(format);
                    Assert(digits.data() + digits.size() == format.data());

                    if (not Atoi(&parsedRepeat, digits, 10))
                        AssertNotReached();
                }

                Assert(parsedRepeat > 0);
                props.Repeat = checked_cast<size_t>(parsedRepeat);
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
template <typename _Char, typename _Traits>
static void FormatArgs_(
    std::basic_ostream<_Char, _Traits>& oss,
    const BasicStringView<_Char>& format,
    const MemoryView<const details::_FormatFunctor<_Char, _Traits>>& args ) {
    Assert(format.Pointer());
    Assert(!oss.bad());

    FormatProperties_ original;
    original.From(oss); // backups original state

    FormatProperties_ props = original;
    BasicStringView<_Char> formatIt = format;
    BasicStringView<_Char> outp;

    size_t index = size_t(-1);
    while (FormatParser_(formatIt, &outp, &index, props)) {
        if (outp.size())
            oss.write(outp.Pointer(), outp.size());

        if (size_t(-1) != index) {
            AssertRelease(index < args.size()); // detects invalid user input

            for (size_t n = 0; n < props.Repeat; ++n) {
                props.To(oss);
                oss << args[index];
            }
        }

        Assert(!oss.bad());

        original.To(oss); // restores original state
        props = original;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
void _FormatArgs(std::basic_ostream<char>& oss, const StringView& format, const MemoryView<const _FormatFunctor<char>>& args) {
    FormatArgs_(oss, format, args);
}
//----------------------------------------------------------------------------
void _FormatArgs(std::basic_ostream<wchar_t>& oss, const WStringView& format, const MemoryView<const _FormatFunctor<wchar_t>>& args) {
    FormatArgs_(oss, format, args);
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
