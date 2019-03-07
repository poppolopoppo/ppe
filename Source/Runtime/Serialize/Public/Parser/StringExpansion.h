#pragma once

#include "Serialize_fwd.h"

#include "RTTI_fwd.h"
#include "Lexer/Location.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_SERIALIZE_API
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& scalar, const RTTI::IScalarTraits& traits, const Lexer::FSpan& site);
//----------------------------------------------------------------------------
PPE_SERIALIZE_API
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& tuple, const RTTI::ITupleTraits& traits, const Lexer::FSpan& site);
//----------------------------------------------------------------------------
PPE_SERIALIZE_API
FString PerformStringExpansion(const FString& fmt, const RTTI::FAtom& list, const RTTI::IListTraits& traits, const Lexer::FSpan& site);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE