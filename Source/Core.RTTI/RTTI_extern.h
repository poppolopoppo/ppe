#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeTraits.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _DEF_METATYPE_SCALAR_EXTERN_TEMPLATE(T) \
    extern template struct  TMetaType<T>; \
    extern template struct  TMetaTypeTraits<T>; \
    extern template class   TMetaTypeScalarTraits<T>; \
    /*extern template class   TMetaTypePairTraits<T, T>;*/ \
    extern template class   TMetaTypeVectorTraits<T>; \
    /*extern template class   TMetaTypeDictionaryTraits<T, T>;*/ \
    extern template class   TMetaTypedAtom<T>; \
    extern template class   TMetaTypedProperty<T>; \
    extern template class   TMetaFieldAccessor<T>; \
    extern template class   TMetaWrappedProperty<T, TMetaFieldAccessor<T> >;

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    _DEF_METATYPE_SCALAR_EXTERN_TEMPLATE(T)
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
#undef _DEF_METATYPE_SCALAR_EXTERN_TEMPLATE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
