#include "stdafx.h"

#define EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR
#define EXPORT_PPE_RUNTIME_CORE_SCALARBOUNDINGBOX

#include "Maths/PackedVectors.h"
#include "Maths/PackingHelpers.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarRectangle.h"
#include "Maths/ScalarVector.h"

PRAGMA_DISABLE_RUNTIMECHECKS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::has_forceinit_constructor<uint2>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<word3>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<float4>::value);
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::is_pod_v<int2>);
STATIC_ASSERT(Meta::is_pod_v<uword2>);
STATIC_ASSERT(Meta::is_pod_v<float3>);
STATIC_ASSERT(Meta::is_pod_v<double4>);
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::TCheckSameSize<float, TScalarVector<float, 1>>::value);
STATIC_ASSERT(Meta::TCheckSameSize<float[2], float2>::value);
STATIC_ASSERT(Meta::TCheckSameSize<float[3], float3>::value);
STATIC_ASSERT(Meta::TCheckSameSize<float[4], float4>::value);
STATIC_ASSERT(Meta::TCheckSameSize<u32[2], u322>::value);
//----------------------------------------------------------------------------
namespace Constants {
    const float2 Float2_One     = float2(1.0f);
    const float2 Float2_Half    = float2(0.5f);
    const float2 Float2_Zero    = float2(0.0f);
    const float3 Float3_One     = float3(1.0f);
    const float3 Float3_Half    = float3(0.5f);
    const float3 Float3_Zero    = float3(0.0f);
    const float4 Float4_One     = float4(1.0f);
    const float4 Float4_Half    = float4(0.5f);
    const float4 Float4_Zero    = float4(0.0f);
} //!Constants
//----------------------------------------------------------------------------
// TScalarVector
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(bool, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(bool, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(bool, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(bool, 4);
////----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(int, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(int, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(int, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(int, 4);
////----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(unsigned int, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(unsigned int, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(unsigned int, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(unsigned int, 4);
////----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(float, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(float, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(float, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(float, 4);
////----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(double, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(double, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(double, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(double, 4);
//----------------------------------------------------------------------------
// Packed vectors
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u8>, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u8>, 4);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u8>, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u8>, 4);
////----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u16>, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u16>, 4);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u16>, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u16>, 4);
////----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u32>, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u32>, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TSNorm<u32>, 4);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u32>, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u32>, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(TUNorm<u32>, 4);
//----------------------------------------------------------------------------
// TScalarBoundingBox
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<int, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<int, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<int, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<unsigned int, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<unsigned int, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<unsigned int, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<unsigned int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<float, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<float, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<float, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<float, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<double, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<double, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<double, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoundingBox<double, 4>;
//----------------------------------------------------------------------------
// TScalarBoxWExtent
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<int, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<int, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<int, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<float, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<float, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<float, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<float, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<double, 1>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<double, 2>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<double, 3>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TScalarBoxWExtent<double, 4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_RESTORE_RUNTIMECHECKS
