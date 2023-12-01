// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#define EXPORT_PPE_RUNTIME_SERIALIZE_BASICMATCH

#include "Lexer/Match.h"

#include "Lexer/Symbols.h"

#include "IO/StringView.h"

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::TBasicMatch() NOEXCEPT
:   _symbol(FSymbols::Invalid) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::~TBasicMatch() = default;
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::TBasicMatch(TBasicMatch&& rvalue) NOEXCEPT
    : _symbol(rvalue._symbol)
    , _value(std::move(rvalue._value))
    , _site(std::move(rvalue._site))
{
    rvalue._symbol = FSymbols::Invalid;
    rvalue._site = Default;
}
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>& TBasicMatch<T>::operator =(TBasicMatch&& rvalue) NOEXCEPT {
    _symbol = rvalue._symbol;
    _value = std::move(rvalue._value);
    _site = std::move(rvalue._site);

    rvalue._symbol = FSymbols::Invalid;
    rvalue._site = Default;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::TBasicMatch(const symbol_type* symbol, value_type&& rvalue, const FSpan& site) NOEXCEPT
    : _symbol(symbol), _value(std::move(rvalue)), _site(site)
{}
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::TBasicMatch(const symbol_type* symbol, const value_type& value, const FSpan& site) NOEXCEPT
    : TBasicMatch(symbol, value_type(value), site)
{}
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::TBasicMatch(const symbol_type* symbol, value_type&& rvalue, const FLocation& start, const FLocation& stop) NOEXCEPT
    : TBasicMatch(symbol, std::move(rvalue), FSpan::FromSite(start, stop))
{}
//----------------------------------------------------------------------------
template <typename T>
TBasicMatch<T>::TBasicMatch(const symbol_type* symbol, const value_type& value, const FLocation& start, const FLocation& stop) NOEXCEPT
    : TBasicMatch(symbol, value_type(value), start, stop)
{}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicMatch<T>::Symbol() const -> const symbol_type* {
    return _symbol;
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicMatch<T>::Value() -> value_type& {
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
auto TBasicMatch<T>::Value() const -> const value_type& {
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
const FSpan& TBasicMatch<T>::Site() const {
    return _site;
}
//----------------------------------------------------------------------------
template <typename T>
FStringView TBasicMatch<T>::MakeView() const {
    return MakeStringView(_value);
}
//----------------------------------------------------------------------------
template <typename T>
bool TBasicMatch<T>::Valid() const {
    return symbol_type::Invalid != _symbol->Type();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE

EXTERN_TEMPLATE_CLASS_DEF(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FString>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FStringView>;
