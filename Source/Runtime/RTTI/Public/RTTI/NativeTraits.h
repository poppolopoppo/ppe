#pragma once

#include "RTTI_fwd.h"

#include "MetaObject.h"
#include "RTTI/NativeTypes.Definitions-inl.h"
#include "RTTI/TypeInfos.h"
#include "RTTI/TypeTraits.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// ENatypeType traits
//----------------------------------------------------------------------------
PPE_RTTI_API PTypeTraits MakeTraits(ENativeType nativeType) NOEXCEPT;
//----------------------------------------------------------------------------
enum class ENativeType : FTypeId {
    Invalid = 0,
#define DECL_RTTI_NATIVETYPE_ENUM(_Name, T, _TypeId) _Name = _TypeId,
    FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ENUM)
#undef DECL_RTTI_NATIVETYPE_ENUM
    __Count
}; //!enum class ENativeType
//----------------------------------------------------------------------------
namespace details {
template <ENativeType typeId, typename T>
struct TNativeTypeInfos_ {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::Native< T, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, bool> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::Boolean< bool, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, FAny> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::NativeObject< FAny, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, PMetaObject> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::NativeObject< PMetaObject, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, FString> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::String< FString, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, FWString> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::String< FWString, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, FBasename> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::String< FBasename, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, FDirpath> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::String< FDirpath, static_cast<FTypeId>(typeId) >();
    }
};
template <ENativeType typeId>
struct TNativeTypeInfos_<typeId, FFilename> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::String< FFilename, static_cast<FTypeId>(typeId) >();
    }
};
} //!details
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    PPE_RTTI_API PTypeTraits Traits(TType<T>) NOEXCEPT; \
    CONSTEXPR FTypeId NativeTypeId(TType<T>) { \
        return FTypeId(_TypeId); \
    } \
    CONSTEXPR auto/* trick for constexpr */ TypeInfos(TType<T>) { \
        return details::TNativeTypeInfos_< ENativeType::_Name, T >{}; \
    }
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_TRAITS)
#undef DECL_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR FTypeId NativeTypeId() NOEXCEPT {
    return NativeTypeId(Type<T>);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
