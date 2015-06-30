#pragma once

#include "Core/IO/Format.h"

#include <iomanip>
#include <locale> // std::isdigit
#include <sstream>
#include <strstream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
struct FormatTraits {};
template <>
struct FormatTraits<char> {
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
template <>
struct FormatTraits<wchar_t> {
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
struct FormatProperties {
    int Fill;
    std::streamsize Width;
    std::streamsize Precision;
    std::ios_base::fmtflags Flags;
    size_t Repeat;

    template <typename _Char, typename _Traits = std::char_traits<_Char> >
    void From(const std::basic_ostream<_Char, _Traits>& io) {
        Fill = checked_cast<int>(io.fill());
        Width = io.width();
        Precision = io.precision();
        Flags = io.flags();
        Repeat = 1;
    }

    template <typename _Char, typename _Traits = std::char_traits<_Char> >
    FormatProperties To(std::basic_ostream<_Char, _Traits>& io) const {
        return FormatProperties {
            io.fill(checked_cast<_Char>(Fill)),
            io.width(Width),
            io.precision(Precision),
            io.flags(Flags),
            Repeat
        };
    }
};
//----------------------------------------------------------------------------
// helper to parse arguments, reduce codegen size in Format()
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
class FormatParser : public FormatTraits<_Char> {
public:
    typedef FormatTraits<_Char> format_traits;
    using format_traits::Flags;

    FormatParser(const _Char* format);
    ~FormatParser();

    enum Result {
        parse_eof,
        parse_outp,
        parse_param,
    };

    Result Parse(_Char& outp, size_t& index, FormatProperties& props);

private:
    const _Char* _p;
};
//----------------------------------------------------------------------------
extern template class FormatParser< char, std::char_traits<char> >;
extern template class FormatParser< wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
FormatParser<_Char, _Traits>::FormatParser(const _Char* format)
:   _p(format) {
    Assert(format);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
FormatParser<_Char, _Traits>::~FormatParser() {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
auto FormatParser<_Char, _Traits>::Parse(_Char& outp, size_t& index, FormatProperties& props) -> Result {
    if (format_traits::null == *_p)
        return parse_eof;

    bool fixed = false;

    const std::locale& locale = std::locale::classic();
    if (format_traits::lbrace == *_p && std::isdigit(*(_p + 1), locale)) {
        const _Char* d = _p + 1;
        for (; std::isdigit(*(d + 1), locale); ++d);

        size_t i = 0;
        for (size_t n = d - _p, base = 1; n > 0; --n, base *= 10)
            i += (_p[n] - format_traits::zero) * base;

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

                if (!std::isdigit(*d, locale))
                    break;

                const _Char* b = d;
                for (; std::isdigit(*d, locale); ++d);

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

            if (std::isdigit(*d, locale)) {
                const _Char* b = d;
                for (; std::isdigit(*d, locale); ++d);

                std::streamsize s = 0;
                for (size_t n = d - b, base = 1; n > 0; --n, base *= 10)
                    s += (b[n - 1] - format_traits::zero) * base;

                Assert(s);
                props.Repeat = checked_cast<size_t>(s);
            }
        }

        if (format_traits::rbrace == *d)
        {
            index = i;
            _p = d + 1;
            return parse_param;
        }
        else
        {
            Assert(false); // bad string format
            // but the string will still be printed :
        }
    }

    outp = *_p++;
    return parse_outp;
}
//----------------------------------------------------------------------------
// not superb but does the trick
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
FORCE_INLINE static void AppendArg_(std::basic_ostream<_Char, _Traits>&/* oss */, size_t/* index */) {
    Assert(false); // index it out of bounds !
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
FORCE_INLINE static void AppendArg_(std::basic_ostream<_Char, _Traits>& oss, size_t index, _Arg0&& arg0, _Args&&... args) {
    if (index)
        AppendArg_(oss, index - 1, std::forward<_Args>(args)...);
    else
        oss << arg0;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
void Format(std::basic_ostream<_Char, _Traits>& oss, const _Char* format, _Arg0&& arg0, _Args&&... args) {
    Assert(format);
    typedef FormatParser<_Char, _Traits> format_parser;

    FormatProperties props, original;
    original.From(oss);
    props = original;

    _Char outp;
    size_t index;
    format_parser::Result parseResult;

    format_parser parser(format);
    while (format_parser::parse_eof != (parseResult = parser.Parse(outp, index, props)) ) {
        if (format_parser::parse_outp == parseResult)
        {
            oss << outp;
        }
        else
        {
            Assert(format_parser::parse_param == parseResult);

            for (size_t n = 0; n < props.Repeat; ++n) {
                props.To(oss);
                AppendArg_(oss, index, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
            }

            original.To(oss); // restores stream original state
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t Format(_Char* result, size_t capacity, const _Char* format, _Arg0&& arg0, _Args&&... args) {
    Assert(result);
    Assert(capacity);

    typedef std::char_traits<_Char> char_traits;
    BasicOCStrStream<_Char, char_traits> oss{ result, checked_cast<std::streamsize>(capacity) };
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    return checked_cast<size_t>(oss.size());
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
void Format(BasicString<_Char, _Traits>& result, const _Char* format, _Arg0&& arg0, _Args&&... args) {
    result.resize(256);
    BasicOCStrStream<_Char, _Traits> oss{ &result[0], checked_cast<std::streamsize>(result.size()) };
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    result.resize(checked_cast<size_t>(oss.size()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg>
String ToString(_Arg&& arg) {
    String result;
    result.resize(256);
    OCStrStream oss{ &result[0], checked_cast<std::streamsize>(result.size()) };
    oss << arg;
    result.resize(checked_cast<size_t>(oss.size()));
    return result;
}
//----------------------------------------------------------------------------
template <typename _Arg>
WString ToWString(_Arg&& arg) {
    WString result;
    result.resize(256);
    WOCStrStream oss{ &result[0], checked_cast<std::streamsize>(result.size()) };
    oss << arg;
    result.resize(checked_cast<size_t>(oss.size()));
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline String ToString(const String& str) {
    return str;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
