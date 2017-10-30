#pragma once

#include "Core/Container/Token.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _TokenTraits>
bool ValidateToken(const TBasicStringView<_Char>& content) {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits>::FindOrAdd_(const stringview_type& str) -> const handle_type* {
    if (str.empty())
        return nullptr;

    Assert(str.size() > 0);
    Assert(ValidateToken<_Char, _TokenTraits>(str));

    const hash_t hash = hasher_type{}(str);

    FTokenFactory& factory = factory_type::Instance();
    const handle_type* head = nullptr;
    const handle_type* result;
    for (;;) {
        result = factory.Lookup(str.size(), hash, head);
        if (nullptr == result)
            break;

        const stringview_type cmp(reinterpret_cast<const _Char*>(result->Data()), result->Length);
        if (equalto_type{}(str, cmp))
            break;

        head = result;
    }

    if (nullptr == result)
        result = factory.Allocate((void*)str.data(), str.size(), sizeof(_Char), hash, head);

    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
