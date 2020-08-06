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
template <ENativeType _NativeType, typename T>
struct TNativeTypeInfos {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::Native< T, FTypeId(_NativeType) >();
    }
};
template <ENativeType _NativeType>
struct TNativeTypeInfos<_NativeType, FAny> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::NativeObject< FAny, FTypeId(_NativeType) >();
    }
};
template <ENativeType _NativeType>
struct TNativeTypeInfos<_NativeType, PMetaObject> {
    CONSTEXPR FTypeInfos operator ()() const {
        return FTypeHelpers::NativeObject< PMetaObject, FTypeId(_NativeType) >();
    }
};
} //!details
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    PPE_RTTI_API PTypeTraits Traits(TType<T>) NOEXCEPT; \
    CONSTEXPR FTypeId NativeTypeId(TType<T>) { return FTypeId(_TypeId); } \
    CONSTEXPR auto TypeInfos(TType<T>) { \
        return details::TNativeTypeInfos< ENativeType::_Name, T >{}; \
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
