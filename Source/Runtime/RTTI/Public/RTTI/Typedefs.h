
#pragma once

#include "RTTI_fwd.h"

#include "Container/AssociativeVector.h"
#include "Container/RawStorage.h"
#include "Container/Token.h"
#include "Container/Vector.h"
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
BASICTOKEN_CLASS_DECL(PPE_RTTI_API, Name, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
// /!\ not guaranteed to be stable : depends on initialization order
using FClassId = TPrimeNumberProduct<class FMetaClass>;
//----------------------------------------------------------------------------
// Use this as a constructor parameter for RTTI only construction pass
//      ex:     explicit MyClass(FConstructorTag);
//----------------------------------------------------------------------------
struct FConstructorTag {};
CONSTEXPR FConstructorTag ConstructorTag;
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FBinaryData, RAWSTORAGE_ALIGNED(BinaryData, u8, ALLOCATION_BOUNDARY));
//----------------------------------------------------------------------------
// Instantiate official "opaque" data containers, TRawInlineAllocator<> is used InSitu without defining FAny
namespace details {
constexpr int _sizeof_FAny = (4 * sizeof(intptr_t));
using FOpaqueArrayContainer_ = TVector<
    FAny,
    TRawInlineAllocator<MEMORYDOMAIN_TAG(OpaqueData), _sizeof_FAny * 3>
>;
using FOpaqueDataContainer_ = TAssociativeVector<
    FName, FAny,
    Meta::TEqualTo<FName>,
    TVector<
        TPair<FName, FAny>,
        TRawInlineAllocator<MEMORYDOMAIN_TAG(OpaqueData), (sizeof(FName) + _sizeof_FAny) * 3>
    >
 >;
} //!details
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FOpaqueArray, details::FOpaqueArrayContainer_);
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FOpaqueData, details::FOpaqueDataContainer_);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLazyPathName {
    PPE_ASSUME_FRIEND_AS_POD(RTTI::FLazyPathName)

    FLazyName Namespace;
    FLazyName Identifier;

    bool empty() const NOEXCEPT { return (Identifier.empty()); }

    static PPE_RTTI_API bool Parse(FLazyPathName* pathName, const FStringView& text);

    friend bool operator ==(const FLazyPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return ((lhs.Identifier == rhs.Identifier) & (lhs.Namespace == rhs.Namespace));
    }
    friend bool operator !=(const FLazyPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend bool operator < (const FLazyPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return (lhs.Namespace == rhs.Namespace ? lhs.Identifier < rhs.Identifier : lhs.Namespace < rhs.Namespace);
    }
    friend bool operator >=(const FLazyPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return (not operator < (lhs, rhs));
    }

    friend hash_t hash_value(const FLazyPathName& pathName) NOEXCEPT {
        return hash_tuple(pathName.Namespace, pathName.Identifier);
    }
};
//----------------------------------------------------------------------------
struct FPathName {
    PPE_ASSUME_FRIEND_AS_POD(RTTI::FPathName)

    FName Namespace;
    FName Identifier;

    bool empty() const NOEXCEPT { return (Identifier.empty()); }

    static PPE_RTTI_API FPathName FromObject(const FMetaObject& obj) NOEXCEPT;
    static PPE_RTTI_API bool Parse(FPathName* pathName, const FStringView& text);

    friend bool operator ==(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return ((lhs.Identifier == rhs.Identifier) & (lhs.Namespace == rhs.Namespace));
    }
    friend bool operator !=(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend bool operator < (const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (lhs.Namespace == rhs.Namespace ? lhs.Identifier < rhs.Identifier : lhs.Namespace < rhs.Namespace);
    }
    friend bool operator >=(const FPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (not operator < (lhs, rhs));
    }

    friend bool operator ==(const FPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return ((lhs.Identifier == rhs.Identifier) & (lhs.Namespace == rhs.Namespace));
    }
    friend bool operator !=(const FPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend bool operator < (const FLazyPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (lhs.Namespace == rhs.Namespace ? lhs.Identifier < rhs.Identifier : lhs.Namespace < rhs.Namespace);
    }
    friend bool operator >=(const FLazyPathName& lhs, const FPathName& rhs) NOEXCEPT {
        return (not operator < (lhs, rhs));
    }

    friend hash_t hash_value(const FPathName& pathName) NOEXCEPT {
        return hash_tuple(pathName.Namespace, pathName.Identifier);
    }
};
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
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FLazyPathName& pathName);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FLazyPathName& pathName);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FPathName& pathName);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FPathName& pathName);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
