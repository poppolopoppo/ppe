#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_CONTENTPIPELINE_MESHBUILDER
#   define PPE_MESHBUILDER_API DLL_EXPORT
#else
#   define PPE_MESHBUILDER_API DLL_IMPORT
#endif

#include "RHI_fwd.h"

#include "Diagnostic/Logger_fwd.h"

#include "Memory/InSituPtr.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
FWD_INTEFARCE_UNIQUEPTR(MeshBuilderService);
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
enum class ERecomputeMode : u8;
struct FMeshBuilderSettings;
FWD_INTEFARCE_INSITUPTR(MeshFormat);
//----------------------------------------------------------------------------
typedef TGenericVertexSubPart<float3> FPositions3f;
typedef TGenericVertexSubPart<float4> FPositions4f;
typedef TGenericVertexSubPart<float2> FTexcoords2f;
typedef TGenericVertexSubPart<float3> FTexcoords3f;
typedef TGenericVertexSubPart<float4> FTexcoords4f;
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
