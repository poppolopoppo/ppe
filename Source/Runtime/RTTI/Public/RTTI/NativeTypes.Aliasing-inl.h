#pragma once

#include "RTTI/NativeTypes.h"

#include "RTTI/NativeTypes.Struct-inl.h"

#include "Maths/Packing_fwd.h"
#include "Maths/ScalarMatrix_fwd.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
struct FGuid;
class FQuaternion;
class PPE_CORE_API FTimestamp;
class FTransform;
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _From, typename _To>
struct TAliasedTypeInfos {
    CONSTEXPR FTypeInfos operator ()() const {
        STATIC_ASSERT(sizeof(_From) == sizeof(_To));
        return MakeTypeInfos<_To>();
    }
};
} //!details
//----------------------------------------------------------------------------
#define DEF_RTTI_ALIASING_TRAITS(_FROM, ...) \
    CONSTEXPR auto TypeInfos(TType< _FROM >) { \
        return details::TAliasedTypeInfos< _FROM, __VA_ARGS__ >{}; \
    } \
    inline PTypeTraits Traits(TType< _FROM >) NOEXCEPT { \
        return Traits(Type< __VA_ARGS__ >); \
    }
//----------------------------------------------------------------------------
// packed/quantized data are wrapped as encoded forms :
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_TRAITS(byten, i8)
DEF_RTTI_ALIASING_TRAITS(shortn, i16)
DEF_RTTI_ALIASING_TRAITS(wordn, i32)
DEF_RTTI_ALIASING_TRAITS(ubyten, u8)
DEF_RTTI_ALIASING_TRAITS(ushortn, u16)
DEF_RTTI_ALIASING_TRAITS(uwordn, u32)
DEF_RTTI_ALIASING_TRAITS(FHalfFloat, u16)
DEF_RTTI_ALIASING_TRAITS(UX10Y10Z10W2N, u32)
//----------------------------------------------------------------------------
// extern instantiation for most common types wrapped as a static array :
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_TRAITS(byte2, TArray<byte, 2>)
DEF_RTTI_ALIASING_TRAITS(byte4, TArray<byte, 4>)
DEF_RTTI_ALIASING_TRAITS(ubyte2, TArray<ubyte, 2>)
DEF_RTTI_ALIASING_TRAITS(ubyte4, TArray<ubyte, 4>)
DEF_RTTI_ALIASING_TRAITS(short2, TArray<short, 2>)
DEF_RTTI_ALIASING_TRAITS(short4, TArray<short, 4>)
DEF_RTTI_ALIASING_TRAITS(ushort2, TArray<ushort, 2>)
DEF_RTTI_ALIASING_TRAITS(ushort4, TArray<ushort, 4>)
DEF_RTTI_ALIASING_TRAITS(word2, TArray<word, 2>)
DEF_RTTI_ALIASING_TRAITS(word3, TArray<word, 3>)
DEF_RTTI_ALIASING_TRAITS(word4, TArray<word, 4>)
DEF_RTTI_ALIASING_TRAITS(uword2, TArray<uword, 2>)
DEF_RTTI_ALIASING_TRAITS(uword3, TArray<uword, 3>)
DEF_RTTI_ALIASING_TRAITS(uword4, TArray<uword, 4>)
DEF_RTTI_ALIASING_TRAITS(float2, TArray<float, 2>)
DEF_RTTI_ALIASING_TRAITS(float3, TArray<float, 3>)
DEF_RTTI_ALIASING_TRAITS(float4, TArray<float, 4>)
DEF_RTTI_ALIASING_TRAITS(float2x2, TArray<float, 2*2>)
DEF_RTTI_ALIASING_TRAITS(float3x3, TArray<float, 3*3>)
DEF_RTTI_ALIASING_TRAITS(float4x4, TArray<float, 4*4>)
//----------------------------------------------------------------------------
// common types wrapped as tuples :
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_TRAITS(FGuid, TArray<u32, 2>)
DEF_RTTI_ALIASING_TRAITS(FQuaternion, float4)
DEF_RTTI_ALIASING_TRAITS(FTimestamp, i64)
DEF_RTTI_ALIASING_TRAITS(u128, TArray<u64, 2>)
//----------------------------------------------------------------------------
#undef DEF_RTTI_ALIASING_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bindings for some basic types which don't justify being a native type
//----------------------------------------------------------------------------
#define DEF_RTTI_ALIASING_STRUCT(T) \
    CONSTEXPR auto TypeInfos(TType< T > t) { \
        return StructInfos(t); \
    } \
    CONSTEXPR PTypeTraits Traits(TType< T > t) NOEXCEPT { \
        return StructTraits(t); \
    }
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_STRUCT(FPathName)
//DEF_RTTI_ALIASING_STRUCT(FTransform) #TODO : wrap ahead without definition ?
//----------------------------------------------------------------------------
#undef DEF_RTTI_ALIASING_STRUCT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
