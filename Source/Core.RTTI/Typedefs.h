#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Token.h"

#include "Core/Maths/PrimeNumbers.h" // FClassId

#include <iosfwd>

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
INSTANTIATE_CLASS_TYPEDEF(FBinaryData, RAWSTORAGE_ALIGNED(RTTI, u8, 16));
//----------------------------------------------------------------------------
//INSTANTIATE_CLASS_TYPEDEF(FOpaqueData, ASSOCIATIVE_VECTOR(RTTI, FName, PMetaAtom));
//----------------------------------------------------------------------------
// /!\ not guaranted to be stable : dependant of initialization order
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
CORE_RTTI_API std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const RTTI::FBinaryData& bindata);
CORE_RTTI_API std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const RTTI::FBinaryData& bindata);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
