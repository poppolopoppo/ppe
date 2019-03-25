#pragma once

#include "Core_fwd.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"

#include <regex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*
template <typename _Char>
class TRegexTraits : public std::regex_traits<_Char> {
public:
    using parent_type = std::regex_traits<_Char>;

    using typename parent_type::char_type;
    using typename parent_type::locale_type;
    using typename parent_type::char_class_type;

    using parent_type::length;
    using parent_type::translate;
    using parent_type::translate_nocase;
    //using parent_type::transform;
    using parent_type::transform_primary;
    using parent_type::lookup_collatename;
    using parent_type::lookup_classname;
    using parent_type::isctype;
    using parent_type::value;
    using parent_type::imbue;
    using parent_type::getloc;

    using string_type = TBasicString<_Char>;

    TRegexTraits() = default;

    template <typename _FwdIt>
    string_type transform(_FwdIt first, _FwdIt last) {
        return std::use_facet<std::collate<char_type>>(getloc())
            .transform() // meh, returns std::string ... #TODO
    }
};
*/
template <typename _Char>
using TRegexTraits = std::regex_traits<_Char>;
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
