#include "stdafx.h"

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE

EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) PPE::Lexer::TBasicMatch<PPE::FString>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) PPE::Lexer::TBasicMatch<PPE::FStringView>;
