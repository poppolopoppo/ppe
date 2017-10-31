#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
POOL_TAG_DECL(RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAtom;
//----------------------------------------------------------------------------
class ITypeTraits;
//----------------------------------------------------------------------------
class IScalarTraits;
class IPairTraits;
class IListTraits;
class IDicoTraits;
//----------------------------------------------------------------------------
class FMetaClass;
class FMetaFunction;
class FMetaParameter;
class FMetaProperty;
//----------------------------------------------------------------------------
class FMetaClassHandle;
class FMetaNamespace;
FWD_REFPTR(MetaTransaction);
class FMetaDatabase;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
