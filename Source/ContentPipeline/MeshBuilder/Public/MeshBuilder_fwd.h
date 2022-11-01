#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_CONTENTPIPELINE_MESHBUILDER
#   define PPE_MESHBUILDER_API DLL_EXPORT
#else
#   define PPE_MESHBUILDER_API DLL_IMPORT
#endif

#include "RHI_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_MESHBUILDER_API, MeshBuilder);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGenericMesh;
FWD_REFPTR(GenericMaterial);
//----------------------------------------------------------------------------
FWD_UNIQUEPTR(GenericVertexData);
template <typename T>
class TGenericVertexSubPart;
//----------------------------------------------------------------------------
typedef TGenericVertexSubPart<float3> FPositions3f;
typedef TGenericVertexSubPart<float4> FPositions4f;
typedef TGenericVertexSubPart<float2> FTexCoords2f;
typedef TGenericVertexSubPart<float3> FTexCoords3f;
typedef TGenericVertexSubPart<float4> FTexCoords4f;
typedef TGenericVertexSubPart<float4> FColors4f;
typedef TGenericVertexSubPart<float3> FNormals3f;
typedef TGenericVertexSubPart<float4> FNormals4f;
typedef TGenericVertexSubPart<float3> FTangents3f;
typedef TGenericVertexSubPart<float4> FTangents4f;
typedef TGenericVertexSubPart<float3> FBinormals3f;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
