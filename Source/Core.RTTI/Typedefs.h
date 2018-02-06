#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Token.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Maths/PrimeNumbers.h" // FClassId


namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const { return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.'; }
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DEF(FName, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(CORE_RTTI_API, FBinaryData, RAWSTORAGE_ALIGNED(RTTI, u8, ALLOCATION_BOUNDARY));
//----------------------------------------------------------------------------
// /!\ not guaranteed to be stable : depends on initialization order
using FClassId = TPrimeNumberProduct<class FMetaClass>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FBinaryData& bindata);
CORE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FBinaryData& bindata);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
