#include "stdafx.h"

#include "RTTI/NativeTraits.h"

#include "RTTI/NativeTypes.h"

// for static assertions:
#include "IO/String.h"
#include "Maths/ScalarVector.h"
#include "Maths/PackedVectors.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Add some static unit-testing to all TypeInfos() boilerplate:
STATIC_ASSERT(MakeTypeInfos<bool>().IsBoolean());
STATIC_ASSERT(MakeTypeInfos<i8>().IsArithmetic());
STATIC_ASSERT(MakeTypeInfos<double>().IsArithmetic());
STATIC_ASSERT(MakeTypeInfos<int>().IsIntegral());
STATIC_ASSERT(MakeTypeInfos<i16>().IsSignedIntegral());
STATIC_ASSERT(MakeTypeInfos<u64>().IsUnsignedIntegral());
STATIC_ASSERT(MakeTypeInfos<float>().IsFloatingPoint());
STATIC_ASSERT(MakeTypeInfos<u32>().IsPOD());
STATIC_ASSERT(MakeTypeInfos<u64>().IsNative());
STATIC_ASSERT(MakeTypeInfos<FString>().IsString());
STATIC_ASSERT(MakeTypeInfos<FWString>().IsString());
STATIC_ASSERT(MakeTypeInfos<FWString>().IsTriviallyDestructible() == false);
STATIC_ASSERT(MakeTypeInfos<FName>().IsTriviallyDestructible());
STATIC_ASSERT(MakeTypeInfos<UX10Y10Z10W2N>().IsUnsignedIntegral());
STATIC_ASSERT(MakeTypeInfos<UX10Y10Z10W2N>().IsTriviallyDestructible());
STATIC_ASSERT(MakeTypeInfos<UX10Y10Z10W2N>().IsNative());
STATIC_ASSERT(MakeTypeInfos<UX10Y10Z10W2N>().IsAlias());
STATIC_ASSERT(MakeTypeInfos<UX10Y10Z10W2N>().TypeId == MakeTypeInfos<u32>().TypeId);
STATIC_ASSERT(MakeTypeInfos<u32>().IsNative());
STATIC_ASSERT(not MakeTypeInfos<u32>().IsAlias());
STATIC_ASSERT(MakeTypeInfos<float4>().IsPOD());
STATIC_ASSERT(MakeTypeInfos<float4>().IsTriviallyDestructible());
//----------------------------------------------------------------------------
PTypeTraits MakeTraits(ENativeType nativeType) NOEXCEPT {
    switch (nativeType) {
#define DEF_RTTI_MAKETRAITS(_Name, T, _TypeId) \
    case ENativeType::_Name: return MakeTraits<T>();
    FOREACH_RTTI_NATIVETYPES(DEF_RTTI_MAKETRAITS)
#undef DEF_RTTI_MAKETRAITS
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
