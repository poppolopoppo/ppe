// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaEnumHelpers.h"

#include "RTTI/AtomVisitor.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/Module.h"
#include "RTTI/NativeTypes.Definitions-inl.h"

#include "MetaClass.h"
#include "MetaFunction.h"
#include "MetaProperty.h"
#include "MetaObject.h"
#include "MetaTransaction.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(MakeTypeInfos<ETypeFlags>().IsArithmetic());
STATIC_ASSERT(MakeTypeInfos<EClassFlags>().IsUnsignedIntegral());
STATIC_ASSERT(MakeTypeInfos<EClassFlags>().IsPOD());
STATIC_ASSERT(MakeTypeInfos<EClassFlags>().IsTriviallyDestructible());
//----------------------------------------------------------------------------
bool MetaEnumParse(const FMetaEnum& menum, const FStringView& str, FMetaEnumOrd* ord) NOEXCEPT {
    return menum.ParseValue(str, ord);
}
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, ENativeType);
RTTI_ENUM_BEGIN(RTTI, ENativeType)
#define RTTI_NATIVETYPE_ENUMVALUE(_Name, T, _TypeId) RTTI_ENUM_VALUE(_Name)
FOREACH_RTTI_NATIVETYPES(RTTI_NATIVETYPE_ENUMVALUE)
#undef RTTI_NATIVETYPE_ENUMVALUE
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EClassFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, EClassFlags)
RTTI_ENUM_VALUE(None)
RTTI_ENUM_VALUE(Concrete)
RTTI_ENUM_VALUE(Abstract)
RTTI_ENUM_VALUE(Dynamic)
RTTI_ENUM_VALUE(Public)
RTTI_ENUM_VALUE(Mergeable)
RTTI_ENUM_VALUE(Deprecated)
RTTI_ENUM_VALUE(Registered)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EEnumFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, EEnumFlags)
RTTI_ENUM_VALUE(None)
RTTI_ENUM_VALUE(Flags)
RTTI_ENUM_VALUE(Deprecated)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EFunctionFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, EFunctionFlags)
RTTI_ENUM_VALUE(Const)
RTTI_ENUM_VALUE(Public)
RTTI_ENUM_VALUE(Protected)
RTTI_ENUM_VALUE(Private)
RTTI_ENUM_VALUE(Deprecated)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EObjectFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, EObjectFlags)
RTTI_ENUM_VALUE(None)
RTTI_ENUM_VALUE(Loaded)
RTTI_ENUM_VALUE(Unloaded)
RTTI_ENUM_VALUE(Exported)
RTTI_ENUM_VALUE(TopObject)
RTTI_ENUM_VALUE(Transient)
RTTI_ENUM_VALUE(Frozen)
#ifdef WITH_RTTI_VERIFY_PREDICATES
RTTI_ENUM_VALUE(Verifying)
#endif
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EParameterFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, EParameterFlags)
RTTI_ENUM_VALUE(Default)
RTTI_ENUM_VALUE(Output)
RTTI_ENUM_VALUE(Optional)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EPropertyFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, EPropertyFlags)
RTTI_ENUM_VALUE(Public)
RTTI_ENUM_VALUE(Protected)
RTTI_ENUM_VALUE(Private)
RTTI_ENUM_VALUE(ReadOnly)
RTTI_ENUM_VALUE(Deprecated)
RTTI_ENUM_VALUE(Member)
RTTI_ENUM_VALUE(Dynamic)
RTTI_ENUM_VALUE(Transient)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, ETypeFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, ETypeFlags)
RTTI_ENUM_VALUE(Unknown)
RTTI_ENUM_VALUE(Scalar)
RTTI_ENUM_VALUE(Tuple)
RTTI_ENUM_VALUE(List)
RTTI_ENUM_VALUE(Dico)
RTTI_ENUM_VALUE(Alias)
RTTI_ENUM_VALUE(Arithmetic)
RTTI_ENUM_VALUE(Boolean)
RTTI_ENUM_VALUE(Enum)
RTTI_ENUM_VALUE(FloatingPoint)
RTTI_ENUM_VALUE(Native)
RTTI_ENUM_VALUE(Object)
RTTI_ENUM_VALUE(String)
RTTI_ENUM_VALUE(UnsignedIntegral)
RTTI_ENUM_VALUE(POD)
RTTI_ENUM_VALUE(TriviallyDestructible)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, ETransactionFlags);
RTTI_ENUM_FLAGS_BEGIN(RTTI, ETransactionFlags)
RTTI_ENUM_VALUE(Default)
RTTI_ENUM_VALUE(KeepDeprecated)
RTTI_ENUM_VALUE(KeepTransient)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, ETransactionState);
RTTI_ENUM_BEGIN(RTTI, ETransactionState)
RTTI_ENUM_VALUE(Unloaded)
RTTI_ENUM_VALUE(Loading)
RTTI_ENUM_VALUE(Loaded)
RTTI_ENUM_VALUE(Mounting)
RTTI_ENUM_VALUE(Mounted)
RTTI_ENUM_VALUE(Unmounting)
RTTI_ENUM_VALUE(Unloading)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_RTTI_API, EVisitorFlags);
RTTI_ENUM_BEGIN(RTTI, EVisitorFlags)
RTTI_ENUM_VALUE(Unknown)
RTTI_ENUM_VALUE(KeepDeprecated)
RTTI_ENUM_VALUE(KeepTransient)
RTTI_ENUM_VALUE(OnlyObjects)
RTTI_ENUM_VALUE(NoRecursion)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
