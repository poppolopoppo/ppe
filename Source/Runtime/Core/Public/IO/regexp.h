#pragma once

#include "Core_fwd.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"

#include <locale>
#include <regex>

// This is highly implementation dependent, and
// it's going quite too deep down the rabbit hole :'(
// #TODO: third party C regex engine ? (HYPERSCAN, TRE, PCRE, RE2, ...)
// #NOTE: since we don't use capture groups this is not so important (I hope)
#define USE_PPE_REGEX_CUSTOMTRAITS (0)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_REGEX_CUSTOMTRAITS
// Needed to use FString instead of std::string, also avoids locale bullshit
template <typename _Char>
class TRegexTraits {
public:
    using char_type = _Char;
    using string_type = TBasicString<_Char>;
    using locale_type = std::locale;
    using char_class_type = typename std::ctype_base::mask;

#ifdef _MSC_VER
    // workaround a defect in M$TL which relies on non standard aliases
    using _Uelem = std::make_unsigned_t<_Char>;
    static CONSTEXPR const std::ctype_base::mask _Ch_alpha = std::ctype_base::alpha;
    static CONSTEXPR const std::ctype_base::mask _Ch_upper = std::ctype_base::upper;
#endif

    TRegexTraits() = default;

    static size_t length(const char_type* s);

    char_type translate(char_type ch) const;

    char_type translate_nocase(char_type ch) const;

    template <typename _FwdIt>
    string_type transform(_FwdIt first, _FwdIt last) const;

    template <typename _FwdIt>
    string_type transform_primary(_FwdIt first, _FwdIt last) const;

    template <typename _FwdIt>
    string_type lookup_collatename(_FwdIt first, _FwdIt last) const;

    template <class _FwdIt>
    char_class_type lookup_classname(_FwdIt first, _FwdIt last,
        bool insensitive = false) const;

    bool isctype(char_type ch, char_class_type classname) const;

    int value(char_type ch, int base) const;

    locale_type imbue(locale_type) {
        return std::locale::classic();
    }

    locale_type getloc() const {
        return std::locale::classic();
    }
};
#else
template <typename  _Char>
using TRegexTraits = std::regex_traits<_Char>;
#endif //!USE_PPE_REGEX_CUSTOMTRAITS
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicRegexp {
public:
    using regex_type = std::basic_regex<_Char, TRegexTraits<_Char>>;
    using stringview_type = TBasicStringView<_Char>;

    TBasicRegexp() NOEXCEPT;
    ~TBasicRegexp();

    explicit TBasicRegexp(regex_type&& re) NOEXCEPT;
    TBasicRegexp(const stringview_type& expr, ECase sensitive);

    TBasicRegexp(const TBasicRegexp&) = delete;
    TBasicRegexp& operator =(const TBasicRegexp&) = delete;

    TBasicRegexp(TBasicRegexp&& rvalue) NOEXCEPT;
    TBasicRegexp& operator =(TBasicRegexp&& rvalue) NOEXCEPT;

    void Compile(const stringview_type& expr, ECase sensitive);
    bool Match(const stringview_type& str) const;

    void Swap(TBasicRegexp& other) NOEXCEPT;

    inline friend void swap(TBasicRegexp& lhs, TBasicRegexp& rhs) NOEXCEPT {
        lhs.Swap(rhs);
    }

    static bool ValidateSyntax(const stringview_type& expr) NOEXCEPT;

private:
    regex_type _re;
};
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicRegexp<char>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicRegexp<wchar_t>;
//----------------------------------------------------------------------------
using FRegexp = TBasicRegexp<char>;
using FWRegexp = TBasicRegexp<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
