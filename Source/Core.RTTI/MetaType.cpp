#include "stdafx.h"

#include "MetaType.h"

#include "MetaAtom.h"
#include "MetaObject.h"

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
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    MetaTypeId MetaType< T >::Id() { STATIC_ASSERT(_TypeId == TypeId); return TypeId; } \
    const char *MetaType< T >::Name() { return STRINGIZE(_Name); } \
    T MetaType< T >::DefaultValue() { return T(); } \
    bool MetaType< T >::IsDefaultValue(const T& value) { return IsDefaultValue_(value); }
//----------------------------------------------------------------------------
#include "MetaType.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
