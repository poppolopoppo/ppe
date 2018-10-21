#pragma once

#include "Core.h"

#include "IO/StringView.h"
#include "Memory/MemoryDomain.h"
#include "Memory/RefPtr.h"

#ifdef EXPORT_PPE_RTTI
#   define PPE_RTTI_API DLL_EXPORT
#else
#   define PPE_RTTI_API DLL_IMPORT
#endif

#if !defined(FINAL_RELEASE) && !defined(PROFILING_ENABLED)
#   define USE_PPE_RTTI_CHECKS 1
#else
#   define USE_PPE_RTTI_CHECKS 0
#endif

#if USE_PPE_MEMORYDOMAINS
#   define NEW_RTTI(T) new (*::PPE::RTTI::MetaClass<T>()) T
#else
#   define NEW_RTTI(T) NEW_REF(MetaObject, T)
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
class FMetaEnum;
template <typename T>
const FMetaEnum* MetaEnum();
FStringView MetaEnumName(const FMetaEnum* metaEnum);
i64 MetaEnumDefaultValue(const FMetaEnum* metaEnum);
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
class FMetaEnumHandle;
class FMetaNamespace;
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
class FMetaDatabase;
class FMetaDatabaseReadable;
class FMetaDatabaseReadWritable;
//----------------------------------------------------------------------------
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
FWD_REFPTR(AtomHeap);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
