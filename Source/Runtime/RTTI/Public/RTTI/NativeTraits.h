#pragma once

#include "RTTI_fwd.h"

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
template <typename T, FTypeId _NativeType, ETypeFlags _TypeFlags>
static CONSTEXPR const PTypeInfos MakeScalarTypeInfos = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
    return FTypeInfos{ _NativeType, FTypeInfos::BasicInfos<T>(ETypeFlags::Scalar + _TypeFlags) };
};
//----------------------------------------------------------------------------
template <typename T, FTypeId _NativeType>
static CONSTEXPR const PTypeInfos MakeBooleanTypeInfos = MakeScalarTypeInfos<T, _NativeType, ETypeFlags::Boolean + ETypeFlags::Native>;
template <typename T, FTypeId _NativeType>
static CONSTEXPR const PTypeInfos MakeEnumTypeInfos = MakeScalarTypeInfos<T, _NativeType, ETypeFlags::Enum + ETypeFlags::Native>;
template <typename T, FTypeId _NativeType>
static CONSTEXPR const PTypeInfos MakeNativeTypeInfos = MakeScalarTypeInfos<T, _NativeType, ETypeFlags::Native>;
template <typename T, FTypeId _NativeType>
static CONSTEXPR const PTypeInfos MakeNativeObjectTypeInfos = MakeScalarTypeInfos<T, _NativeType, ETypeFlags::Native + ETypeFlags::Object>;
template <typename T, FTypeId _NativeType>
static CONSTEXPR const PTypeInfos MakeObjectTypeInfos = MakeScalarTypeInfos<T, _NativeType, ETypeFlags::Object>;
template <typename T, FTypeId _NativeType>
static CONSTEXPR const PTypeInfos MakeStringTypeInfos = MakeScalarTypeInfos<T, _NativeType, ETypeFlags::String + ETypeFlags::Native>;
//----------------------------------------------------------------------------
enum class ENativeType : FTypeId {
    Unknown = 0,
#define DECL_RTTI_NATIVETYPE_ENUM(_Name, T, _TypeId) _Name = _TypeId,
    FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ENUM)
#undef DECL_RTTI_NATIVETYPE_ENUM
    __Count
}; //!enum class ENativeType
//----------------------------------------------------------------------------
namespace details {
template <ENativeType typeId, typename T>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< T >) {
    return MakeNativeTypeInfos< T, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< bool >) {
    return MakeBooleanTypeInfos< bool, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< FAny >) {
    return MakeNativeObjectTypeInfos< FAny, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< PMetaObject >) {
    return MakeNativeObjectTypeInfos< PMetaObject, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< FString >) {
    return MakeStringTypeInfos< FString, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< FWString >) {
    return MakeStringTypeInfos< FWString, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< FBasename >) {
    return MakeStringTypeInfos< FBasename, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< FDirpath >) {
    return MakeStringTypeInfos< FDirpath, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId>
CONSTEXPR PTypeInfos NativeTypeInfos_(TTypeTag< FFilename >) {
    return MakeStringTypeInfos< FFilename, static_cast<FTypeId>(typeId) >;
}
template <ENativeType typeId, typename T>
struct TDeferredTypeInfos_ {
    CONSTEXPR FTypeInfos operator ()() const NOEXCEPT {
        return NativeTypeInfos_<typeId>(TypeTag< T >)();
    }
};
} //!details
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    PPE_RTTI_API PTypeTraits RTTI_Traits(TTypeTag< T >) NOEXCEPT; \
    CONSTEXPR FTypeId NativeTypeId(TTypeTag< T >) { \
        return FTypeId(_TypeId); \
    } \
    CONSTEXPR auto/* TDeferredTypeInfos_<> */ RTTI_TypeInfos(TTypeTag< T >) { \
        return details::TDeferredTypeInfos_< ENativeType::_Name, T >{}; \
    }
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_TRAITS)
#undef DECL_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR FTypeId NativeTypeId() NOEXCEPT {
    return NativeTypeId(TypeTag< T >);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR FTypeInfos MakeTypeInfos() NOEXCEPT {
    return RTTI_TypeInfos(TypeTag< T >)();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
