#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/RefPtr.h"

#include "Core.RTTI/MetaObjectHelpers.h"

namespace Core {
namespace RTTI {
POOL_TAG_DECL(RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
template <typename T>
class TMetaTypedAtom;
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
class IMetaAtomVisitor;
class IMetaAtomConstVisitor;
//----------------------------------------------------------------------------
class FMetaClass;
class FMetaClassHashMap;
class FMetaClassName;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
class FMetaProperty;
template <typename T>
class TMetaTypedProperty;
template <typename T>
class TMetaFieldAccessor;
template <typename T, typename _Accessor >
class TMetaWrappedProperty;
//----------------------------------------------------------------------------
template <typename T>
struct TMetaType;
enum class EMetaTypeFlags : u32;
template <typename _From, typename _To>
struct TMetaTypePromote;
template <typename T>
struct TMetaTypeTraits;
template <typename T, typename _Enabled>
struct TMetaTypeTraitsImpl;
class IMetaTypeVirtualTraits;
class FAbstractMetaTypeScalarTraits;
template <typename T>
class TMetaTypeScalarTraits;
class FAbstractMetaTypePairTraits;
template <typename _First, typename _Second>
class TMetaTypePairTraits;
class FAbstractMetaTypeVectorTraits;
template <typename T>
class TMetaTypeVectorTraits;
class FAbstractMetaTypeDictionaryTraits;
template <typename _Key, typename _Value>
class TMetaTypeDictionaryTraits;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
