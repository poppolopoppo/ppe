#pragma once

#include "Container/Token.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _TokenTraits, typename _Char>
bool IsValidToken(const TBasicStringView<_Char>& content) NOEXCEPT {
    if (content.empty())
        return false;

    if (content.size() > FTokenFactory::MaxTokenLength)
        return false;

    const _TokenTraits traits = {};
    for (const _Char& ch : content)
        if (!traits.IsAllowedChar(ch))
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename _TokenTraits, typename _Char>
void SanitizeToken(const TMemoryView<_Char>& content) NOEXCEPT {
    const _TokenTraits traits = {};
    for (_Char& ch : content) {
        if (!traits.IsAllowedChar(ch))
            ch = STRING_LITERAL(_Char, '_');
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits>::FindOrAdd_(const stringview_type& str) -> const handle_type* {
    if (str.empty())
        return nullptr;

    return FindOrAdd_(lazytoken_type{ str });
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits>::FindOrAdd_(const lazytoken_type& lazy) -> const handle_type* {
    if (Unlikely(lazy.empty()))
        return nullptr;

    Assert(lazy.data());
    AssertRelease_NoAssume(lazy.Valid());

    FTokenFactory& factory = token_traits::Factory();
    const handle_type* head = nullptr;
    const handle_type* result;
    for (;;) {
        result = factory.Lookup(lazy.size(), lazy.HashValue(), head);
        if (nullptr == result) {
            result = factory.Allocate((void*)lazy.data(), lazy.size(), sizeof(_Char), lazy.HashValue(), head);
            break;
        }

        const stringview_type cmp(reinterpret_cast<const _Char*>(result->Data()), result->Length);
        if (Likely(lazy == cmp))
            break;

        head = result;
    }

    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
