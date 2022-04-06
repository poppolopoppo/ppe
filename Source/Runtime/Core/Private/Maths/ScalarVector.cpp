#include "stdafx.h"

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
