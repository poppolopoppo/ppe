#include "stdafx.h"

#include "RTTI_extern.h"

#include "MetaAtom.h"
#include "MetaProperty.h"
#include "MetaTypeTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    template struct MetaType<T>; \
    template struct MetaTypeTraits<T>; \
    template class  MetaTypedAtom<T>; \
    template class  MetaTypedProperty<T>; \
    template struct MetaTypeTraits< RTTI::Vector<T> >; \
    template class  MetaTypedAtom< RTTI::Vector<T> >; \
    template class  MetaTypedProperty< RTTI::Vector<T> >;
//----------------------------------------------------------------------------
#include "MetaType.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
