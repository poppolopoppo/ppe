#pragma once

#include "RTTI.h"

#include "RTTI_fwd.h"

#include "Container/RawStorage.h"
#include "Container/Token.h"
#include "IO/TextWriter_fwd.h"
#include "Maths/PrimeNumbers.h" // FClassId

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const { return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.' || ch == '/'; }
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DEF(FName, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FBinaryData, RAWSTORAGE_ALIGNED(NativeTypes, u8, ALLOCATION_BOUNDARY));
//----------------------------------------------------------------------------
// /!\ not guaranteed to be stable : depends on initialization order
using FClassId = TPrimeNumberProduct<class FMetaClass>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FPathName {
    FName Transaction;
    FName Identifier;

    FPathName() NOEXCEPT {}
    FPathName(const FName& transaction, const FName& identifier) NOEXCEPT
        : Transaction(transaction)
        , Identifier(identifier)
    {}

    /* not explicit on purpose */FPathName(const FMetaObject& obj);

    bool empty() const NOEXCEPT { return (Identifier.empty()); }

    static bool Parse(FPathName* pathName, const FStringView& text);

    inline friend bool operator ==(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (lhs.Identifier == rhs.Identifier && lhs.Transaction == rhs.Transaction);
    }
    inline friend bool operator !=(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    inline friend hash_t hash_value(const FPathName& pathName) {
        return hash_tuple(pathName.Transaction, pathName.Identifier);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use this as a constructor parameter for RTTI only construction pass
//      ex:     explicit MyClass(FConstructorTag);
//----------------------------------------------------------------------------
struct FConstructorTag {};
constexpr FConstructorTag ConstructorTag;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FBinaryData& bindata);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FBinaryData& bindata);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FPathName& pathName);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FPathName& pathName);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
