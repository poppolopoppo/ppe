#include "stdafx.h"

#define EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR
#define EXPORT_PPE_RUNTIME_CORE_SCALARBOUNDINGBOX

#include "Maths/PackedVectors.h"
#include "Maths/PackingHelpers.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarRectangle.h"
#include "Maths/ScalarVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::has_forceinit_constructor<uint2>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<word3>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<float4>::value);
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
#if not EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR_DISABLED
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<bool, 1>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<bool, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<bool, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<bool, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<int, 1>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<int, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<int, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<unsigned int, 1>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<unsigned int, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<unsigned int, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<unsigned int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<float, 1>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<float, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<float, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<float, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<double, 1>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<double, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<double, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<double, 4>;
//----------------------------------------------------------------------------
// Packed vectors
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u8>, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u8>, 4>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u8>, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u8>, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u16>, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u16>, 4>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u16>, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u16>, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u32>, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u32>, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TSNorm<u32>, 4>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u32>, 2>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u32>, 3>;
EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) TScalarVector<TUNorm<u32>, 4>;
#endif //!EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR_DISABLED
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
