
#pragma once

#include "RTTI_fwd.h"

#include "Container/RawStorage.h"
#include "Container/Token.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Maths/PrimeNumbers.h" // FClassId

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNameTokenTraits {
public:
    bool IsAllowedChar(char ch) const { return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.' || ch == '/' || ch == '$'; }
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
inline CONSTEXPR FConstructorTag ConstructorTag;
//----------------------------------------------------------------------------
template <typename T, class = Meta::TEnableIf<Meta::is_pod_v<T>> >
using TRawData = RAWSTORAGE_ALIGNED(BinaryData, T, ALLOCATION_BOUNDARY);
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FBinaryData, TRawData<u8>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLazyPathName {
    PPE_ASSUME_FRIEND_AS_POD(RTTI::FLazyPathName)

    FLazyName Namespace;
    FLazyName Identifier;

    bool empty() const NOEXCEPT { return (Identifier.empty()); }
    bool Valid() const NOEXCEPT { return (Identifier.Valid() && (Namespace.empty() || Namespace.Valid())); }

    static PPE_RTTI_API bool Parse(FLazyPathName* pathName, const FStringView& text, bool withPrefix = true);

    friend bool operator ==(const FLazyPathName& lhs, const FLazyPathName& rhs) NOEXCEPT {
        return ((lhs.Identifier == rhs.Identifier) && (lhs.Namespace == rhs.Namespace));
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
        return ((lhs.Identifier == rhs.Identifier) && (lhs.Namespace == rhs.Namespace));
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
        return ((lhs.Identifier == rhs.Identifier) && (lhs.Namespace == rhs.Namespace));
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
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const FBinaryData& bindata);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const FBinaryData& bindata);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const FLazyPathName& pathName);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const FLazyPathName& pathName);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const FPathName& pathName);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const FPathName& pathName);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_RTTI_API bool operator >>(const FStringConversion& iss, FBinaryData* bindata);
NODISCARD PPE_RTTI_API bool operator >>(const FWStringConversion& iss, FBinaryData* bindata);
//----------------------------------------------------------------------------
NODISCARD PPE_RTTI_API bool operator >>(const FStringConversion& iss, FLazyPathName* pathName);
NODISCARD PPE_RTTI_API bool operator >>(const FWStringConversion& iss, FLazyPathName* pathName);
//----------------------------------------------------------------------------
NODISCARD PPE_RTTI_API bool operator >>(const FStringConversion& iss, FPathName* pathName);
NODISCARD PPE_RTTI_API bool operator >>(const FWStringConversion& iss, FPathName* pathName);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline RTTI::FName operator ""_rtti (const char* str, size_t len) {
    return RTTI::FName{ FStringView(str, len) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
