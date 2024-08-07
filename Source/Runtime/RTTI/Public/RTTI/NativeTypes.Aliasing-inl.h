﻿#pragma once

#include "RTTI/NativeTypes.h"

#include "RTTI/NativeTypes.Scalar-inl.h"
#include "RTTI/NativeTypes.Tuple-inl.h"
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
// defer alias evaluation, so _From/_To don't need to be defined (forward declaration is enough)
template <typename _From, typename _To>
inline CONSTEXPR const PTypeInfos MakeAliasTypeInfos = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
    CONSTEXPR FTypeInfos infos = RTTI_TypeInfos(TypeTag< _To >)();
    return FTypeInfos{
        infos.TypeId,
        FSizeAndFlags{
            infos.Alignment(),
            infos.SizeInBytes(),
            infos.Flags() + ETypeFlags::Alias
        }};
};
//----------------------------------------------------------------------------
#define  DEF_RTTI_ALIASING_TRAITS(_FROM, ...) \
inline CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< _FROM >) { \
    return MakeAliasTypeInfos< _FROM, __VA_ARGS__ >; \
} \
inline PTypeTraits RTTI_Traits(TTypeTag<_FROM >) NOEXCEPT { \
    return MakeTraits< __VA_ARGS__ >(); \
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
DEF_RTTI_ALIASING_TRAITS(byte2, TStaticArray<byte, 2>)
DEF_RTTI_ALIASING_TRAITS(byte4, TStaticArray<byte, 4>)
DEF_RTTI_ALIASING_TRAITS(ubyte2, TStaticArray<ubyte, 2>)
DEF_RTTI_ALIASING_TRAITS(ubyte4, TStaticArray<ubyte, 4>)
DEF_RTTI_ALIASING_TRAITS(short2, TStaticArray<short, 2>)
DEF_RTTI_ALIASING_TRAITS(short4, TStaticArray<short, 4>)
DEF_RTTI_ALIASING_TRAITS(ushort2, TStaticArray<ushort, 2>)
DEF_RTTI_ALIASING_TRAITS(ushort4, TStaticArray<ushort, 4>)
DEF_RTTI_ALIASING_TRAITS(word2, TStaticArray<word, 2>)
DEF_RTTI_ALIASING_TRAITS(word3, TStaticArray<word, 3>)
DEF_RTTI_ALIASING_TRAITS(word4, TStaticArray<word, 4>)
DEF_RTTI_ALIASING_TRAITS(uword2, TStaticArray<uword, 2>)
DEF_RTTI_ALIASING_TRAITS(uword3, TStaticArray<uword, 3>)
DEF_RTTI_ALIASING_TRAITS(uword4, TStaticArray<uword, 4>)
DEF_RTTI_ALIASING_TRAITS(float2, TStaticArray<float, 2>)
DEF_RTTI_ALIASING_TRAITS(float3, TStaticArray<float, 3>)
DEF_RTTI_ALIASING_TRAITS(float4, TStaticArray<float, 4>)
DEF_RTTI_ALIASING_TRAITS(float2x2, TStaticArray<float, 2*2>)
DEF_RTTI_ALIASING_TRAITS(float3x3, TStaticArray<float, 3*3>)
DEF_RTTI_ALIASING_TRAITS(float4x4, TStaticArray<float, 4*4>)
//----------------------------------------------------------------------------
// common types wrapped as tuples :
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_TRAITS(FGuid, TStaticArray<u32, 2>)
DEF_RTTI_ALIASING_TRAITS(FQuaternion, TStaticArray<float, 4>)
DEF_RTTI_ALIASING_TRAITS(FTimestamp, i64)
DEF_RTTI_ALIASING_TRAITS(u128, TStaticArray<u64, 2>)
//----------------------------------------------------------------------------
#undef DEF_RTTI_ALIASING_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bindings for some basic types which don't justify being a native type
//----------------------------------------------------------------------------
#define DEF_RTTI_ALIASING_STRUCT(T) \
inline CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< T > tag) { \
    return StructInfos(tag); \
} \
inline PTypeTraits RTTI_Traits(TTypeTag< T > tag) NOEXCEPT { \
    return StructTraits(tag); \
}
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_STRUCT(FPathName)
//DEF_RTTI_ALIASING_STRUCT(FTransform) #TODO : wrap ahead without definition ?
//----------------------------------------------------------------------------
#undef DEF_RTTI_ALIASING_STRUCT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bindings for all TRawData<> template instances
//----------------------------------------------------------------------------
template <typename T>
inline CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< TRawData<T> >) {
    return MakeAliasTypeInfos< TRawStorage<T, typename FBinaryData::allocator_type>, FBinaryData >;
}
template <typename T>
inline CONSTEXPR PTypeTraits RTTI_Traits(TTypeTag< TRawData<T> >) NOEXCEPT {
    return MakeTraits< FBinaryData >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
