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
static bool FormatParser_(const _Char **pformat, _Char *outp, size_t *index, FormatProperties_& props) {
    Assert(pformat);
    Assert(*pformat);

    typedef typename FormatTraits_<_Char>::Flags format_traits;

    if (format_traits::null == **pformat)
        return false;

    bool fixed = false;

    if (format_traits::lbrace == **pformat && IsDigit(*(*pformat + 1)) ) {
        const _Char* d = *pformat + 1;
        for (; IsDigit(*(d + 1)); ++d);

        size_t i = 0;
        for (size_t n = d - *pformat, base = 1; n > 0; --n, base *= 10)
            i += ((*pformat)[n] - format_traits::zero) * base;

        ++d;

        if (format_traits::colon == *d)
        {
            ++d;
            while (format_traits::null != *d &&
                format_traits::rbrace != *d) {
                switch (*d)
                {
                case format_traits::fmt_ALPHA:
                    props.Flags |= std::ios_base::uppercase;
                case format_traits::fmt_alpha:
                    props.Flags |= std::ios_base::boolalpha;
                    ++d;
                    continue;

                case format_traits::fmt_DEC:
                case format_traits::fmt_dec:
                    props.Flags = (props.Flags & ~std::ios_base::basefield) | (std::ios_base::dec & std::ios_base::basefield);
                    ++d;
                    continue;

                case format_traits::fmt_HEX:
                    props.Flags |= std::ios_base::uppercase;
                case format_traits::fmt_hex:
                    props.Flags = (props.Flags & ~std::ios_base::basefield) | (std::ios_base::hex & std::ios_base::basefield);
                    ++d;
                    continue;

                case format_traits::fmt_OCT:
                case format_traits::fmt_oct:
                    props.Flags = (props.Flags & ~std::ios_base::basefield) | (std::ios_base::oct & std::ios_base::basefield);
                    ++d;
                    continue;

                case format_traits::fmt_FIXED:
                case format_traits::fmt_fixed:
                    props.Flags = (props.Flags & ~std::ios_base::floatfield) | (std::ios_base::fixed & std::ios_base::floatfield);
                    ++d;
                    fixed = true;
                    continue;

                case format_traits::fmt_SCIENT:
                case format_traits::fmt_scient:
                    props.Flags = (props.Flags & ~std::ios_base::floatfield) | (std::ios_base::scientific & std::ios_base::floatfield);
                    ++d;
                    continue;

                case format_traits::fmt_UPPER:
                case format_traits::fmt_upper:
                    props.Flags |= std::ios_base::uppercase;
                    ++d;
                    continue;

                case format_traits::fmt_BASE:
                case format_traits::fmt_base:
                    props.Flags |= std::ios_base::showbase;
                    ++d;
                    continue;

                case format_traits::fmt_point:
                    props.Flags |= std::ios_base::showpoint;
                    ++d;
                    continue;

                case format_traits::fmt_sharp:
                    props.Fill = format_traits::zero;
                    ++d;
                    continue;
                }

                int sign = 1;
                if (format_traits::fmt_minus == *d) {
                    ++d;
                    sign = -1;
                }

                if (false == IsDigit(*d))
                    break;

                const _Char* b = d;
                for (; IsDigit(*d); ++d);

                std::streamsize s = 0;
                for (size_t n = d - b, base = 1; n > 0; --n, base *= 10)
                    s += (b[n - 1] - format_traits::zero) * base;

                if (fixed)
                    props.Precision = s;
                else
                    props.Width = s;

                props.Flags |= (sign < 0) ? std::ios_base::left : std::ios_base::right;
            }
        }

        props.Repeat = 1;
        if (format_traits::multiply == *d)
        {
            ++d;

            if (IsDigit(*d)) {
                const _Char* b = d;
                for (; IsDigit(*d); ++d);

                std::streamsize s = 0;
                for (size_t n = d - b, base = 1; n > 0; --n, base *= 10)
                    s += (b[n - 1] - format_traits::zero) * base;

                Assert(s);
                props.Repeat = checked_cast<size_t>(s);
            }
        }

        if (format_traits::rbrace == *d)
        {
            *outp = _Char(0);
            *index = i;
            *pformat = d + 1;

            return true;
        }
        else
        {
            AssertNotImplemented(); // bad string format
            // but the string will still be printed :
        }
    }

    *outp = **pformat;
    *pformat = *pformat + 1;
    Assert(*outp != _Char());

    return true;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
static void FormatArgs_(
    std::basic_ostream<_Char, _Traits>& oss,
    const _Char *format,
    const details::_FormatFunctor<_Char, _Traits> *args, size_t count ) {
    Assert(format);

    FormatProperties_ original;
    original.From(oss); // backups original state

    FormatProperties_ props = original;

    _Char outp(0);
    size_t index(0);
    while (FormatParser_(&format, &outp, &index, props)) {
        if (outp != _Char(0)) {
            oss.put(outp);
        }
        else {
            AssertRelease(index < count); // detects invalid user input

            for (size_t n = 0; n < props.Repeat; ++n) {
                props.To(oss);
                oss << args[index];
            }

            props = original.To(oss); // restores original state
        }
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
void _FormatArgs(std::basic_ostream<char>& oss, const char *format, const _FormatFunctor<char> *args, size_t count) {
    FormatArgs_(oss, format, args, count);
}
//----------------------------------------------------------------------------
void _FormatArgs(std::basic_ostream<wchar_t>& oss, const wchar_t *format, const _FormatFunctor<wchar_t> *args, size_t count) {
    FormatArgs_(oss, format, args, count);
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
