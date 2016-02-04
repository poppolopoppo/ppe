#include "stdafx.h"

#include "MetaType.h"

#include "MetaAtom.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h"

#include "Core/Container/Hash.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static bool IsDefaultValue_(const T& value) {
    return T() == value;
}
//----------------------------------------------------------------------------
static bool IsDefaultValue_(const PMetaAtom& atom) {
    return (nullptr == atom || atom->IsDefaultValue() );
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    MetaTypeId MetaType< T >::Id() { STATIC_ASSERT(_TypeId == TypeId); return TypeId; } \
    const char *MetaType< T >::Name() { return STRINGIZE(_Name); } \
    T MetaType< T >::DefaultValue() { return T(); } \
    bool MetaType< T >::IsDefaultValue(const T& value) { return IsDefaultValue_(value); } \
    hash_t MetaType< T >::HashValue(const T& value) { return HashValue_(value); } \
    bool MetaType< T >::DeepEquals(const T& lhs, const T& rhs) { return DeepEquals_(lhs, rhs); } \
    const MetaTypeScalarTraits< T >* MetaType< T >::VirtualTraits() { return MetaTypeScalarTraits< T >::Instance(); }
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const AbstractMetaTypeScalarTraits* ScalarTraitsFromTypeId(MetaTypeId typeId) {
    switch (typeId) {
#define CASE_METATYPE_ID(_Name, T, _TypeId, _Unused) \
    case _TypeId: return MetaType< T >::VirtualTraits();
FOREACH_CORE_RTTI_NATIVE_TYPES(CASE_METATYPE_ID)
#undef CASE_METATYPE_ID

    default:
        break;
    }
    AssertNotReached(); // typeId is not a native scalar !
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaTypeInfo ScalarTypeInfoFromTypeId(MetaTypeId typeId) {
    switch (typeId) {
#define CASE_METATYPE_ID(_Name, T, _TypeId, _Unused) \
    case _TypeId: return TypeInfo< T >();
FOREACH_CORE_RTTI_NATIVE_TYPES(CASE_METATYPE_ID)
#undef CASE_METATYPE_ID

    default:
        break;
    }
    AssertNotReached(); // typeId is not a native scalar !
    return MetaTypeInfo();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
