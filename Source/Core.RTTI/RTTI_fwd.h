#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/RefPtr.h"

#include "Core.RTTI/MetaObjectHelpers.h"

namespace Core {
namespace RTTI {
POOLTAG_DECL(RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
template <typename T>
class MetaTypedAtom;
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
class IMetaAtomVisitor;
class IMetaAtomConstVisitor;
//----------------------------------------------------------------------------
class MetaClass;
class MetaClassHashMap;
class MetaClassName;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaObject);
class MetaObjectName;
//----------------------------------------------------------------------------
class MetaProperty;
class MetaPropertyName;
template <typename T>
class MetaTypedProperty;
//----------------------------------------------------------------------------
template <typename T>
struct MetaType;
enum class MetaTypeFlags : u32;
template <typename _From, typename _To>
struct MetaTypePromote;
template <typename T>
struct MetaTypeTraits;
class IMetaTypeVirtualTraits;
class AbstractMetaTypeScalarTraits;
class AbstractMetaTypePairTraits;
class AbstractMetaTypeVectorTraits;
class AbstractMetaTypeDictionaryTraits;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
