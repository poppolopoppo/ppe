#include "stdafx.h"

#include "MetaType.h"

#include "MetaAtom.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h"

#include "Core/Container/Hash.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static T DefaultValue_(std::true_type) {
    return T(Meta::FForceInit{});
}
//----------------------------------------------------------------------------
template <typename T>
static T DefaultValue_(std::false_type) {
    return T();
}
//----------------------------------------------------------------------------
template <typename T>
static T DefaultValue_() {
    return DefaultValue_<T>(Meta::has_forceinit_constructor<T>{});
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static bool IsDefaultValue_(const T& value, std::false_type) {
    return (T() == value);
}
//----------------------------------------------------------------------------
template <typename T>
static bool IsDefaultValue_(const T& value, std::true_type) {
    return (T(Meta::FForceInit{}) == value);
}
//----------------------------------------------------------------------------
template <typename T>
static bool IsDefaultValue_(const T& value) {
    return IsDefaultValue_(value, Meta::has_forceinit_constructor<T>{});
}
//----------------------------------------------------------------------------
static bool IsDefaultValue_(const PMetaAtom& atom) {
    return (nullptr == atom || atom->IsDefaultValue() );
}
//----------------------------------------------------------------------------
static bool IsDefaultValue_(const FName& name) {
    return name.empty();
}
//----------------------------------------------------------------------------
static bool IsDefaultValue_(const FBinaryData& rawdata) {
    return rawdata.empty();
}
//----------------------------------------------------------------------------
static bool IsDefaultValue_(const FOpaqueData& opaqueData) {
    return opaqueData.empty();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static hash_t HashValue_(const T& value) {
    using Core::hash_value;
    return hash_value(value);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static bool DeepEquals_(const T& lhs, const T& rhs) {
    return (lhs == rhs);
}
//----------------------------------------------------------------------------
static bool DeepEquals_(const PMetaAtom& lhs, const PMetaAtom& rhs) {
    return (lhs != nullptr && rhs != nullptr)
        ? lhs->DeepEquals(rhs.get())
        : lhs == rhs;
}
//----------------------------------------------------------------------------
static bool DeepEquals_(const PMetaObject& lhs, const PMetaObject& rhs) {
    return (lhs != nullptr && rhs != nullptr)
        ? DeepEquals(*lhs, *rhs)
        : lhs == rhs;
}
//----------------------------------------------------------------------------
static bool DeepEquals_(const FOpaqueData& lhs, const FOpaqueData& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    for (const auto& it : lhs) {
        PMetaAtom value;
        if (not rhs.TryGet(it.first, &value))
            return false;

        if (not DeepEquals_(it.second, value))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    FStringView TMetaType< T >::Name() { return MakeStringView(STRINGIZE(_Name)); } \
    T TMetaType< T >::DefaultValue() { return DefaultValue_<T>(); } \
    bool TMetaType< T >::IsDefaultValue(const T& value) { return IsDefaultValue_(value); } \
    hash_t TMetaType< T >::HashValue(const T& value) { return HashValue_(value); } \
    bool TMetaType< T >::DeepEquals(const T& lhs, const T& rhs) { return DeepEquals_(lhs, rhs); } \
    const TMetaTypeScalarTraits< T >* TMetaType< T >::VirtualTraits() { return TMetaTypeScalarTraits< T >::Instance(); }
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FAbstractMetaTypeScalarTraits* ScalarTraitsFromTypeId(FMetaTypeId typeId) {
    switch (typeId) {
#define CASE_METATYPE_ID(_Name, T, _TypeId, _Unused) \
    case _TypeId: return TMetaType< T >::VirtualTraits();
FOREACH_CORE_RTTI_NATIVE_TYPES(CASE_METATYPE_ID)
#undef CASE_METATYPE_ID

    default:
        break;
    }
    AssertNotReached(); // typeId is not a native scalar !
    return nullptr;
}
//----------------------------------------------------------------------------
FMetaTypeInfo ScalarTypeInfoFromTypeId(FMetaTypeId typeId) {
    switch (typeId) {
#define CASE_METATYPE_ID(_Name, T, _TypeId, _Unused) \
    case _TypeId: return TypeInfo< T >();
FOREACH_CORE_RTTI_NATIVE_TYPES(CASE_METATYPE_ID)
#undef CASE_METATYPE_ID

    default:
        break;
    }
    AssertNotReached(); // typeId is not a native scalar !
    return FMetaTypeInfo();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const RTTI::FName& name) {
    return StringFormat("Name:{0}", name.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const RTTI::FBinaryData& rawdata) {
    return hash_mem(rawdata.MakeConstView());
}
//----------------------------------------------------------------------------
FString ToString(const RTTI::FBinaryData& rawdata) {
    return StringFormat("BinaryData:{0}", rawdata.SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const RTTI::FOpaqueData& opaqueData) {
    return StringFormat("OpaqueData:{0}",
        static_cast<const RTTI::TDictionary<RTTI::FName, RTTI::PMetaAtom>&>(opaqueData));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
