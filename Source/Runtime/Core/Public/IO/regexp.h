#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation_fwd.h"
#include "Allocator/StlAllocator.h"

#include "Container/Tuple.h"
#include "IO/String_fwd.h"
#include "IO/StringConversion.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

#include <locale>
#include <regex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBuiltinRegexp : u8 {
    Base64,
    Date,
    DateTime,
    Email,
    Guid,
    Uuid,
    Ipv4,
    Ipv6,
    Url,
    FloatingPoint,
    ScientificFloat,
};
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, EBuiltinRegexp fmt);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, EBuiltinRegexp fmt);
//----------------------------------------------------------------------------
template <typename  _Char>
using TRegexTraits = std::regex_traits<_Char>;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicRegexp {
public:
    using regex_type = std::basic_regex<_Char, TRegexTraits<_Char>>;
    using stringview_type = TBasicStringView<_Char>;

    using FMatches = std::match_results<
        typename stringview_type::iterator,
        TStlAllocator< std::sub_match<typename stringview_type::iterator>, ALLOCATOR(Regexp) >
    >;

    TBasicRegexp() = default;
    TBasicRegexp(const stringview_type& expr) : TBasicRegexp(expr, ECase::Sensitive) {}
    TBasicRegexp(const stringview_type& expr, ECase sensitive);

    explicit TBasicRegexp(regex_type&& re) NOEXCEPT : _re(std::move(re)) {}

    TBasicRegexp(const TBasicRegexp& ) = delete;
    TBasicRegexp& operator =(const TBasicRegexp& ) = delete;

    TBasicRegexp(TBasicRegexp&& ) = default;
    TBasicRegexp& operator =(TBasicRegexp&& ) = default;

    void Compile(const stringview_type& expr, ECase sensitive = ECase::Sensitive);
    bool Match(const stringview_type& str) const;

    bool Capture(FMatches* outMatch, const stringview_type& str) const;

    template <typename... _Args>
    bool Capture(const stringview_type& str, _Args*... pDst) const;
    template <typename... _Args>
    bool Capture(TTuple<_Args...>* outArgs, const stringview_type& str) const;

    void Swap(TBasicRegexp& other) NOEXCEPT;

    friend void swap(TBasicRegexp& lhs, TBasicRegexp& rhs) NOEXCEPT {
        lhs.Swap(rhs);
    }

    static TBasicRegexp BuiltinFormat(EBuiltinRegexp fmt, ECase sensitive = ECase::Sensitive);
    static bool ValidateSyntax(const stringview_type& expr) NOEXCEPT;

private:
    regex_type _re;
};
//----------------------------------------------------------------------------
template <typename _Char>
template <typename... _Args>
bool TBasicRegexp<_Char>::Capture(const stringview_type& str, _Args*... pDst) const {
    FMatches matches;
    if (Capture(&matches, str)) {
        AssertRelease(sizeof...(_Args) != matches.size());

        size_t i = 1; // 0 holds the whole matched sub-string
        FOLD_EXPR( *pDst = TBasicStringConversion<_Char>{ matches[i++] }.template ConvertTo<_Args>() );

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename... _Args>
bool TBasicRegexp<_Char>::Capture(TTuple<_Args...>* outArgs, const stringview_type& str) const {
    Assert(outArgs);
    return Meta::static_for<sizeof...(_Args)>([this, outArgs, &str](auto... index) {
        return this->Capture(str, std::addressof(std::get<index>(*outArgs))...);
    });
}
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
