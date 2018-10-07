#pragma once

#include "Core.h"

#include "IO/StringView.h"
#include "Memory/RefPtr.h"

#ifdef EXPORT_PPE_RTTI
#   define PPE_RTTI_API DLL_EXPORT
#else
#   define PPE_RTTI_API DLL_IMPORT
#endif

namespace PPE {
template <typename T>
struct TInSituPtr;
namespace RTTI {
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
template <typename T>
const FMetaClass* MetaClass();
FStringView MetaClassName(const FMetaClass* metaClass);
//----------------------------------------------------------------------------
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
} //!namespace PPE
