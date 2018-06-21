#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
template <typename T>
struct TInSituPtr;
namespace RTTI {
POOL_TAG_DECL(RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FName;
struct FPathName;
//----------------------------------------------------------------------------
class FAny;
class FAtom;
//----------------------------------------------------------------------------
class ITypeTraits;
using PTypeTraits = TInSituPtr<ITypeTraits>;
//----------------------------------------------------------------------------
class IScalarTraits;
class ITupleTraits;
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
FWD_REFPTR(AtomHeap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
