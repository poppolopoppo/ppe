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
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FTypeId = u32;
//----------------------------------------------------------------------------
class FName;
struct FPathName;
//----------------------------------------------------------------------------
class FAny;
class FAtom;
//----------------------------------------------------------------------------
class ITypeTraits;
struct PTypeTraits;
//----------------------------------------------------------------------------
class IScalarTraits;
class ITupleTraits;
class IListTraits;
class IDicoTraits;
//----------------------------------------------------------------------------
enum class ENativeType : FTypeId;
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
class FMetaModule;
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
using FMetaEnumOrd = i64;
//----------------------------------------------------------------------------
class FMetaDatabase;
class FMetaDatabaseReadable;
class FMetaDatabaseReadWritable;
//----------------------------------------------------------------------------
enum class EObjectFlags : u32;
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
enum class EVisitorFlags : u32;
class IAtomVisitor;
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
