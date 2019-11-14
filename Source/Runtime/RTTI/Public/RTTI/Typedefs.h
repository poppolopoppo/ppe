
#pragma once

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
BASICTOKEN_CLASS_DECL(PPE_RTTI_API, FName, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FBinaryData, RAWSTORAGE_ALIGNED(NativeTypes, u8, ALLOCATION_BOUNDARY));
//----------------------------------------------------------------------------
// /!\ not guaranteed to be stable : depends on initialization order
using FClassId = TPrimeNumberProduct<class FMetaClass>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FPathName {
    FName Namespace;
    FName Identifier;

    bool empty() const NOEXCEPT { return (Identifier.empty()); }

    static FPathName FromObject(const FMetaObject& obj) NOEXCEPT;
    static bool Parse(FPathName* pathName, const FStringView& text);

    inline friend bool operator ==(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return ((lhs.Identifier == rhs.Identifier) & (lhs.Namespace == rhs.Namespace));
    }
    inline friend bool operator !=(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    inline friend bool operator < (const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (lhs.Namespace == rhs.Namespace ? lhs.Identifier < rhs.Identifier : lhs.Namespace < rhs.Namespace);
    }
    inline friend bool operator >=(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (not operator < (lhs, rhs));
    }

    inline friend hash_t hash_value(const FPathName& pathName) {
        return hash_tuple(pathName.Namespace, pathName.Identifier);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use this as a constructor parameter for RTTI only construction pass
//      ex:     explicit MyClass(FConstructorTag);
//----------------------------------------------------------------------------
struct FConstructorTag {};
CONSTEXPR FConstructorTag ConstructorTag;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(RTTI::FName)
PPE_ASSUME_TYPE_AS_POD(RTTI::FPathName)
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
