#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
template <typename T>
class MetaTypedAtom;
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
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
class MetaTypeScalarTraits;
class MetaTypePairTraits;
class MetaTypeVectorTraits;
class MetaTypeDictionaryTraits;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
