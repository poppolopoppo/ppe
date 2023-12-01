#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Dim, typename _Expr>
struct TScalarVectorExpr;
template <typename T, u32 _Dim, typename _Expr>
struct TScalarVectorAssignable;
template <typename T, u32 _Dim>
struct TScalarVectorStorage;
} //!namespace details
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
using TScalarVector = details::TScalarVectorExpr<T, _Dim, details::TScalarVectorAssignable<T, _Dim, details::TScalarVectorStorage<T, _Dim>>>;
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_ALIAS(_Scalar, _ALIAS) \
    using CONCAT(_ALIAS, 2) = TScalarVector<_Scalar, 2>; \
    using CONCAT(_ALIAS, 3) = TScalarVector<_Scalar, 3>; \
    using CONCAT(_ALIAS, 4) = TScalarVector<_Scalar, 4>;

#define PPE_SCALARVECTOR_DECL(_Scalar) PPE_SCALARVECTOR_ALIAS(_Scalar, _Scalar)
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_DECL(byte);
PPE_SCALARVECTOR_DECL(ubyte);
PPE_SCALARVECTOR_ALIAS(i16, short);
PPE_SCALARVECTOR_DECL(ushort);
PPE_SCALARVECTOR_DECL(word);
PPE_SCALARVECTOR_DECL(uword);
PPE_SCALARVECTOR_DECL(int)
PPE_SCALARVECTOR_DECL(unsigned)
PPE_SCALARVECTOR_DECL(float)
PPE_SCALARVECTOR_DECL(double)
PPE_SCALARVECTOR_DECL(i16);
PPE_SCALARVECTOR_DECL(i32);
PPE_SCALARVECTOR_DECL(i64);
PPE_SCALARVECTOR_DECL(u16);
PPE_SCALARVECTOR_DECL(u32);
PPE_SCALARVECTOR_DECL(u64);
PPE_SCALARVECTOR_DECL(bool);
PPE_SCALARVECTOR_DECL(size_t);
PPE_SCALARVECTOR_ALIAS(u32, mask);
//----------------------------------------------------------------------------
#undef PPE_SCALARVECTOR_DECL
#undef PPE_SCALARVECTOR_ALIAS
//----------------------------------------------------------------------------
using uint2 = unsigned2;
using uint3 = unsigned3;
using uint4 = unsigned4;
//----------------------------------------------------------------------------
using FPoint = float3;
//----------------------------------------------------------------------------
using FRgba32f = float4;
using FRgba32u = u324;
using FRgba32i = i324;
using FRgba8u = ubyte4;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Constants {
extern const float2 PPE_CORE_API Float2_One;
extern const float2 PPE_CORE_API Float2_Half;
extern const float2 PPE_CORE_API Float2_Zero;
extern const float3 PPE_CORE_API Float3_One;
extern const float3 PPE_CORE_API Float3_Half;
extern const float3 PPE_CORE_API Float3_Zero;
extern const float4 PPE_CORE_API Float4_One;
extern const float4 PPE_CORE_API Float4_Half;
extern const float4 PPE_CORE_API Float4_Zero;
} //!Constants
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
