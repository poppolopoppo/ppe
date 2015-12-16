#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    extern template struct  MetaType<T>; \
    extern template struct  MetaTypeTraits<T>; \
    extern template class   MetaTypedAtom<T>; \
    extern template class   MetaTypedProperty<T>; \
    extern template struct  MetaType< RTTI::Vector<T> >; \
    extern template struct  MetaTypeTraits< RTTI::Vector<T> >; \
    extern template class   MetaTypedAtom< RTTI::Vector<T> >; \
    extern template class   MetaTypedProperty< RTTI::Vector<T> >;
//----------------------------------------------------------------------------
#include "MetaType.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
