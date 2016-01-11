#include "stdafx.h"

#include "MetaAtom.h"
#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaType.h"
#include "MetaTypeTraits.h"

#include "RTTI_extern.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _DEF_METATYPE_SCALAR_INSTANTIATE_TEMPLATE(T) \
    STATIC_ASSERT(false == MetaTypeTraits<T>::Wrapping); \
    /*extern*/ template struct  MetaType<T>; \
    /*extern*/ template struct  MetaTypeTraits<T>; \
    /*extern*/ template class   MetaTypeScalarTraits<T>; \
    /*extern*/ template class   MetaTypePairTraits<T, T>; \
    /*extern*/ template class   MetaTypeVectorTraits<T>; \
    /*extern*/ template class   MetaTypeDictionaryTraits<T, T>; \
    /*extern*/ template class   MetaTypedAtom<T>; \
    /*extern*/ template class   MetaTypedProperty<T>; \
    /*extern*/ template class   MetaFieldAccessor<T>; \
    /*extern*/ template class   MetaWrappedProperty<T, MetaFieldAccessor<T> >;

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    _DEF_METATYPE_SCALAR_INSTANTIATE_TEMPLATE(T)
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
#undef _DEF_METATYPE_SCALAR_INSTANTIATE_TEMPLATE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
