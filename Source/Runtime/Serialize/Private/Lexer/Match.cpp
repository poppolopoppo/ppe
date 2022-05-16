#include "stdafx.h"

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE

EXTERN_TEMPLATE_CLASS_DEF(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FString>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FStringView>;
