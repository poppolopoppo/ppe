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
    extern template struct  MetaType<T>; \
    extern template struct  MetaTypeTraits<T>; \
    extern template class   MetaTypeScalarTraits<T>; \
    /*extern template class   MetaTypePairTraits<T, T>;*/ \
    extern template class   MetaTypeVectorTraits<T>; \
    /*extern template class   MetaTypeDictionaryTraits<T, T>;*/ \
    extern template class   MetaTypedAtom<T>; \
    extern template class   MetaTypedProperty<T>; \
    extern template class   MetaFieldAccessor<T>; \
    extern template class   MetaWrappedProperty<T, MetaFieldAccessor<T> >;

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
