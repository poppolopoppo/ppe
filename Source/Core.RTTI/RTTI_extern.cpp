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
    STATIC_ASSERT(false == TMetaTypeTraits<T>::Wrapping); \
    /*extern*/ CORE_RTTI_API template struct  TMetaType<T>; \
    /*extern*/ CORE_RTTI_API template struct  TMetaTypeTraits<T>; \
    /*extern*/ CORE_RTTI_API template class   TMetaTypeScalarTraits<T>; \
    /*extern*//* CORE_RTTI_API template class   TMetaTypePairTraits<T, T>; */ \
    /*extern*/ CORE_RTTI_API template class   TMetaTypeVectorTraits<T>; \
    /*extern*//* CORE_RTTI_API template class   TMetaTypeDictionaryTraits<T, T>; */ \
    /*extern*/ CORE_RTTI_API template class   TMetaTypedAtom<T>; \
    /*extern*/ CORE_RTTI_API template class   TMetaTypedProperty<T>; \
    /*extern*/ CORE_RTTI_API template class   TMetaFieldAccessor<T>; \
    /*extern*/ CORE_RTTI_API template class   TMetaWrappedProperty<T, TMetaFieldAccessor<T> >;

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
