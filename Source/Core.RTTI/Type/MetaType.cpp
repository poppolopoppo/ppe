#include "stdafx.h"

#include "MetaType.h"

#include "Atom/MetaAtom.h"
#include "Object/MetaObject.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(T, _TypeId) \
    MetaTypeId MetaType< T >::Id() { STATIC_ASSERT(_TypeId == TypeId); return TypeId; } \
    const char *MetaType< T >::Name() { return STRINGIZE(T); } \
    T MetaType< T >::DefaultValue() { return T(); }
//----------------------------------------------------------------------------
#include "MetaType.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
