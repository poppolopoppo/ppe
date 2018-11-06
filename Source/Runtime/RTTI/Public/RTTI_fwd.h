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

#define USE_PPE_RTTI_CHECKS (!USE_PPE_PROFILING && !USE_PPE_FINAL_RELEASE)

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
enum class EClassFlags : u32;
class FMetaClass;
enum class EEnumFlags : u32;
class FMetaEnum;
enum class EFunctionFlags : u32;
class FMetaFunction;
enum class EParameterFlags : u32;
class FMetaParameter;
enum class EPropertyFlags : u32;
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
template <typename T>
const FMetaEnum* MetaEnum();
FStringView MetaEnumName(const FMetaEnum* metaEnum);
i64 MetaEnumDefaultValue(const FMetaEnum* metaEnum);
//----------------------------------------------------------------------------
template <typename T>
const FMetaClass* MetaClass();
FStringView MetaClassName(const FMetaClass* metaClass);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
