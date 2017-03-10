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
    extern CORE_RTTI_API template struct  TMetaType<T>; \
    extern CORE_RTTI_API template struct  TMetaTypeTraits<T>; \
    extern CORE_RTTI_API template class   TMetaTypeScalarTraits<T>; \
    /*extern CORE_RTTI_API template class   TMetaTypePairTraits<T, T>;*/ \
    extern CORE_RTTI_API template class   TMetaTypeVectorTraits<T>; \
    /*extern CORE_RTTI_API template class   TMetaTypeDictionaryTraits<T, T>;*/ \
    extern CORE_RTTI_API template class   TMetaTypedAtom<T>; \
    extern CORE_RTTI_API template class   TMetaTypedProperty<T>; \
    extern CORE_RTTI_API template class   TMetaFieldAccessor<T>; \
    extern CORE_RTTI_API template class   TMetaWrappedProperty<T, TMetaFieldAccessor<T> >;

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
