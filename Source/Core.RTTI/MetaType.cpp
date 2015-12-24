#include "stdafx.h"

#include "MetaType.h"

#include "MetaAtom.h"
#include "MetaObject.h"

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
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    MetaTypeId MetaType< T >::Id() { STATIC_ASSERT(_TypeId == TypeId); return TypeId; } \
    const char *MetaType< T >::Name() { return STRINGIZE(_Name); } \
    T MetaType< T >::DefaultValue() { return T(); } \
    bool MetaType< T >::IsDefaultValue(const T& value) { return IsDefaultValue_(value); } \
    hash_t MetaType< T >::HashValue(const T& value) { return HashValue_(value); } \
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
