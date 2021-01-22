#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVector;
//----------------------------------------------------------------------------
#define DECL_SCALARVECTOR_ALIAS(_Scalar, _ALIAS) \
    using CONCAT(_ALIAS, 2) = TScalarVector<_Scalar, 2>; \
    using CONCAT(_ALIAS, 3) = TScalarVector<_Scalar, 3>; \
    using CONCAT(_ALIAS, 4) = TScalarVector<_Scalar, 4>;

#define DECL_SCALARVECTOR(_Scalar) DECL_SCALARVECTOR_ALIAS(_Scalar, _Scalar)
//----------------------------------------------------------------------------
DECL_SCALARVECTOR(byte);
DECL_SCALARVECTOR(ubyte);
DECL_SCALARVECTOR_ALIAS(i16, short);
DECL_SCALARVECTOR(ushort);
DECL_SCALARVECTOR(word);
DECL_SCALARVECTOR(uword);
DECL_SCALARVECTOR(int)
DECL_SCALARVECTOR(unsigned)
DECL_SCALARVECTOR(float)
DECL_SCALARVECTOR(double)
//----------------------------------------------------------------------------
DECL_SCALARVECTOR(i16);
DECL_SCALARVECTOR(i32);
DECL_SCALARVECTOR(i64);
DECL_SCALARVECTOR(u16);
DECL_SCALARVECTOR(u32);
DECL_SCALARVECTOR(u64);
//----------------------------------------------------------------------------
using uint2 = unsigned2;
using uint3 = unsigned3;
using uint4 = unsigned4;
//----------------------------------------------------------------------------
using bool2 = TScalarVector<bool, 2>;
using bool3 = TScalarVector<bool, 3>;
using bool4 = TScalarVector<bool, 4>;
//----------------------------------------------------------------------------
using mask2 = TScalarVector<u32, 2>;
using mask3 = TScalarVector<u32, 3>;
using mask4 = TScalarVector<u32, 4>;
//----------------------------------------------------------------------------
#undef DECL_SCALARVECTOR
#undef DECL_SCALARVECTOR_ALIAS
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
