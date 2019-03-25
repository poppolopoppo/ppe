#include "stdafx.h"

#include "IO/Regexp.h"

#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp() NOEXCEPT
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::~TBasicRegexp()
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(regex_type&& re) NOEXCEPT
:   _re(std::move(re))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(const stringview_type& expr, ECase icase) {
    Compile(expr, icase);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicRegexp<_Char>::TBasicRegexp(TBasicRegexp&& rvalue) NOEXCEPT
:   TBasicRegexp(std::move(rvalue._re))
{}
//----------------------------------------------------------------------------
template <typename _Char>
auto TBasicRegexp<_Char>::operator =(TBasicRegexp&& rvalue) NOEXCEPT -> TBasicRegexp& {
    _re = std::move(rvalue._re);
    return (*this);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicRegexp<_Char>::Compile(const stringview_type& expr, ECase icase) {
    typename regex_type::flag_type flags = regex_type::ECMAScript | regex_type::optimize;
    if (icase == ECase::Insensitive)
        flags |= regex_type::icase;

    _re = regex_type(expr.begin(), expr.end(), flags);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicRegexp<_Char>::Match(const stringview_type& str) const {
    return std::regex_match(str.begin(), str.end(), _re, std::regex_constants::match_default);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicRegexp<_Char>::Swap(TBasicRegexp& other) NOEXCEPT {
    std::swap(_re, other._re);
}
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicRegexp<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicRegexp<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
