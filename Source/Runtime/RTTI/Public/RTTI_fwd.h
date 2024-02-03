#pragma once

#include "Core_fwd.h"

#include "IO/StringView.h"
#include "Memory/MemoryDomain.h"
#include "Memory/RefPtr.h"

#ifdef EXPORT_PPE_RUNTIME_RTTI
#   define PPE_RTTI_API DLL_EXPORT
#else
#   define PPE_RTTI_API DLL_IMPORT
#endif

#define USE_PPE_RTTI_CHECKS (!USE_PPE_PROFILING && !USE_PPE_FINAL_RELEASE)

#if USE_PPE_MEMORYDOMAINS
#   if PPE_VA_OPT_SUPPORTED
#       define NEW_RTTI(T, ...) PPE::RTTI::NewRtti< T >( *::PPE::RTTI::MetaClass<T>() __VA_OPT__(,) __VA_ARGS__ )
#   else
#       define NEW_RTTI(T, ...) PPE::RTTI::NewRtti< T >( *::PPE::RTTI::MetaClass<T>() ,## __VA_ARGS__ )
#   endif
#else
#   define NEW_RTTI(T, ...) PPE::RTTI::NewRtti< T >( __VA_ARGS__ )
#endif

namespace PPE {
class IRTTIService;
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
class FBinaryData;
class FOpaqueData;
class FOpaqueArray;
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
//----------------------------------------------------------------------------
enum class ETransactionState : u32;
enum class ETransactionFlags : u32;
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
// manipulate TRefPtr<FMetaObject> without defining FMetaObject
PPE_RTTI_API void AddRef(const FMetaObject* objref);
PPE_RTTI_API void RemoveRef(FMetaObject* objref);
PPE_RTTI_API void RemoveRef(const FMetaObject* objref);
PPE_RTTI_API hash_t hash_value(const PMetaObject& pobj);
//----------------------------------------------------------------------------
class ILoadContext;
class IUnloadContext;
//----------------------------------------------------------------------------
enum class EVisitorFlags : u32;
class IAtomVisitor;
//----------------------------------------------------------------------------
FWD_REFPTR(AtomHeap);
//----------------------------------------------------------------------------
template <typename _Enum>
const FMetaEnum* MetaEnum();
FStringLiteral MetaEnumName(const FMetaEnum* metaEnum);
i64 MetaEnumDefaultValue(const FMetaEnum* metaEnum);
//----------------------------------------------------------------------------
template <typename _Class>
const FMetaClass* MetaClass();
FStringLiteral MetaClassName(const FMetaClass* metaClass);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
